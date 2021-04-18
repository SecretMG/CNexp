import time


def get_timestamp():
    ts = int(round(time.time() * 1000))
    ts = time.strftime('%Y-%m-%d-%H-%M-%S', time.localtime(ts / 1000))
    return ts

