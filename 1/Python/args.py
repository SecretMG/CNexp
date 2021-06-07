'''
@author: MRB
'''
import argparse
parser = argparse.ArgumentParser()
parser.add_argument(
    '--port1', '-p1',
    default=42328,
    help='UDP 端口1'
)   # UDP端口1
parser.add_argument(
    '--port2', '-p2',
    default=42329,
    help='UDP 端口2'
)   # UDP端口2
parser.add_argument(
    '--port3', '-p3',
    default=42330,
    help='UDP 端口3'
)   # UDP端口3
parser.add_argument(
    '--port4', '-p4',
    default=42331,
    help='UDP 端口4'
)   # UDP端口4
parser.add_argument(
    '--data_size', '-ds',
    default=16384,
    help='PDU 中数据字段的长度，单位为字节'
)   # PDU(Protocol Data Unit)中数据字段的大小：默认为1024字节
parser.add_argument(
    '--error_rate', '-er',
    default=10,
    help='PDU 错误率'
)   # 每10帧中有1帧出错
parser.add_argument(
    '--loss_rate', '-lr',
    default=10,
    help='PDU 丢失率'
)   # 每10帧中有1帧丢失
parser.add_argument(
    '--max_sending_no', '-msn',
    default=5,
    help='最大序号数'
)
parser.add_argument(
    '--sending_window_size', '-sws',
    default=4,
    help='发送窗口大小'
)
parser.add_argument(
    '--init_no', '-in',
    default=0,
    help='起始 PDU 的序号'
)
parser.add_argument(
    '--time_out', '-to',
    default=1500,
    help='超时定时器值，单位为毫秒'
)   # 超时时间为1秒
args = parser.parse_args()