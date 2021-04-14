'''
@author: MRB
'''
import socket
from args import args
from pdu import PDU, unpack_pdu, crc_check

port = args.port
data_size = args.data_size


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('127.0.0.1', port))

"""
while True:
    data, addr = s.recvfrom(data_size)  # 返回数据和接入连接的（客户端）地址
    data = data.decode()
    if not data:
        break
    print('[Received]', data)
    send = input('Input: ')
    s.sendto(send.encode(), addr)  # UDP 是无状态连接，所以每次连接都需要给出目的地址
s.close()
"""

data, addr = s.recvfrom(PDU.size)  # 返回数据和接入连接的（客户端）地址
print(len(data))
if crc_check(data):
    ack, seq, info = unpack_pdu(data)
    print(ack, seq, info)
send_frame = PDU(0, 1, 'alicesay01')
s.sendto(send_frame.bin_pack, addr)  # UDP 是无状态连接，所以每次连接都需要给出目的地址
s.close()
