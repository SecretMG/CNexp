'''
@author: MRB
'''
import socket
import threading
import time
import os
from gbn import GbnProtocol

from args import args
from utils import *

file_in = 'resource/Harry Potter.txt'
dir_out = 'outputs'

ip = '127.0.0.1'  # 服务器ip和端口
port1, port2, port3, port4 = args.port1, args.port2, args.port3, args.port4
data_size = args.data_size

def sender(binding_from, sock_to, file):
    with open(file, 'rb') as f:
        while True:
            time.sleep(1e-2)    # significant
            data = f.read(data_size)
            binding_from.sendto(data, sock_to)
            if data == b'':
                break
    print('sender close')


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

    file1 = ['023a', '134e', '24567y', '34dsfa', '41234s', '52134', '61234ss', 'eeenD', 'ax]=========']
    file2 = ['123123', '2341234', '3到底是谁', '4ADSF']

    a = GbnProtocol(binding1, sock2, file1)
    b = GbnProtocol(binding2, sock1, file2)

    # 双工
    a.mode = 0
    b.mode = 0

    # 单工
    # a.mode = 1
    # b.mode = 2

    a.start()
    time.sleep(0.5)
    b.start()



'''
    alice_server = threading.Thread(target=receiver, args=(binding1, 'alice'))
    bob_server = threading.Thread(target=receiver, args=(binding2, 'bob'))
    carl_server = threading.Thread(target=receiver, args=(binding3, 'carl'))
    david_server = threading.Thread(target=receiver, args=(binding4, 'david'))
    alice_client = threading.Thread(target=sender, args=(binding1, sock2, file_in))
    bob_client = threading.Thread(target=sender, args=(binding2, sock1, file_in))
    carl_client = threading.Thread(target=sender, args=(binding3, sock4, file_in))
    david_client = threading.Thread(target=sender, args=(binding4, sock2, file_in))

    alice_server.start(), alice_client.start()
    bob_server.start(), bob_client.start()
    carl_server.start(), carl_client.start()
    david_server.start(), david_client.start()
'''

if __name__ == '__main__':
    main()

