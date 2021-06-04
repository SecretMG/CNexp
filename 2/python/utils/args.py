import argparse
parser = argparse.ArgumentParser()
parser.add_argument(
    'my_name',
    help='本节点的名字（ID）'
)
parser.add_argument(
    'init_file',
    help='本节点的初始化文件'
)
parser.add_argument(
    '--port1', '-p1',
    default=42328,
    help='UDP 端口1'
)
parser.add_argument(
    '--port2', '-p2',
    default=42329,
    help='UDP 端口2'
)
parser.add_argument(
    '--port3', '-p3',
    default=42330,
    help='UDP 端口3'
)
parser.add_argument(
    '--port4', '-p4',
    default=42331,
    help='UDP 端口4'
)
parser.add_argument(
    '--port5', '-p5',
    default=42332,
    help='UDP 端口5'
)
parser.add_argument(
    '--port6', '-p6',
    default=42333,
    help='UDP 端口6'
)
parser.add_argument(
    '--port7', '-p7',
    default=42334,
    help='UDP 端口7'
)
parser.add_argument(
    '--port8', '-p8',
    default=42335,
    help='UDP 端口8'
)
parser.add_argument(
    '--port9', '-p9',
    default=42336,
    help='UDP 端口9'
)
parser.add_argument(
    '--Unreachable', '-UNR',
    default=20.0,
    help='不可达距离'
)
parser.add_argument(
    '--Frequency', '-F',
    default=5000,
    help='定期时间间隔（ms）'
)
parser.add_argument(
    '--MaxValidTime', '-MVT',
    default=15000,
    help='最大等待时间（ms）'
)
parser.add_argument(
    '--FrameSize', '-FS',
    default=1024,
    help='接受帧的最大长度'
)
args = parser.parse_args()