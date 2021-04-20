# encoding: utf-8
# author: Zheng Zhihan

"""
功能实现：
发送窗口-发送线程
发送窗口-接收线程
发送窗口-超时控制定时器线程池

待完成：
发送窗口-文件读入
接收窗口
"""

from threading import Thread, Timer, Event
from args import args
import time
from pdu import PDU, unpack_pdu, crc_check


class SendingWindow:
    # 发送窗口
    def __init__(self):
        # 事件定义
        self.__event_send = Event()
        self.__event_notimeout = Event()
        self.__event_recv = Event()
        # 退出信号
        self.__stop = False
        # 发送窗口序号列表
        self.sw_nolist = list(range(args.init_no, args.sending_window_size))
        # 发送窗口帧列表
        self.sw_pdulist = [PDU(seq=i) for i in self.sw_nolist]
        # 发送线程
        self.__thread_sending = Thread(target=self.__keep_sending)
        # 接收进程
        self.__thread_recving = Thread(target=self.__keep_recving)
        # 超时控制定时器线程池
        self.sw_timeouter = [None]*args.sending_window_size
        # 记录最后一个已发送的帧
        self.last_sent = -1

    def start_sending(self):
        self.__event_send.set()
        self.__event_notimeout.set()
        self.__thread_sending.start()

    def start_recving(self):
        self.__event_recv.set()
        self.__thread_recving.start()

    def stop(self):
        self.__stop = True

    def __keep_recving(self):
        while True:
            if self.__stop:
                break
            self.__event_recv.wait()

            # bin_pack = recv()
            # 验错、提取ack
            ack = self.sw_nolist[0]                                                 # unfinished

            if self.last_sent >= 0 and ack == self.sw_nolist[0]:
                # 阻塞发送
                self.__event_send.clear()
                print("recv%d"%ack)
                # 向右滑动一位
                self.sw_nolist.append(self.sw_nolist.pop(0))
                self.last_sent -= 1

                if self.sw_timeouter[0] is not None:
                    self.sw_timeouter[0].cancel()
                self.sw_timeouter.pop(0)
                self.sw_timeouter.append(None)

                # PDU读取
                self.sw_pdulist[0].update(seq=self.sw_nolist[0], info="读到的模块")      # unfinished

                # 发送进程开始
                self.__event_send.set()
            else:
                # 丢弃
                continue

    def __keep_sending(self):
        while True:
            if self.__stop:
                break

            self.__event_send.wait()

            if self.last_sent != args.sending_window_size - 1:
                self.__event_recv.clear()
                self.last_sent += 1

                send_frame(self.last_sent, self.sw_nolist[self.last_sent])              # unfinished
                self.sw_timeouter[self.last_sent] = Timer(args.time_out/1000, self.__time_out)
                self.sw_timeouter[self.last_sent].start()

                self.__event_recv.set()
            else:
                continue

    def __time_out(self):
        # 阻塞其他timer
        if self.__event_notimeout.is_set():
            self.__event_notimeout.clear()
        else:
            return

        # 超时一定是最先出发的超时，这样更安全
        print("超时触发")
        # 阻塞发送接收进程
        self.__event_send.clear()
        self.__event_recv.clear()
        # 停止其他的超时计数器
        for i in self.sw_timeouter:
            if i is not None:
                i.cancel()
        # 重置最后发送帧的序号
        self.last_sent = -1
        # 重启发送接收进程
        self.__event_send.set()
        self.__event_notimeout.set()
        self.__event_recv.set()


def send_frame(no, seq):
    print("sent%d seq%d" % (no, seq))


def client():
    # 发送窗口实例化
    sw = SendingWindow()
    # 读入发送内容到发送窗口
                                                                    # unfinished
    # 开始发送线程
    sw.start_sending()

    sw.start_recving()

    # 5秒后停止
    time.sleep(5)
    sw.stop()


client()
