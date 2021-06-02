import os.path
import socket

from utils.args import args


def setup(ports):
    my_RoutingTable = {}
    Unreachable = args.Unreachable
    dir_folder = 'utils/params'
    my_ID, my_File = args.my_ID, args.my_File
    file_name = os.path.join(dir_folder, my_File)
    with open(file_name) as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip()
            if line:
                neighbor, dist, port = line.split()
                dist, port = int(dist), int(port)   # 除名字外都使用int类型
                my_RoutingTable[neighbor] = (dist, port)
    print(my_RoutingTable)



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

    setup(ports)



if __name__ == '__main__':
    main()