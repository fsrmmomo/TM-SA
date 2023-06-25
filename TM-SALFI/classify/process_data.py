import pickle
import struct
import sys
from collections import namedtuple

import pyshark

from common_utils import save_pkl
from packet_struct import *
import os

Instance = namedtuple("Instance", ["features", "label", "id"])  # 实例


def pcap_parse(rf, wf, shift=0):
    # fpcap = open('../../Data/raw_data/MAWI/test.pcap', 'rb')

    # rf = '../../Data/raw_data/MAWI/test.pcap'
    # wf = '../../Data/raw_data/MAWI/test.dat'
    # wf = open('../../Data/raw_data/MAWI/test.dat','wb')
    wf = open(wf, 'wb')
    packet_num = 0
    trace_byte_size = 16
    pkt_list = []
    start = -1

    with open(rf, 'rb') as f:
        print("打开了pcap")
        tmp = f.read(24)

        bin_trace = f.read(trace_byte_size)
        # print(" ".join([hex(int(i)) for i in bin_trace]))
        x = 0

        while bin_trace:
            x += 1
            # print(x)
            hex_trace = struct.unpack("IIII", bin_trace)
            GMTtime = hex_trace[0]
            MicroTime = hex_trace[1]
            caplen = hex_trace[2]
            plen = hex_trace[3]

            # data = f.read(plen)
            data = f.read(caplen)
            bin_trace = f.read(trace_byte_size)
            # print(" ".join([hex(int(i)) for i in bin_trace]))

            # print(caplen)

            ipProto = data[12 + shift:14 + shift]
            if len(data) < 38:
                continue

            if ipProto != b'\x08\x00':
                # print("   不是ipv4")
                continue
            else:
                if int(data[23 + shift]) != 6 and int(data[23 + shift]) != 17:
                    # print("不是tcpudp")
                    # print(hex(data[23]))
                    continue
                # p = packet_struct()
                packet_num += 1
                time = 0
                if start != -1:
                    time = GMTtime + MicroTime / 1000000.0 - start
                else:
                    start = GMTtime + MicroTime / 1000000.0
                savep = b''
                src_ip = data[26 + shift:30 + shift]
                dst_ip = data[30 + shift:34 + shift]
                trans_proto = data[23 + shift]
                src_port = data[34 + shift:36 + shift]
                dst_port = data[36 + shift:38 + shift]
                t = struct.pack("BId", trans_proto, plen, time)

                # print(time)

                # print(" ".join([hex(int(i)) for i in savep]))
                # print(packet_num)

                savep = struct.pack("BBBBHBBBBHBId", int(src_ip[0]), int(src_ip[1]), int(src_ip[2]), int(src_ip[3]),
                                    int(src_port[0] * 256 + src_port[1]),
                                    int(dst_ip[0]), int(dst_ip[1]), int(dst_ip[2]), int(dst_ip[3]),
                                    int(dst_port[0] * 256 + dst_port[1]),
                                    trans_proto,
                                    plen,
                                    time
                                    )

                pkt_list.append(savep)
                if len(pkt_list) > 1000000:
                    print(packet_num)
                    for pkt in pkt_list:
                        wf.write(pkt)
                    pkt_list = []
    for pkt in pkt_list:
        pass
        wf.write(pkt)
    print(packet_num)
    wf.close()


