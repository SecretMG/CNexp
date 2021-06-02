# encoding: utf-8
# author: Zheng Zhihan

"""
功能实现：
发送窗口-发送线程
发送窗口-接收线程
发送窗口-超时控制定时器线程池
发送窗口-文件读入
接收窗口

待完成：
文件写入接口
"""

from threading import Thread, Timer, Event
from args import args
from pdu import PDU, unpack_pdu, crc_check
import os
import socket
from loss_error import loss_with_rate, error_with_rate
from utils import get_timestamp

error_rate = 0
loss_rate = 0
dir_out = 'outputs'


def send_pdu(pdu, my_binding, target_sock):
    print("%s send pdu to %s:seq=%d, ack=%d info=%s\n" % (str(my_binding.getsockname()), str(target_sock), pdu.seq, pdu.ack, pdu.info[0:10]), end='')
    send_pack = error_with_rate(error_rate, pdu.bin_pack)  # 以0.2的几率模拟在信道中发生的错误
    my_binding.sendto(send_pack, target_sock)   # 无论如何都需要传输


class SendingWindow:
    '''发送窗口'''
    def __init__(self, my_binding, sendto_sock, filelist):
        self.mybinding = my_binding
        self.target = sendto_sock
        self.filelist = filelist
        self.fileptr = args.sending_window_size - 1

        '''Events'''
        self.__event_mainthread = Event()
        self.__event_send = Event()
        self.__event_endsend = Event()
        self.__event_slide = Event()
        self.__event_endslide = Event()
        self.__event_timer = Event()
        self.__event_timeout = Event()
        self.__running = Event()

        '''Threads'''
        self.__thread_main = Thread(target=self.__main_thread)  # 主线程
        self.__thread_sending = Thread(target=self.__thread_send)   # 发送线程
        self.__thread_sliding = Thread(target=self.__thread_slide)  # 滑窗线程
        self.__thread_timeouting = Thread(target=self.__thread_timeouter)   # 超时控制定时器线程池

        '''Utils'''
        # 最大序号
        self.max_no = args.max_sending_no
        # 发送窗口序号列表
        self.sw_nolist = list(range(0, args.sending_window_size))
        # 发送窗口帧列表
        self.sw_pdulist = [PDU(seq=i, info=self.filelist[i]) for i in self.sw_nolist]
        self.sw_timeouter = [None]*args.sending_window_size
        self.last_sent = -1     # 记录最后一个已发送的帧
        self.sw_recvlist = []   # 记录收到的帧的ack
        self.success_sent = 0   # 成功发送并接收的包数

    def start(self):
        ''''''
        self.__running.set()
        '''Threads'''
        self.__thread_main.start()  # 进行滑窗/发送
        self.__thread_sending.start()
        self.__thread_sliding.start()
        self.__thread_timeouting.start()

        '''Events'''
        # set()：将事件内部标识置为True
        # clear()：将事件内部标识置为False
        # wait()：阻塞，直到事件内部标识置为True

        self.__event_mainthread.set()
        self.__event_timer.set()

    def __main_thread(self):
        while self.__running.is_set():
            self.__event_mainthread.wait()  # 先等待主线程

            if (self.last_sent >= 0) and (len(self.sw_recvlist) > 0):
                # 滑窗
                self.__event_endslide.clear()
                self.__event_slide.set()
                self.__event_endslide.wait()
            elif self.last_sent < args.sending_window_size - 1:
                # 发送
                self.__event_endsend.clear()
                self.__event_send.set()
                self.__event_endsend.wait()
            else:
                self.__event_endsend.set()
                self.__event_endslide.set()

    def stop(self):
        self.__running.clear()
        self.__event_timer.clear()
        self.__event_mainthread.clear()
        print("停止请求已发送\n", end='')
        self.__running.clear()

    def get_ack(self, ack):
        self.sw_recvlist.append(ack)

    def __thread_send(self):
        while self.__running.is_set():
            self.__event_send.wait()    # p一个send信号
            if (self.last_sent >= -1) and (self.last_sent < args.sending_window_size-1):
                self.last_sent += 1
                if self.sw_pdulist[self.last_sent].info != b'#':
                    send_pdu(self.sw_pdulist[self.last_sent], self.mybinding, self.target)
                    self.sw_timeouter[self.last_sent] = Timer(args.time_out/1000, self.__thread_timeout)    # 超时后执行超时线程
                    # print("%d已设置Timer"%self.last_sent)
                    self.sw_timeouter[self.last_sent].start()
            self.__event_send.clear()
            self.__event_endsend.set()

    def __thread_timeouter(self):
        while self.__running.is_set():
            self.__event_timer.wait()
            self.__event_timeout.wait()
            # 停止发送和滑动
            print("等待结束发送滑动线程\n", end='')
            self.__event_mainthread.clear()
            self.__event_endsend.wait()
            self.__event_endslide.wait()
            # 清理线程池
            for i in self.sw_timeouter:
                if i is not None:
                    i.cancel()
            print("计数器线程池清理完成\n", end='')
            self.last_sent = -1
            # 恢复主线程
            self.__event_timeout.clear()
            self.__event_mainthread.set()

    def __thread_timeout(self):
        # 超时一定是最先出发的超时，这样更安全
        if self.__event_timeout.is_set():
            print("已触发\n", end='')
            return
        else:
            self.__event_timeout.set()
            print("超时触发\n", end='')

    def __thread_slide(self):
        while self.__running.is_set():
            self.__event_slide.wait()
            # 文件未读完
            if self.fileptr < len(self.filelist) + args.sending_window_size - 1 and (len(self.sw_recvlist) > 0):
                # 是否滑动
                if self.sw_recvlist[0] in self.sw_nolist[0:self.last_sent+1]:
                    slide_times = self.sw_nolist.index(self.sw_recvlist[0], 0, self.last_sent+1) + 1
                    for i in range(slide_times):
                        self.success_sent += 1
                        print("%s slide seq=%d , num of succeeded packet:%d\n" %
                              (str(self.mybinding.getsockname()), self.sw_nolist[0], self.success_sent), end='')
                        # timer向右滑动一位
                        if self.sw_timeouter[0] is not None:
                            self.sw_timeouter[0].cancel()
                        self.sw_timeouter.pop(0)
                        self.sw_timeouter.append(None)
                        # nolist向右滑动一位
                        self.sw_nolist.pop(0)
                        next_add = (self.sw_nolist[-1] + 1) % self.max_no
                        self.sw_nolist.append(next_add)
                        self.last_sent -= 1
                        # PDU向右滑动一位
                        self.sw_pdulist.append(self.sw_pdulist.pop(0))

                        self.fileptr += 1
                        if self.fileptr + 1 > len(self.filelist):
                            self.sw_pdulist[-1].update(seq=self.sw_nolist[-1], info=b'#')
                        else:
                            self.sw_pdulist[-1].update(seq=self.sw_nolist[-1], info=self.filelist[self.fileptr])
            self.sw_recvlist.pop(0)
            self.__event_slide.clear()
            self.__event_endslide.set()


