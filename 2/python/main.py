import os.path
import socket
import time

from utils.args import args
from DVNode import DVNode





def main():
    ip = '127.0.0.1'
    names = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i']
    ports = {
        'a': args.port1,
        'b': args.port2,
        'c': args.port3,
        'd': args.port4,
        'e': args.port5,
        'f': args.port6,
        'g': args.port7,
        'h': args.port8,
        'i': args.port9
    }
    socks = {}
    for name in names:
        socks[name] = (ip, ports[name])
    my_node = DVNode(args.my_name, socks[args.my_name], socks)
    my_node.start(args.init_file)

    while True:
        command = input()
        if command == 'k':
            break
        if command == 'p':
            my_node.pause()
        if command == 's':
            my_node.restart(args.init_file)
    logs = my_node.stop()

    dir_logs = 'logs'
    os.makedirs(dir_logs, exist_ok=True)
    log_file = os.path.join(dir_logs, args.my_name + '.txt')
    with open(log_file, 'a') as f:
        for log in logs:
            f.write(log)


if __name__ == '__main__':
    main()