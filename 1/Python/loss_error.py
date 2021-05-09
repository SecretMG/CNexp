# encoding: utf-8
# author: Zheng Zhihan

"""
功能实现：
丢包
误包

"""

from random import random


def loss_with_rate(rate):
    """
    丢包
    :param rate: 丢包率
    :return:T/F
    """
    if random() <= rate:
        return True
    else:
        return False


def error_with_rate(rate, binpack):
    """
    误包
    :param rate:误包率
    :return: bin_pack
    """
    if random() <= rate:
        rt = bytearray(binpack)
        rt[0] ^= 0xFF   # 取反
        return rt
    else:
        return binpack
