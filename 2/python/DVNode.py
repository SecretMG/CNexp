import json
import os
import socket
import datetime

from threading import Thread, Event, Timer
from time import sleep
from utils.args import args


class DVNode:
    """距离矢量节点"""
    def __init__(self, name, address, socks):
        self.name = name
        self.address = address      # tuple
        self.Routing_Table = {}     # name: (dist, port)
        self.neighbors = {}         # name: sock
        self.dir_folder = 'utils/params'
        self.Frequency = args.Frequency / 1000
        self.Unreachable = args.Unreachable
        self.binding = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.binding.bind(address)
        self.socks = socks  # name: (ip, port)
        self.logs = []
        self.send_seq = 1
        self.recv_seq = 1
        self.recv_RTs = {}  # 以Source Node Name为KEY，记录一个时间段内从每个点收到的最新的路由表

        self.MaxValidTime = args.MaxValidTime / 1000         # 超时时间
        self.timer_pool = {}        # Timer线程池

        self.__thread_sending = Thread(target=self.__thread_send)
        self.__thread_recving = Thread(target=self.__thread_recv)
        self.__event_stop = Event()
        self.__event_run = Event()

    def start(self, init_table_path: str):
        print("start!")
        self.__read_table(init_table_path)

        self.__event_stop.clear()

        if not self.__thread_sending.is_alive():
            self.__thread_sending = Thread(target=self.__thread_send)
            self.__thread_sending.start()
        if not self.__thread_recving.is_alive():
            self.__thread_recving = Thread(target=self.__thread_recv)
            self.__thread_recving.start()

        self.__event_run.set()

    def stop(self):
        print("stop!")
        self.__event_run.clear()
        self.__event_stop.set()
        return self.logs

    def restart(self, init_table_path):
        print('restart!')
        self.__read_table(init_table_path)
        self.__event_stop.clear()
        self.__event_run.set()


    def pause(self):
        print("pause!")
        self.__event_run.clear()

    def __thread_send(self):
        print("send_thread running.")
        while not self.__event_stop.is_set():
            if self.__event_run.is_set():
                self.__send_table()
            sleep(self.Frequency)
        self.binding.close()    # 否则接收线程由于一直在监听无法退出
        print("send_thread end.")

    def __thread_recv(self):
        print("recv_thread running.")
        while not self.__event_stop.is_set():
            if self.__event_run.is_set():
                # socket接收
                try:
                    recv_table, sender_ip = self.binding.recvfrom(args.FrameSize)
                except:
                    continue
                recv_table = recv_table.decode()
                recv_table = json.loads(recv_table)

                # 如果收到，则重置timer
                sock_name = {v: k for k, v in self.neighbors.items()}       # 反转下neighbors
                name = sock_name[sender_ip]
                # print(f'收到{name}的表，重置超时计时器')
                # 重置Timer并启动
                self.timer_pool[name].cancel()
                self.timer_pool[name] = Timer(self.MaxValidTime, self.__task_timeout, args=name)
                self.timer_pool[name].start()

                self.__update_table(recv_table)
        print("recv_thread end.")

    def __read_table(self, path):
        self.Routing_Table = {self.name: (0, self.address[1])}    # 目标名字：（最短距离，到达该目标的首选出境线路）

        file_name = os.path.join(self.dir_folder, path)
        with open(file_name) as file:
            lines = file.readlines()
            for line in lines:
                line = line.strip()
                if line:
                    neighbor, dist, port = line.split()
                    dist, port = int(dist), int(port)  # 除名字外都使用int类型
                    self.Routing_Table[neighbor] = (dist, port)
                    self.neighbors[neighbor] = self.socks[neighbor]
                    # 为每一个neighbor添加超时线程并启动
                    self.timer_pool[neighbor] = Timer(self.MaxValidTime, self.__task_timeout, args=neighbor)
                    self.timer_pool[neighbor].start()

    def __send_table(self):
        send_table = {}  # 目的节点：（距离，发送节点name，使用的邻居name）
        for target_name, (dist, neighbor_name) in self.Routing_Table.items():
            send_table[target_name] = (dist, self.name, neighbor_name)
        data = json.dumps(send_table)
        # 整理出需要发送的报文
        for neighbor in self.neighbors:
            # 发送给所有邻居
            self.binding.sendto(data.encode(), self.neighbors[neighbor])
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
        log = ''
        print(f'## Sent. Source Node= {self.name}; Sequence Number = {self.send_seq}, time = {now}')
        log += f'## Sent. Source Node= {self.name}; Sequence Number = {self.send_seq}, time = {now}\n'
        self.send_seq += 1
        for target, (dist, port) in self.Routing_Table.items():
            print(f'DestNode = {target}; Distance = {dist}; Neighbor = {port}')
            log += f'DestNode = {target}; Distance = {dist}; Neighbor = {port}\n'
        self.logs.append(log)

    def __update_table(self, recv_table: dict):
        # 更新表

        for to_name, (dist, from_name, neighbor_name) in recv_table.items():
            # if self.Routing_Table[from_name] == args.Unreachable:
            #     # 若是一个新启动的点，则先更新到它的距离
            #     self.Routing_Table[from_name] = recv_table[self.name][0]
            # 目的节点：（距离，发送节点name，使用的邻居name）
            if to_name not in self.Routing_Table:
                self.Routing_Table[to_name] = (dist + self.Routing_Table[from_name][0], from_name)
            else:
                old_target_dist = self.Routing_Table[to_name][0]
                new_target_dist = dist + self.Routing_Table[from_name][0]
                if new_target_dist < old_target_dist:
                    # 二者相同不能更新，这样会导致正确邻居被覆盖成转发邻居
                    self.Routing_Table[to_name] = (new_target_dist, from_name)
        now = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
        log = ''
        print(f'## Received. Source Node= {from_name}; Sequence Number = {self.recv_seq}, time = {now}')
        log += f'## Received. Source Node= {from_name}; Sequence Number = {self.recv_seq}, time = {now}\n'
        self.recv_seq += 1
        for to_name, (dist, from_name, neighbor_name) in recv_table.items():
            print(f'DestNode = {to_name}; Distance = {dist}, Neighbor = {neighbor_name}')
            log += f'DestNode = {to_name}; Distance = {dist}, Neighbor = {neighbor_name}\n'
        self.logs.append(log)

    def __task_timeout(self, id):
        # 超时后的任务
        log = f'{id} time out\n'
        print(log)
        self.logs.append(log)
        self.Routing_Table[id] = (self.Unreachable, self.Routing_Table[id][1])
