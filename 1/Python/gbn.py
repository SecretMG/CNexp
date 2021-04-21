# encoding: utf-8
# author: Zheng Zhihan

"""
功能实现：
发送窗口-发送线程
发送窗口-接收线程
发送窗口-超时控制定时器线程池
发送窗口-文件读入

待完成：
接收窗口
一些细节
"""

from threading import Thread, Timer, Event
from args import args
import time
from pdu import PDU, unpack_pdu, crc_check
import random

class SendingWindow:
    # 发送窗口
    def __init__(self, filelist):
        #
        self.filelist = filelist
        self.fileptr = args.sending_window_size - 1
        # 事件定义
        self.__event_mainthread = Event()
        self.__event_send = Event()
        self.__event_endsend = Event()
        self.__event_slide = Event()
        self.__event_endslide = Event()
        self.__event_timer = Event()
        self.__event_timeout = Event()
        # self.__event_recv = Event()
        # 发送窗口序号列表
        self.sw_nolist = list(range(args.init_no, args.sending_window_size))
        # 发送窗口帧列表
        self.sw_pdulist = [PDU(seq=i, info=self.filelist[i]) for i in self.sw_nolist]
        # 主线程
        self.__thread_main = Thread(target=self.__main_thread)
        # 发送线程
        self.__thread_sending = Thread(target=self.__thread_send)
        # 滑动窗口线程
        self.__thread_sliding = Thread(target=self.__thread_slide)
        # 接收线程
        # self.__thread_recving = Thread(target=self.__thread_recv)
        # 超时控制定时器线程池
        self.__thread_timeouting = Thread(target=self.__thread_timeouter)
        self.sw_timeouter = [None]*args.sending_window_size
        # 记录最后一个已发送的帧
        self.last_sent = -1
        # 记录收到的帧的ack
        self.sw_recvlist = []

    def start(self):
        self.__thread_main.start()
        self.__thread_sending.start()
        self.__thread_sliding.start()
        # self.__thread_recving.start()
        self.__thread_timeouting.start()
        self.__event_mainthread.set()
        # self.__event_recv.set()
        self.__event_timer.set()

    def stop(self):
        #self.__event_recv.clear()
        self.__event_timer.clear()
        self.__event_mainthread.clear()
        print("停止请求已发送")

    def __main_thread(self):
        while True:
            self.__event_mainthread.wait()

            if (self.last_sent >= 0) and (len(self.sw_recvlist) > 0):
                self.__event_endslide.clear()
                self.__event_slide.set()
                self.__event_endslide.wait()
            elif self.last_sent < args.sending_window_size - 1:
                self.__event_endsend.clear()
                self.__event_send.set()
                self.__event_endsend.wait()
            else:
                self.__event_endsend.set()
                self.__event_endslide.set()

    """
    def __thread_recv(self):
        ack = -1
        while True:
            self.__event_recv.wait()
            self.sw_recvlist.append(ack)
    """

    def get_ack(self, ack):
        self.sw_recvlist.append(ack)

    def __thread_send(self):
        while True:
            self.__event_send.wait()
            if (self.last_sent >= -1) and (self.last_sent < args.sending_window_size-1):
                self.last_sent += 1
                if self.sw_pdulist[self.last_sent].info != '#':
                    send_pdu(self.sw_pdulist[self.last_sent])
                    self.sw_timeouter[self.last_sent] = Timer(args.time_out/1000, self.__thread_timeout)
                    # print("%d已设置Timer"%self.last_sent)
                    self.sw_timeouter[self.last_sent].start()
            else:
                pass
            self.__event_send.clear()
            self.__event_endsend.set()

    def __thread_timeouter(self):
        while True:
            self.__event_timer.wait()
            self.__event_timeout.wait()
            # 停止发送和滑动
            print("等待结束发送滑动线程")
            self.__event_mainthread.clear()
            self.__event_endsend.wait()
            self.__event_endslide.wait()
            # 清理线程池
            for i in self.sw_timeouter:
                if i is not None:
                    i.cancel()
            print("计数器进程池清理完成")
            self.last_sent = -1
            # 恢复主线程
            self.__event_timeout.clear()
            self.__event_mainthread.set()

    def __thread_timeout(self):
        # 超时一定是最先出发的超时，这样更安全
        if self.__event_timeout.is_set():
            print("已触发")
            return
        else:
            self.__event_timeout.set()
            print("超时触发")

    def __thread_slide(self):
        while True:
            self.__event_slide.wait()
            if (self.last_sent >= 0) and (len(self.sw_recvlist) > 0) and (self.fileptr < len(self.filelist) + args.sending_window_size - 1):
                if self.sw_recvlist[0] == self.sw_nolist[0]:
                    print("slide%d"%self.sw_nolist[0])
                    # nolist向右滑动一位
                    self.sw_nolist.append(self.sw_nolist.pop(0))
                    self.last_sent -= 1
                    # timer向右滑动一位
                    if self.sw_timeouter[0] is not None:
                        self.sw_timeouter[0].cancel()
                    self.sw_timeouter.pop(0)
                    self.sw_timeouter.append(None)
                    # PDU向右滑动一位
                    self.sw_pdulist.append(self.sw_pdulist.pop(0))

                    self.fileptr += 1
                    if self.fileptr + 1 > len(self.filelist):
                        self.sw_pdulist[-1].update(seq=self.sw_nolist[-1], info='#')
                    else:
                        self.sw_pdulist[-1].update(seq=self.sw_nolist[-1], info=self.filelist[self.fileptr])
                else:
                    self.sw_recvlist.pop(0)
            else:
                # 无效包
                pass
            self.__event_slide.clear()
            self.__event_endslide.set()


class RecvingWindow:
    # 接收窗口
    def __init__(self):
        # 事件定义
        self.__event_mainthread = Event()
        # 主线程
        self.__thread_main = Thread(target=self.__main_thread)
        # 接收窗口
        self.rw_needack = 0
        # 反馈包
        self.rw_pdu = PDU()

    def start(self):
        self.__thread_main.start()
        self.__event_mainthread.set()

    def stop(self):
        self.__event_mainthread.clear()

    def __main_thread(self):
        while True:
            self.__event_mainthread.wait()

            # recv
            # 验错
            # 获得pdu
            # 比较seq与needack

            seq = random.randint(0, 3)
            if seq == self.rw_needack:
                # pdu.info写入
                self.rw_needack = (seq+1) % args.sending_window_size
                # 发送反馈
                self.rw_pdu.update(ack=seq)
                print("sendack%d"%seq)
                # send(pdu.binpack)
            else:
                # 丢弃pdu
                pass


def send_pdu(pdu):
    print("sentpdu:seq=%d, info=%s" % (pdu.seq, pdu.info))

def recv_thread(sw, rw):
    # 处理
    while True:
        # pdu = unpack_pdu()
        ack = random.randint(0, 3)
        # 分类
        time.sleep(0.05)
        sw.get_ack(ack)


def client():
    file = ['023a', '134e', '24567y', '34dsfa', '41234s', '52134', '61234ss', 'eeenD' ,'ax]=========']
    # 发送窗口实例化
    sw = SendingWindow(file)
    rw = RecvingWindow()


    # 开始发送线程
    sw.start()

    recv_thread(sw,rw)
    # rw = RecvingWindow()
    # rw.start()
    # rw.stop()
    sw.stop()


client()
