from threading import *
import time


class test:
    def __init__(self, my_binding=None, sendto_sock=None):
        # 事件定义
        self.my_binding = my_binding
        self.target = sendto_sock
        # 主线程
        self.__thread_main = Thread(target=self.__main_thread)
        self.__event_mainthread = Event()
        # 接收窗口
        self.ack_expected = 0
        # seq&info
        self.seq_and_info = []
        # 接收到的信息
        self.recv_info = []
        print('init')

    def start(self):
        self.__thread_main.start()
        print('已开启主线程')
        self.__event_mainthread.set()
        print('主信号=true')


    def stop(self):
        self.__event_mainthread.clear()
        print('主信号=false')


    def __main_thread(self):
        while True:
            self.__event_mainthread.wait()
            print('已等到主信号')


a = test()
a.start()