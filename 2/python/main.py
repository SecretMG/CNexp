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
    bindings = {}
    for name in names:
        socks[name] = (ip, ports[name])
        bindings[name] = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for name, binding in bindings.items():
        binding.bind(socks[name])

    my_node = DVNode(args.my_name, (ip, ports[args.my_name]))
    my_node.start(args.init_file)
    time.sleep(3)
    my_node.stop()


if __name__ == '__main__':
    main()