# encoding: utf-8
# author: Zheng Zhihan

"""
功能实现：
日志文件输出

"""
import datetime
from pdu import PDU


class Logs:

    def __init__(self):
        self.log_list = []

    def send_catch(self, subject, sendto, seq, ack, status):
        log = {
            'time': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'),
            'type': 'SEND',
            'subject': subject,
            'sendto': sendto,
            'send_seq': seq,
            'ack': ack,
            'status': status
        }
        # print(log)
        self.log_list.append(log)

    def recv_catch(self, subject, recvfrom, seq, ack, status):
        log = {
            'time': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'),
            'type': 'RECV',
            'subject': subject,
            'recvfrom': recvfrom,
            'send_seq': seq,
            'ack': ack,
            'status': status
        }
        # print(log)
        self.log_list.append(log)

    def err_catch(self, err_type, subject, recvfrom, seq, ack, status):
        log = {
            'time': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f'),
            'type': err_type,
            'subject': subject,
            'recvfrom': recvfrom,
            'send_seq': seq,
            'ack': ack,
            'status': status
        }
        print(log)
        self.log_list.append(log)
