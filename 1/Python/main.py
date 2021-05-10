'''
@author: MRB
'''
import socket
import threading
import time
import os
from gbn import SendingWindow, RecvingWindow, recv_thread

from args import args
from utils import *
from gbn import *

file_in = 'resource/Harry Potter.txt'
dir_out = 'outputs'

ip = '127.0.0.1'  # 服务器ip和端口
port1, port2, port3, port4 = args.port1, args.port2, args.port3, args.port4
data_size = args.data_size



def get_sw(my_binding, sock_to, file):
    '读文件，调用SW发送，返回一个SenderWindow实例'
    content = [file.encode()]
    with open(file, 'rb') as f:
        while True:
            # time.sleep(1e-2)    # significant
            data = f.read(data_size)
            content.append(data)
            my_binding.sendto(data, sock_to)
            if data == b'':
                break
    print("共发送%d个包" % len(content))
    sw = SendingWindow(my_binding, sock_to, content)
    return sw

def receiver(name, my_binding, sws):
    '接下来在每个receiver线程中，每接收到一个新端口的包就在线程中建立一个新的receiver_window'
    '注意需要知道本端口所有的sender_windows'
    for port in sws:
        sws[port].start()  # 开启传输窗口
    all_ports = {}  # 记录是否是一个新端口向我方传输文件
    rws = []  # 本端口为每个信道对岸都建立一个rw
    cnt = 0
    while True:
        cnt += 1
        print(cnt)
        binpack, sender_ip = my_binding.recvfrom(PDU.size)  # 仅接收PDU帧的大小
        seq, ack, info = unpack_pdu(binpack)  # 解码数据包
        '''判断有无问题'''
        # 丢包
        if loss_with_rate(loss_rate):
            print(f'Loss: seq={seq}, ack={ack}\n')
            continue
        # 坏包
        if not crc_check(binpack):
            print(f'check crc error: seq={seq}, ack={ack}\n')
            continue
        '''此时可以认为是正确的数据包'''
        port = sender_ip[1]
        if port not in all_ports:
            rws[port] = RecvingWindow(my_binding, (ip, port))
            rws[port].start()

        # ack包
        if seq == -1:
            print('\nack\n')    # noshow
            sws[port].get_ack(ack)
        # info包
        if ack == -1:
            print('\ninfo\n')   # noshow
            rws[port].get_seq_and_info(seq, info)

        # 拼接字符串
        str = b''.join(rws[port].recv_info)
        print(str)

        # context = data[0]
        # _, new_port = data[1]
        # if new_port not in all_ports:
        #     # 新开一个文件指针
        #     os.makedirs(f'{dir_out}/books/{name}', exist_ok=True)
        #     all_ports[new_port] = f'{dir_out}/books/{name}/{get_timestamp()}_{str(new_port)}.txt'
        # with open(all_ports[new_port], 'ab') as f:
        #     f.write(context)

def main():

    sock1 = (ip, port1)
    sock2 = (ip, port2)
    sock3 = (ip, port3)
    sock4 = (ip, port4)

    binding1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    binding2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    binding3 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    binding4 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    binding1.bind(sock1)
    binding2.bind(sock2)
    binding3.bind(sock3)
    binding4.bind(sock4)

    """
    alice_server = threading.Thread(target=receiver, args=(binding1, 'alice'))
    bob_server = threading.Thread(target=receiver, args=(binding2, 'bob'))
    carl_server = threading.Thread(target=receiver, args=(binding3, 'carl'))
    david_server = threading.Thread(target=receiver, args=(binding4, 'david'))
    alice_client = threading.Thread(target=get_sw, args=(binding1, sock2, file_in))
    bob_client = threading.Thread(target=get_sw, args=(binding2, sock1, file_in))
    carl_client = threading.Thread(target=get_sw, args=(binding3, sock4, file_in))
    david_client = threading.Thread(target=get_sw, args=(binding4, sock2, file_in))

    # alice_server.start(), alice_client.start()
    # bob_server.start(), bob_client.start()
    # carl_server.start(), carl_client.start()
    # david_server.start(), david_client.start()
    """

    alice_sws, bob_sws, carl_sws, david_sws = {}, {}, {}, {}
    alice_sws[port2] = get_sw(binding1, sock2, file_in)     # a to b
    alice_sws[port3] = get_sw(binding1, sock3, file_in)    # a to c
    bob_sws[port1] = get_sw(binding2, sock1, file_in)     # b to a

    alice_server = threading.Thread(target=receiver, args=('alice', binding1, alice_sws))
    bob_server = threading.Thread(target=receiver, args=('bob', binding2, bob_sws))
    alice_server.start()
    bob_server.start()

    # 全部开始

    # 还没有结束处理，所以只能强行停止 todo：改进结束处理
    time.sleep(90)
    # ab_sw.stop()
    # bob_from_alice_rw.stop()
    # 还没办法停止receiver todo:重写receiver




if __name__ == '__main__':
    main()

