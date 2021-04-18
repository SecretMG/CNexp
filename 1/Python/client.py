'''
@author: MRB
'''
import socket
from args import args
from pdu import PDU, unpack_pdu, crc_check

port = args.port
data_size = args.data_size

class Client():
    def __init__(self):
        pass
    def run(self):
        pass
    def close(self):
        pass


"""
while True:
    trigger = input('Input: ')
    s.sendto(trigger.encode(), ('127.0.0.1', port))
    data, addr = s.recvfrom(data_size)  # 返回数据和接入连接的（服务端）地址
    data = data.decode()
    print('[Recieved]', data)
    if trigger == '###':  # 自定义结束字符串
        break
s.close()
"""

# send_frame = PDU(1, 2, 'boblasdfs啊')
#
# s.sendto(send_frame.bin_pack, ('127.0.0.1', port))
# data, addr = s.recvfrom(PDU.size)  # 返回数据和接入连接的（服务端）地址
# if crc_check(data):
#     ack, seq, info = unpack_pdu(data)
#     print(ack, seq, info.decode())
# s.close()
