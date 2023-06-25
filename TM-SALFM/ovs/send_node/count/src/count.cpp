//
// Created by momo on 2022/6/1.
//

//
// Created by momo on 2022/5/30.
//
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>       /* open */
#include <unistd.h>       /* exit */
#include <sys/ioctl.h> /* ioctl */
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <sys/mman.h>
//#include <hiredis/hiredis.h> // 操作数据库
#include <errno.h>
#include <time.h> // 时间戳
#include <thread> // 操作线程
#include <future> // 线程
#include "util.h"
#include "tuple.h"
//#include "DivSketch.h"
//#include "DivSketch.cpp"
#include "ringbuffer.h"
#include "param.h"
#include <sys/time.h>
#include <unordered_map>
//#include <winsock.h>
//#include "../elasticSketch/ElasticSketch.h"
//#include "../elasticSketch/ElasticSketch.cpp"
//#include "../elasticSketch/param.h"
//#include <winsock2.h>
#include <cstring>
#include <chrono>
#include <arpa/inet.h>
using namespace std;
using namespace std::chrono;

// The number of ringbuffer
// **Must** be (# pmd threads + 1)
#define MAX_RINGBUFFER_NUM 2

#pragma comment(lib,"ws2_32.lib")
// divSketch
//#define LEVEL 4
//#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / COUNTER_PER_BUCKET/8)
#define TOT_MEM_IN_BYTES (1 * 1024 * 1024)
//typedef ElasticSketch<BUCKET_NUM,TOT_MEM_IN_BYTES> Elastic;
//typedef DivSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> Div;
bool flag = true;
//DivSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> *div1 = new DivSketch<BUCKET_NUM, TOT_MEM_IN_BYTES>();
unordered_map<uint32_t, statistic_t> count_map;
int allPkts = 0;
int allBytes = 0;



// 用于操作数据库的指针
//redisContext *pContext;

static inline char *ip2a(uint32_t ip, char *addr) {
    sprintf(addr, "%.3d.%.3d.%.3d.%.3d", (ip & 0xff), ((ip >> 8) & 0xff), ((ip >> 16) & 0xff), ((ip >> 24) & 0xff));
    return addr;
}

void print_tuple(FILE *f, tuple_t *t) {
    char ip1[30], ip2[30];

    fprintf(f, "%s(%u) <-> %s(%u) %u %d\n",
            ip2a(t->key.src_ip, ip1), ntohs(t->key.src_port),
            ip2a(t->key.dst_ip, ip2), ntohs(t->key.dst_port),
            t->key.proto, ntohs(t->size));
}

void insert_pkt(tuple_t *t) {
    allPkts ++;
    allBytes += t->size;
    uint32_t key = t->key.src_ip;
    unordered_map<uint32_t, statistic_t>::iterator iter;
    iter = count_map.find(key);
    if (count_map.find(key) != count_map.end()) {
        statistic_t st = iter->second;
        st.total_byte += t->size;
        st.total_pkts += 1;
        st.max_bytes = st.max_bytes > t->size ? st.max_bytes : t->size;
        st.min_bytes = st.min_bytes > t->size ? t->size : st.min_bytes;
        st.aver_bytes = (st.aver_bytes * (st.total_pkts - 1) + t->size) / st.total_pkts;
        st.end_win = allPkts;
//        st.start_time = t->pkt_ts;
        int twin = t->pkt_ts - st.end_time;
        st.end_time = t->pkt_ts;
        st.max_twin = st.max_twin > twin ? st.max_twin : twin;
        st.min_twin = st.min_twin > twin ? twin : st.min_twin;
//        st.src_port = t->key.src_port;
//        st.dst_port = t->key.dst_port;
    } else {
        statistic_t st = {0};
        st.total_byte += t->size;
        st.total_pkts = 1;
        st.max_bytes = t->size;
        st.min_bytes = t->size;
        st.aver_bytes = t->size;
        st.start_win = allPkts;
        st.end_win = allPkts;
        st.start_time = t->pkt_ts;
        st.end_time = t->pkt_ts;
        st.min_twin = 5000000;
        st.src_port = t->key.src_port;
        st.dst_port = t->key.dst_port;
        count_map[key] = st;
    }
}

