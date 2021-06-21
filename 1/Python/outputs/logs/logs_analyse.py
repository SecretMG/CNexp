# -*- coding: utf-8 -*-
"""
@author: Zheng Zhihan
@file: logs_analyse.py
@time: 2021/06/06
@description: 
"""

import os
import json
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime

def open_ast(path):
    with open(path, 'r') as f:
        try:
            dict_ast = json.load(f)
        except Exception:
            print('ERR: load json failed')
            return False
        else:
            return dict_ast


def show_pack(results: dict, category_names: list, title: str):
    labels = list(results.keys())
    data = np.array(list(results.values()))

    data_cum = data.cumsum(axis=1)
    # category_colors = plt.get_cmap('RdYlGn')(
        # np.linspace(0.15, 0.85, data.shape[1]))

    category_colors = ['lightsteelblue', 'orange', 'aqua', 'green', 'red']

    fig, ax = plt.subplots(figsize=(9.2, 10))
    ax.invert_yaxis()
    ax.xaxis.set_visible(False)
    ax.set_xlim(0, np.sum(data, axis=1).max())

    for i, (colname, color) in enumerate(zip(category_names, category_colors)):
        widths = data[:, i]
        starts = data_cum[:, i] - widths
        rects = ax.barh(labels, widths, left=starts, height=0.5,
                        label=colname, color=color)

        text_color = 'black'
        ax.bar_label(rects, label_type='center', color=text_color)
    ax.set_title(title)
    ax.legend(ncol=len(category_names), bbox_to_anchor=(0, 1),
              loc='upper left', fontsize='small')

    return fig, ax


def show_time(dict_data: dict, title: str):
    labels = list(dict_data.keys())
    data = np.array(list(dict_data.values()))
    y_pos = np.arange(len(labels))

    fig, ax = plt.subplots()
    ab = ax.barh(y_pos, data, align='center')
    ax.bar_label(ab, label_type='edge', color='black')
    ax.set_yticks(y_pos)
    ax.set_yticklabels(labels)
    ax.invert_yaxis()
    ax.set_xlabel('time(s)')
    ax.set_title(title)


# 打开ast
file_list = []
for root, dirs, files in os.walk('./'):
    for file in files:
        if os.path.splitext(file)[1] == '.json':
            file_list.append(os.path.join(root, file))

dict_list = []
for path in file_list:
    dict_list.append(open_ast(path))

send_logs = {}
recv_logs = {}
send_times = {}
recv_times = {}

index = 0
for logs in dict_list:
    index += 1

    start_time = datetime.strptime(logs[0]['time'], '%Y-%m-%d %H:%M:%S.%f')
    finish_time = datetime.strptime(logs[-1]['time'], '%Y-%m-%d %H:%M:%S.%f')
    run_time = finish_time - start_time

    data = [0, 0, 0, 0, 0]
    for log in logs:
        if log['status'] == 'NEW':
            data[0] += 1
        elif log['status'] == 'TO':
            data[1] += 1
        elif log['status'] == 'ACK':
            data[2] += 1
        elif log['status'] == 'OK':
            data[3] += 1
        elif log['status'] == 'RT':
            data[4] += 1
    name = f'#{index}'
    if index % 2 == 1:
        send_logs[name] = data
        send_times[name] = run_time.total_seconds()
    else:
        recv_logs[name] = data
        recv_times[name] = run_time.total_seconds()
    # print(index, len(logs))

category_names = ['NEW', 'TO', 'ACK', 'OK', 'RT']

show_pack(send_logs, category_names, 'Sending Window Packet')
plt.show()

show_pack(recv_logs, category_names, 'Receiving Window Packet')
plt.show()

show_time(send_times, 'Sending Window Time Cost')
plt.show()

show_time(recv_times, 'Receiving Window Time Cost')
plt.show()