#               0     1     2           3           4           5           6           7           8         9         10      11          12      13
# feature = [总Bytes,包数, max bytes , min bytes, aver bytes, start windows,end win, start time, end time, max twin, min twin, aver twin, dst port, src port]
def insert_p(p, tmp_feature, key_map, now_win):
    key = p.src_ip + p.src_port + p.dst_ip + p.dst_port + p.proto
    # key = p.src_ip + " " + p.src_port + " " + p.dst_ip + " " + p.dst_port + " " + p.proto
    # key_map = {}
    # tmp_feature = {}
    length = p.length
    now_time = p.time
    map_key = None
    if key in key_map.keys():
        key_map[key][2] += 1
        key_map[key][1] += length
        map_key = key_map[key][0]
    else:
        map_key = len(key_map)
        # key_map[key] = [map_key, 1]
        # key_map[key] = [map_key, length]
        key_map[key] = [map_key, length, 1]

    if map_key in tmp_feature.keys():
        last_feature = tmp_feature[map_key]
        last_feature[0] += length
        last_feature[1] += 1
        last_feature[2] = max(last_feature[2], length)
        last_feature[3] = min(last_feature[3], length)
        last_feature[4] = ((last_feature[4] * (last_feature[1] - 1)) + length) / last_feature[1]
        last_feature[6] = now_win
        twin = now_time - last_feature[8]
        last_feature[8] = now_time
        last_feature[9] = max(twin, last_feature[9])
        last_feature[10] = min(twin, last_feature[10])
        last_feature[11] = ((last_feature[11] * (last_feature[1] - 2)) + twin) / (last_feature[1] - 1)
    else:
        # last_feature = [0 for _ in range(12)]
        last_feature = [0 for _ in range(14)]
        last_feature[0] += length
        last_feature[1] += 1
        last_feature[2] = length
        last_feature[3] = length
        last_feature[4] = length
        last_feature[5] = now_win
        last_feature[6] = now_win
        # twin = now_time - last_feature[8]
        last_feature[7] = now_time
        last_feature[8] = now_time
        # last_feature[9] = max(twin,last_feature[9])
        last_feature[10] = sys.maxsize
        # last_feature[11] = ((last_feature[11] * (last_feature[1] - 2)) + twin) / (last_feature[1] - 1)

        last_feature[12] = int(p.src_port)
        last_feature[13] = int(p.dst_port)
        # last_feature.append(int(p.src_port))
        # last_feature.append(int(p.dst_port))
        tmp_feature[map_key] = last_feature


def read_dat(rf, pre, wf, big_percent):
    # rf = "./extra/univ/dat/univ2_pt0.dat"
    # pre = "./extra/univ/univ2_pt0/"
    # wf = "./extra/univ/univ2_pt0/"+ str(big_percent) + '/'
    trace_byte_size = 32
    x = 0
    features = []
    key_map = {}
    T = 0.5
    t = T
    tmp_feature = {}
    sorted_list = []
    with open(rf, 'rb') as f:
        bin_trace = f.read(trace_byte_size)
        while bin_trace:
            x += 1
            p = packet_struct()
            p.init_from_binary(bin_trace)
            insert_p(p, tmp_feature, key_map, x)
            if p.time > t:
                # if p.time > t or x > 20000:
                t += T
                # 导出tmp_feature = {}
                # wname = wf+ str(t/5) + '.pkl'
                print('p.time:' + str(p.time))
                tmp_list = []
                for k, v in tmp_feature.items():
                    tmp_list.append([k, v])
                features.append(tmp_list)
                print(len(tmp_feature))
                tmp_feature.clear()

            if p.time > 600:
                break
            bin_trace = f.read(trace_byte_size)

    if len(tmp_feature) != 0 and len(features) < 600:
        print('p.time:' + str(p.time))
        tmp_list = []
        for k, v in tmp_feature.items():
            tmp_list.append([k, v])
        features.append(tmp_list)
        tmp_feature.clear()

    # save_map = [mapkey:key]
    save_map = {}

    for k, v in key_map.items():
        sorted_list.append(v)
        save_map[v[0]] = k
        # print(tmp_feature[v[0]])
        # print(v)
    print(len(sorted_list))
    sorted_list.sort(reverse=True, key=lambda x: x[1])

    w_list_name = pre + 'count.pkl'
    w_map_list_name = pre + 'mapkey_to_key.pkl'
    with open(w_map_list_name, 'wb') as f:
        pickle.dump(save_map, f)

    with open(w_list_name, 'wb') as f:
        pickle.dump(sorted_list, f)

    print('len(features):' + str(len(features)))
    # for i in sorted_list:
    #     print(i)
    big_dict = {}
    small_dict = {}
    bound = int(len(sorted_list) * big_percent)
    for k in sorted_list[:bound]:
        big_dict[k[0]] = k[1]
    print(sorted_list[bound - 100:bound])
    print(sorted_list[:100])

    for tmp_list in features:
        for kv in tmp_list:
            if kv[0] in big_dict.keys():
                kv.append(1)
            else:
                kv.append(0)

    for i in range(len(features)):
        f_list = features[i]
        wname = wf + str(i) + '.pkl'
        with open(wname, 'wb') as f:
            pickle.dump(f_list, f)


def pcap2dat(pcap_dir="../newtrace/pcap/", dat_dir="../newtrace/dat/"):
    for pcap in os.listdir(pcap_dir):
        print(pcap)
        if pcap[:5] == "univ2":
            shift = 0
        else:
            shift = 4
        print(shift)
        pcap_file = pcap_dir + pcap
        print(pcap_file)
        dat_file = dat_dir + pcap.split(".")[0] + ".dat"
        print(dat_file)
        pcap_parse(pcap_file, dat_file, shift)


