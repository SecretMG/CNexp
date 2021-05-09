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

file_in = 'resource/Harry Potter.txt'
dir_out = 'outputs'

ip = '127.0.0.1'  # 服务器ip和端口
port1, port2, port3, port4 = args.port1, args.port2, args.port3, args.port4
data_size = args.data_size


def sender(binding_from, sock_to, file):
    content = [file.encode()]
    with open(file, 'rb') as f:
        while True:
            # time.sleep(1e-2)    # significant
            data = f.read(data_size)
            content.append(data)
            # binding_from.sendto(data, sock_to)
            if data == b'':
                break
    print("共发送%d个包" % len(content))
    sw = SendingWindow(binding_from, sock_to, content)
    print('sender close')
    return sw


def receiver(binding_to, name):
    all_ports = {}  # 记录是否是一个新端口向我方传输文件
    cnt = 0
    while True:
        data = binding_to.recvfrom(data_size)
        if cnt % 1000 == 0:
            print(f'已接收{cnt}帧\n')
        cnt += 1
        context = data[0]
        _, new_port = data[1]
        if new_port not in all_ports:
            # 新开一个文件指针
            os.makedirs(f'{dir_out}/books/{name}', exist_ok=True)
            all_ports[new_port] = f'{dir_out}/books/{name}/{get_timestamp()}_{str(new_port)}.txt'
        with open(all_ports[new_port], 'ab') as f:
            f.write(context)

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
    alice_client = threading.Thread(target=sender, args=(binding1, sock2, file_in))
    bob_client = threading.Thread(target=sender, args=(binding2, sock1, file_in))
    carl_client = threading.Thread(target=sender, args=(binding3, sock4, file_in))
    david_client = threading.Thread(target=sender, args=(binding4, sock2, file_in))

    # alice_server.start(), alice_client.start()
    # bob_server.start(), bob_client.start()
    # carl_server.start(), carl_client.start()
    # david_server.start(), david_client.start()
    """

    # a的发送窗口
    alice_to_bob_sw = sender(binding1, sock2, file_in)
    # a的接收线程 这里用的是gbn中的receiver
    alice_receiver = threading.Thread(target=recv_thread, args=(binding1, alice_to_bob_sw, None))
    # b的接收窗口
    bob_from_alice_rw = RecvingWindow(binding2, sock1)
    # b的接收线程 这里用的是gbn中的receiver
    bob_receiver = threading.Thread(target=recv_thread, args=(binding2, None, bob_from_alice_rw))

    # 全部开始
    alice_to_bob_sw.start()
    alice_receiver.start()
    bob_from_alice_rw.start()
    bob_receiver.start()

    # 还没有结束处理，所以只能强行停止 todo：改进结束处理
    time.sleep(90)
    alice_to_bob_sw.stop()
    bob_from_alice_rw.stop()
    # 还没办法停止receiver todo:重写receiver

    # 拼接字符串
    str = b''.join(bob_from_alice_rw.recv_info)
    print(str.decode())


if __name__ == '__main__':
    main()