feature_t statistic_t_2_feature_t(statistic_t st,uint32_t key){

    feature_t ft = {0};
    ft.total_byte = st.total_byte;
    ft.total_pkts = st.total_pkts;
    ft.max_bytes = st.max_bytes;
    ft.min_bytes = st.min_bytes;
    ft.aver_bytes = st.aver_bytes;
    ft.start_win = st.start_win;
    ft.end_win = st.end_win;
    ft.win_size = st.end_win - st.start_win;
    ft.time_win_size = st.end_time - st.start_time;
    ft.max_twin = st.max_twin;
    ft.min_twin = st.min_twin;
    ft.aver_twin = ft.aver_twin;
    ft.src_port = st.src_port;
    ft.dst_port = st.dst_port;

    ft.total_byte /= allBytes;
    ft.total_pkts /= allPkts;
    ft.start_win /= allPkts;
    ft.end_win /= allPkts;
    ft.win_size /= allPkts;
    ft.key = key;
    return ft;
}
int connect_to_server(){

    //设置服务端属性
    struct sockaddr_in Server;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //配置模式，
    //设置服务器地址addrSrv和监听端口
    Server.sin_family = AF_INET;
    Server.sin_addr.s_addr = inet_addr("192.168.1.21"); //设置服务器主机ip地址（与接收方客户端的IP对应）
    Server.sin_port = htons(8001);

    //设置本地属性
    //*************************************************************************************************************//
    struct sockaddr_in Client;				//创建客户端sockaddr_in结构体
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); //配置模式，
    Client.sin_family = AF_INET;
    Client.sin_addr.s_addr = inet_addr("127.0.0.1");
    Client.sin_port = htons(8002);
    if(connect(clientSocket, (sockaddr *)&Server, sizeof(sockaddr))<0){
        printf("连接失败");
        return -1;
    }
    return clientSocket;
}
/*
函数描述: 接收指定的字节数
函数参数:
    - fd: 通信的文件描述符(套接字)
    - buf: 存储待接收数据的内存的起始地址
    - size: 指定要接收的字节数
函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
*/
int readn(int fd, char* buf, int size)
{
    char* pt = buf;
    int count = size;
    while (count > 0)
    {
        int len = recv(fd, pt, count, 0);
        if (len == -1)
        {
            return -1;
        }
        else if (len == 0)
        {
            return size - count;
        }
        pt += len;
        count -= len;
    }
    return size;
}
void sendAll(int clientSocket){

    char send_info_buf[200];
    int f_num = count_map.size();
    int l = f_num*80 ;
    memcpy(send_info_buf, &l, sizeof(l));
    send(clientSocket, send_info_buf, 4, 0);

    for (unordered_map<uint32_t, statistic_t>::iterator it = count_map.begin(); it != count_map.end(); ++it) {
        feature_t ft = statistic_t_2_feature_t(it->second,it->first);
        memcpy(send_info_buf, &ft, sizeof(feature_t));
        send(clientSocket, send_info_buf, sizeof(feature_t), 0);
    }

    count_map.clear();
}
long long int counter = 0;
bool KEEP_RUNNING;

void handler(int sig) {
    if (sig == SIGINT) {
        KEEP_RUNNING = false;
        return;
    }

    printf("total insert: %lld\n", counter);
    counter = 0;
    alarm(1);
}

int main(int argc, char *argv[]) {
    // 连接数据库
//    pContext = connnectDB();

    tuple_t t;
    LOG_MSG("Initialize the ringbuffer.\n");

    /* link to shared memory (datapath) */
    ringbuffer_t *rbs[MAX_RINGBUFFER_NUM];
    for (int i = 0; i < MAX_RINGBUFFER_NUM; ++i) {
        char name[30] = {0};
        sprintf(name, "/rb_%d", i);
        printf("name=%s\n", name);
        rbs[i] = connect_ringbuffer_shm(name, sizeof(tuple_t));
        // printf("%p\n", rbs[i]);
    }
    printf("connected.\n");
    fflush(stdout);

    /* print number of pkts received per seconds */
    signal(SIGALRM, handler);
    signal(SIGINT, handler);
    alarm(1);



    high_resolution_clock::time_point t1, t2;
    t1 = high_resolution_clock::now();

    /* create your measurement structure (sketch) here !!! */

    /* begin polling */
    int con = connect_to_server();
    if(con == -1){
        printf("连接识别服务器失败");
        return 0;
    }
    int idx = 0;
//    int tt = 0;
    KEEP_RUNNING = true;
    while (KEEP_RUNNING) {
        if (t.flag == TUPLE_TERM) {
            break;
        } else {
            while (read_ringbuffer(rbs[(idx) % MAX_RINGBUFFER_NUM], &t) < 0 && KEEP_RUNNING) {
                idx = (idx + 1) % MAX_RINGBUFFER_NUM;
            }
            counter++;
            print_tuple(stdout, &t);
            //  Insert to sketch here

//            struct Val f = {1, static_cast<uint8_t>(t.tos & 0x01)};
            // printf("%d %d %ld %lf\n", f.pkts, f.sizes, f.delay, f.loss);
            insert_pkt(&t);
            if(duration_cast<chrono::milliseconds>(high_resolution_clock::now() - t1).count()>500){
                sendAll(con);
		        t1 = high_resolution_clock::now();

            }



        }

    }
    return 0;
}




