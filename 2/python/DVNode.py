# -*- coding: utf-8 -*-

from threading import Thread, Event
from time import sleep


class DVNode:
    """距离矢量节点"""
    def __init__(self, name: str, address: tuple):
        self.name = name
        self.address = address
        self.routing_table = {}
        self.neighbors = []

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

    def __read_table(self, path: str):
        self.routing_table = {}
        # 读入路由表
        self.neighbors = []
        # 获得邻居节点

    def __send_table(self):
        for i in self.neighbors:
            # 发送路由表到i
            print(f"发送至{i}")

    def __update_table(self, recv_table: dict):
        # 更新表
        pass


a = DVNode('a', ('127.0.0.1', 12345))
a_path = 'a.txt'
a.start(a_path)
sleep(3)
a.pause()
sleep(3)
a.stop()

sleep(3)
a.start(a_path)
sleep(3)
a.stop()
