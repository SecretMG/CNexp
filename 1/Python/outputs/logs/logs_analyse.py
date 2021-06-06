# -*- coding: utf-8 -*-
"""
@author: Zheng Zhihan
@file: logs_analyse.py
@time: 2021/06/06
@description: 
"""

import os
import json


def open_ast(path):
    with open(path, 'r') as f:
        try:
            dict_ast = json.load(f)
        except Exception:
            print('ERR: load json failed')
            return False
        else:
            return dict_ast


# 打开ast

file_list = []
for root, dirs, files in os.walk('./'):
    for file in files:
        if os.path.splitext(file)[1] == '.json':
            file_list.append(os.path.join(root, file))

dict_list = []
for path in file_list:
    dict_list.append(open_ast(path))

for i in dict_list:
    print(len(i))