class RecvingWindow:
    """接收窗口"""
    def __init__(self, my_binding, sendto_sock):
        # 事件定义
        self.path = None
        self.file_out = None    # 每一个实例只能存一个文件，接收完就关闭。再发送需要再新建一个实例
        self.my_binding = my_binding
        self.target = sendto_sock
        self.__running = Event()
        # 主线程
        self.__thread_main = Thread(target=self.__main_thread)
        self.__event_mainthread = Event()
        # 上一条接收的seq
        self.lastseq = -1
        # 接收窗口
        self.seq_expected = 0
        # seq&info
        self.seq_and_info = []
        # 接收到的信息
        self.recv_info = []
        self.info_len_expected = 1000

    def start(self):
        '''开启主线程，主信号=True'''
        self.__running.set()
        self.__thread_main.start()
        self.__event_mainthread.set()


    def stop(self):
        self.__running.clear()
        self.__event_mainthread.clear()

    def __main_thread(self):
        while self.__running.is_set():
            self.__event_mainthread.wait()  # 此时主信号=True

            p = PDU(info=b'ACK PACKET')

            if len(self.seq_and_info) != 0:
                seq, info = self.seq_and_info.pop(0)

                # 去掉填充的字符
                info = info.rstrip(b'\x00')

                print("seq=%d exp=%d last=%d" % (seq, self.seq_expected, self.lastseq))

                if seq == self.seq_expected:
                    self.recv_info.append(info)

                    # 新开一个文件指针
                    if self.path is None:
                        # 如果是第一次收到这个端口发送的文件，则收到的是文件名
                        self.path = f'{dir_out}/books/{get_timestamp()}_{self.target[1]}'
                        os.makedirs(self.path, exist_ok=True)
                        self.file_out = info.decode().split('/')[-1].split('.')[0]
                    else:
                        file = f'{self.path}/{self.file_out}.txt'
                        with open(file, 'ab') as f:
                            f.write(info)

                    print("%s received seq=%d, num of received packet:%d\n" %
                          (self.my_binding.getsockname(), seq, len(self.recv_info)), end='')
                    self.lastseq = seq
                    self.seq_expected = (seq + 1) % args.max_sending_no
                    # 发送反馈
                p.update(ack=self.lastseq)
                send_pdu(p, self.my_binding, self.target)


    def get_seq_and_info(self, seq, info):
        self.seq_and_info.append((seq, info))