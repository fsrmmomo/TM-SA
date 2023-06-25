"""
预测函数：随机森林
8个特征的（前五个包大小，后五个包间隔）：最小值，最大值，平均值，方差
每次运行前，检查：
四个需要修改的地方，命名是否正确
最后的运行模式是否正确
"""
import random

import _pickle as cPickle
import joblib
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
from sklearn import metrics
import pickle
import pickle
# from imblearn.over_sampling import SMOTE
# from imblearn.under_sampling import RandomUnderSampler
from collections import Counter
from sklearn.linear_model import SGDClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import GridSearchCV

start = time.time()  # 计算时间

random.seed(datetime.now())
# model_dir = os.path.join(get_prj_root(), "classify/model_predict") #修改：模型model文件夹路径
# predict_model_pkl = os.path.join(model_dir, "dt3_1_9.pkl") #修改：模型的版本，只用修改此处就行

Instance = namedtuple("Instance", ["features", "label", "id"])  # 实例

dirs = {
    "video": "./tmp/dt/video",
    "iot": "./tmp/dt/iot",
    "car": "./tmp/dt/car",
}
# instances_dir = os.path.join(get_prj_root(), "./classify/instances2")  # 修改：instances路径
# instances_dir = os.path.join(get_prj_root(), "./even/instances")  # 修改：instances路径
# instances_dir = os.path.join(get_prj_root(), "./data/instances/")  # 修改：instances路径

my_data_list = []
my_test_data = []
my_test_model = None


def train_and_predict(instances_dir):
    my_data_list.clear()
    data_list = []
    print(instances_dir)
    for file in os.listdir(instances_dir):
        if file[-3:] == "pkl":
            data_list = data_list + load_pkl(instances_dir + file)
        for l in data_list:
            my_data_list.extend(l)
        data_list = my_data_list
    print(len(data_list))
    n_train = int(len(data_list) * 0.7)
    d_train, d_test = train_test_split(data_list, train_size=0.7, random_state=10, shuffle=True)

    train = d_train
    train_x = [x.features for x in train]
    train_y = [x.label for x in train]
    # ===============================
    test = d_test
    test_x = [t.features for t in test]
    test_y = [t.label for t in test]
    predict_model = RandomForestClassifier(n_jobs=-1,n_estimators=10)  # 引入训练方法
    print("正在拟合")
    start = time.time()  # 计算时间
    predict_model.fit(train_x, train_y)  # 对训练数据进行拟合
    elapsed = (time.time() - start)
    print("Time used:", elapsed)

    predicts = predict_model.predict(test_x)

    acc = predict_model.score(test_x, test_y)  # 根据给定数据与标签返回正确率的均值
    print('决策树模型评价:', acc)
    print('决策树模型评价:', acc)
    print("ACC {:.2f}%".format(metrics.accuracy_score(test_y, predicts) * 100))

    print("Precision {:.2f}%".format(metrics.precision_score(test_y, predicts) * 100))
    print("recall {:.2f}%".format(metrics.recall_score(test_y, predicts) * 100))
    print("F1 {:.2f}%".format(metrics.f1_score(test_y, predicts) * 100))






if __name__ == '__main__':

    big_percent = 0.1
    instance_dir = "../newtrace/instance/"
    for instance_subdir in os.listdir(instance_dir):
        instance_file = instance_dir + instance_subdir +"/"+ str(big_percent)+"/"
        # print(instance_file)
        train_and_predict(instance_file)





    # n = 1
    # # for i in range(n):
    # # save_model()
    #
    # # train_and_predict()
    # # big_list = [0.05, 0.1, 0.2, 0.3]
    # # big_list = [0.1, 0.2, 0.3]
    # big_list = [0.05,0.1]
    #
    # for big_percent in big_list[:]:
    #     print("processing b={}:".format(big_percent))
    #     data_name = "SB-F-202201051400"
    #     instances_dir = os.path.join(get_prj_root(), "./data/instances/" + data_name + "/" + str(big_percent) + "/")
    #     instances_dir = os.path.join(get_prj_root(), "./data/instances/" + data_name + "/1s/" + str(big_percent) + "/")
    #     instances_dir = os.path.join(get_prj_root(),
    #                                  "./data/instances/" + data_name + "/0.5s/" + str(big_percent) + "/")
    #     instances_dir = "extra/instance/"+ str(big_percent) + "/1/"
    #     # instances_dir = "extra/instance/" + str(big_percent) + "/2/"
    #     train_and_predict(instances_dir)
    #     # test_classify(instances_dir)
    #
    #     data_name = "SB-F-202201021400"
    #     data_name = "SB-F-202201041400"
    #     test_instances_dir = os.path.join(get_prj_root(),
    #                                       "./data/instances/" + data_name + "/" + str(big_percent) + "/")
    #     # test_classify(test_instances_dir)
