import socket
import threading
import time

from args import args

ip = '127.0.0.1'  # 服务器ip和端口
port1, port2, port3, port4 = args.port1, args.port2, args.port3, args.port4
data_size = args.data_size

def sender(sock_from, sock_to, file):
    data = 'here'.encode()
    from_binding = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    from_binding.bind(sock_from)
    while True:
        from_binding.sendto(data, sock_to)
        time.sleep(5)

def receiver(sock_to):
    to_binding = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    to_binding.bind(sock_to)
    while True:
        time.sleep(1)
        data = to_binding.recvfrom(data_size)
        context = data[0].decode()
        other_ip, other_port = data[1]
        print('收到信息:', context, other_ip, other_port)


def main():

    sock1 = (ip, port1)
    sock2 = (ip, port2)
    sock3 = (ip, port3)
    sock4 = (ip, port4)

    alice = threading.Thread(target=receiver, args=(sock1, ))
    bob = threading.Thread(target=sender, args=(sock2, sock1, 'ok'))
    carl = threading.Thread(target=sender, args=(sock3, sock1, 'ok'))
    david = threading.Thread(target=sender, args=(sock4, sock1, 'ok'))

    alice.start()
    bob.start()
    carl.start()
    david.start()

if __name__ == '__main__':
    main()

