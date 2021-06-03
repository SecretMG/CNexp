import os

from threading import Thread, Event
from time import sleep


class DVNode:
    """距离矢量节点"""
    def __init__(self, name: str, address: tuple):
        self.name = name
        self.address = address
        self.Routing_Table = {}
        self.neighbors = []
        self.dir_folder = 'utils/params'

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
        self.__event_stop.set()
        self.__event_run.clear()

    def pause(self):
        print("pause!")
        self.__event_run.clear()

    def __thread_send(self):
        print("send_thread running.")
        while not self.__event_stop.is_set():
            if self.__event_run.is_set():
                # 等待敲钟：全局发送信号.wait()
                self.__send_table()
        print("send_thread end.")

    def __thread_recv(self):
        print("recv_thread running.")
        while not self.__event_stop.is_set():
            if self.__event_run.is_set():
                # socket接收
                recv_table = {}
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
                    self.neighbors.append(neighbor)
        for target, (dist, port) in self.Routing_Table.items():
            pass

    def __send_table(self):
        for i in self.neighbors:
            # 发送路由表到i
            print(f"发送至{i}")

    def __update_table(self, recv_table: dict):
        # 更新表
        pass

