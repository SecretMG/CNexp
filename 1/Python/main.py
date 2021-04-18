import socket
import threading
import time

from args import args
from utils import *

file_in = 'resource/Harry Potter.txt'
dir_out = 'outputs'

ip = '127.0.0.1'  # 服务器ip和端口
port1, port2, port3, port4 = args.port1, args.port2, args.port3, args.port4
data_size = args.data_size

def sender(sock_from, sock_to, file):
    from_binding = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    from_binding.bind(sock_from)
    with open(file, 'rb') as f:
        while True:
            time.sleep(0.001)
            # 经验证，组织成的数据块数量没问题
            data = f.read(data_size)
            from_binding.sendto(data, sock_to)
            if data == b'':
                break
    from_binding.close()
    print('sender close')


def receiver(sock_to):
    to_binding = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    to_binding.bind(sock_to)
    all_ports = {}  # 记录是否是一个新端口向我传输文件

    cnt = 0
    while True:
        data = to_binding.recvfrom(data_size)
        print(cnt)
        cnt += 1
        context = data[0]
        _, new_port = data[1]
        if new_port not in all_ports:
            # 新开一个文件指针
            all_ports[new_port] = f'{dir_out}/books/{get_timestamp()}_{str(new_port)}.txt'
        with open(all_ports[new_port], 'ab') as f:
            f.write(context)

def main():

    sock1 = (ip, port1)
    sock2 = (ip, port2)
    sock3 = (ip, port3)
    sock4 = (ip, port4)

    server = threading.Thread(target=receiver, args=(sock1, ))
    bob = threading.Thread(target=sender, args=(sock2, sock1, file_in))
    carl = threading.Thread(target=sender, args=(sock3, sock1, file_in))
    david = threading.Thread(target=sender, args=(sock4, sock1, file_in))

    server.start()
    bob.start()
    # carl.start()
    # david.start()

if __name__ == '__main__':
    main()

