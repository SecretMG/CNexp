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

    def catch(self, subject, event):
        event_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
        print('[%s]\t%s\t%s' % (event_time, subject, event))
        self.log_list.append((event_time, subject, event))


a = Logs()

a.catch('mike', 'send pdu')