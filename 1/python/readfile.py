from time import time
from args import args

data_size = args.data_size


def read_file(file):
    with open(file, 'rb') as f:
        while True:
            time.sleep(1e-2)    # significant
            data = f.read(data_size)
            binding_from.sendto(data, sock_to)
            if data == b'':
                break
