import errno
import os
import socket
import struct
import _pickle as cPickle
from sklearn.model_selection import train_test_split

from common_utils import *  # 修改了
from argparse import ArgumentParser
from collections import namedtuple
from typing import List, Dict
# from path_utils import get_prj_root
from datetime import datetime
from path_utils import get_prj_root
import numpy as np
import joblib
from sklearn.ensemble import RandomForestClassifier  # 训练模型
import pandas as pd
import time
import threading


def get_prj_root():
    return os.path.abspath(os.curdir)


data_name = "SB-F-202201051400"
big_percent = 0.01
instances_dir = os.path.join(get_prj_root(), "./data/instances/" + data_name + "/0.5s/" + str(big_percent) + "/")
model = None
features = []
keys = []
struct_len = 80

big_key = []

client_con = None


def load_model():
    # 加载模型
    model_file_name = "./data/model/0.5s/random_forest" + instances_dir.split("/")[-2] + ".pkl"
    with open(model_file_name, "rb") as fp:
        try:
            predict_model = cPickle.load(fp)
        except EOFError:
            print("模型为空")
    global model
    model = predict_model


def tcp_server():
    # s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # host = "127.0.0.1"
    host = "192.168.1.21"
    port = 8001
    s.bind((host, port))  # 绑定地址（host,port）到套接字

    s.listen(5)
    while True:
        print("Server> listen port...")
        con, address = s.accept()
        print(f"Server> 已与{address}建立连接")
        thd = threading.Thread(target=recv_feature_and_send_result, args=(con, address))
        thd.setDaemon(True)
        thd.start()


def tcp_client():
    ser_host = "192.168.1.81"
    ser_port = 8003
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((ser_host, ser_port))
    global client_con
    client_con = client


def send_result(con, flag):
    if flag:
        length = struct.pack('i', len(big_key) * 4 + 4)
    else:
        length = struct.pack('i', len(big_key) * 4)
    print(len(big_key))
    send_msg = b''
    if flag:
        send_msg += struct.pack('I', 0xFFFFFFFF)

    for key in big_key:
        send_msg += struct.pack('I', key)

    try:
        con.send(length)
        con.send(send_msg)
    except socket.error as e:
        if e.errno == errno.ECONNRESET:
            tcp_client()
            # Handle disconnection -- close & reopen socket etc.
            client_con.send(length)
            client_con.send(send_msg)
        else:
            client_con.close()
            tcp_client()
            client_con.send(length)
            client_con.send(send_msg)
            print("未知错误")
    # Other error, re-raise


def recv_feature_and_send_result(con, address):
    x = 0
    buffer = []
    while True:
        # 接收客户端传过来的数据
        recv_data = con.recv(4)  # data是接收到的数据 addr是对方的地址 也就是发送方的地址
        if recv_data:
            # print(len(recv_data))
            if len(recv_data) != 4:
                print("error")
            else:
                msg_size = struct.unpack("i", recv_data)[0]
                # print(msg_size)
                recv_msg = b''
                recv_size = 0
                while recv_size < msg_size:
                    # recv_msg += con.recv(1024)
                    recv_msg += con.recv(msg_size - recv_size)
                    recv_size = len(recv_msg)
                for i in range(len(recv_msg) // struct_len):
                    data = recv_msg[i * struct_len:(i + 1) * struct_len]
                    dlist = struct.unpack("5d9iI", data)
                    # print(dlist)
                    feature = [0 for i in range(11)]
                    feature[0] = dlist[0]
                    feature[1] = dlist[1]
                    feature[2] = dlist[5]
                    feature[3] = dlist[6]
                    feature[4] = dlist[7]
                    feature[5] = dlist[2]
                    feature[6] = dlist[3]
                    feature[7] = dlist[4]
                    feature[8] = dlist[8] / 1E6
                    feature[9] = dlist[9] / 1E6
                    feature[10] = dlist[10] / 1E6
                    feature.extend(dlist[11:14])
                    # print(feature)
                    # if dlist[5] != 333333:
                    #     print("parse error")
                    # else:
                    #     print("right")
                    features.append(feature)
                    keys.append(dlist[14])

                start = time.perf_counter()
                result = model.predict(features)
                # result = []
                for i, r in enumerate(result):
                    # if r == 1:
                    big_key.append(keys[i])
                    big_key.append(keys[i])
                # send_result(con)
                # send_result(client_con,True)
                elapsed = (time.perf_counter() - start)
                print("Time used:", elapsed)

                res = msg_size % struct_len
                if res != 0:
                    data = recv_msg[-1 * res:]
                    print(data.decode())
                    if data.decode() == "clear":
                        send_result(client_con, True)
                    else:
                        print("error data format")
                else:
                    send_result(client_con, False)

                features.clear()
                keys.clear()
                big_key.clear()
        else:
            print("con closed")
            con.close()
            break


if __name__ == '__main__':
    # recv_feature_and_send_result()
    tcp_client()
    load_model()
    tcp_server()
