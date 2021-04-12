'''
@author: MRB
'''
import socket
from args import args

port = args.port
data_size = args.data_size


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


while True:
    trigger = input('Input: ')
    s.sendto(trigger.encode(), ('127.0.0.1', port))
    data, addr = s.recvfrom(data_size)  # 返回数据和接入连接的（服务端）地址
    data = data.decode()
    print('[Recieved]', data)
    if trigger == '###':  # 自定义结束字符串
        break
s.close()