def dat2feature(dat_dir="../newtrace/dat/", feature_dir="../newtrace/feature/"):
    for dat in os.listdir(dat_dir):
        print(dat)
        dat_file = dat_dir + dat

        feature_subdir = feature_dir + dat.split(".")[0] + "/"
        print(feature_subdir)

        big_percent = 0.1
        rf = dat_file
        pre = feature_subdir
        wf = feature_subdir + str(big_percent) + "/"

        if not os.path.isdir(pre):
            os.mkdir(pre)
        if not os.path.isdir(wf):
            os.mkdir(wf)

        read_dat(rf, pre, wf, big_percent)

    # if os.path.isdir(pkl_dir):
    #     print("是目录")
    # else:
    #     print("不是")
    #     os.mkdir(pkl_dir)


def read_pkl_to_instance(filename, big_dict):
    # print(filename)
    feature_list = []

    with open(filename, 'rb') as f:
        f_list = pickle.load(f)
        # print(f_list[:10])

        # 统计流的数量和总的流大小
        # tmp_dict = {}
        all_bytes = 0
        all_pkts = 0
        # f = [map_key, feature, lable]
        for f in f_list:
            # tmp_dict[f[0]] = 0
            all_bytes += f[1][0]
            all_pkts += f[1][1]
        # print(len(f_list))
        x = 0

        flow_num = len(f_list)

        for f in f_list:
            # print(f)
            # feature = f[1][:5]
            feature = f[1][:7]
            feature.append(f[1][6] - f[1][5])
            feature.append(f[1][8] - f[1][7])
            feature.append(f[1][9])
            feature.append(f[1][10])
            feature.append(f[1][11])

            feature.append(f[1][12])
            feature.append(f[1][13])

            feature[0] /= all_bytes
            feature[1] /= all_pkts
            feature[5] /= all_pkts
            feature[6] /= all_pkts
            feature[7] /= all_pkts
            # feature[0] = feature[0]/all_bytes*flow_num
            # feature[1] = feature[1]/all_pkts*flow_num
            # feature[5] = feature[5]/all_pkts
            # feature[6] = feature[6]/all_pkts
            # feature[7] = feature[7]/all_pkts

            # one_flow_feature = Instance(features=f[1], label=f[2])
            if f[0] in big_dict.keys():
                # print("big")
                one_flow_feature = Instance(features=feature, label=1, id=f[0])
            else:
                one_flow_feature = Instance(features=feature, label=0, id=f[0])

            # one_flow_feature = Instance(features=feature, label=f[2], id=f[0])
            feature_list.append(one_flow_feature)

            x += 1
            # if x<5:
            #     print(feature)
            #     print(feature)
    return feature_list


def generate_instance_pkl_by_feature_pkl(pkl_dir,count_file, instance_file, big_percent):
    files = os.listdir(pkl_dir)
    files.sort(key=lambda x: (len(x), x))
    save_pkl_file = []
    y = 0
    big_dict = {}

    with open(count_file, 'rb') as f:
        sorted_list = pickle.load(f)

        bound = int(len(sorted_list) * big_percent)
        for k in sorted_list[:bound]:
            big_dict[k[0]] = k[1]
    for file in files:
        if file[-3:] == "pkl":
            y += 1
            fname = pkl_dir + file
            print("load pkl:" + fname)
            instances = read_pkl_to_instance(filename=fname, big_dict=big_dict)
            save_pkl_file.append(instances)

    print(len(save_pkl_file))
    save_pkl(instance_file, save_pkl_file)

def feature2instance(feature_dir="../newtrace/feature/", instance_dir="../newtrace/instance/"):
    big_percent = 0.1
    for feature_subdir in os.listdir(feature_dir):
        pkl_dir = feature_dir + feature_subdir + "/" + str(big_percent) + "/"
        count_file = feature_dir + feature_subdir + "/count.pkl"
        instance_subdir = instance_dir + feature_subdir

        if not os.path.isdir(instance_subdir):
            os.mkdir(instance_subdir)

        instance_subsubdir = instance_subdir+ "/" + str(big_percent) + "/"
        if not os.path.isdir(instance_subsubdir):
            os.mkdir(instance_subsubdir)
        instance_file = instance_subsubdir + feature_subdir+".pkl"
        generate_instance_pkl_by_feature_pkl(pkl_dir,count_file,instance_file,big_percent)
        print(feature_subdir)
        print(pkl_dir)
        print(count_file)
        print(instance_file)


if __name__ == '__main__':
    # pcap2dat()
    dat2feature()
    feature2instance()

