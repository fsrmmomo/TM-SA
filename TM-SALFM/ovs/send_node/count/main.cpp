#include <iostream>
#include <unordered_map>
#include "src/tuple.h"
//#include <WS2tcpip.h>
//#include <winsock2.h>
//#include <ctime>
#include <time.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cstring>
#include <unistd.h>
#include <chrono>
using namespace std;
using namespace std::chrono;
//#pragma comment(lib,"WS2_32.lib")
unordered_map<uint32_t , statistic_t> count_map;
void creat(){
    statistic_t st = {0};
    st.total_byte += 1;
    st.total_pkts = 1;
    st.max_bytes = 1;
    st.min_bytes =2;
    st.aver_bytes =3;
    st.start_win = 2;
    st.end_win = 2;
    uint32_t key = 1;
    count_map[key] = st;
}
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
int main() {
    std::cout << "Hello, World!" << std::endl;
    creat();
    uint32_t key = 1;
    printf("%d\n", count_map[key].total_byte);

/*
    WORD wVersion;
    WSADATA wsaData;
    int err;

    wVersion = MAKEWORD(1, 1);
    err = WSAStartup(wVersion, &wsaData);
    if (err != 0) {
        return err;
    }

    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        WSACleanup();
        return -1;
    }
*/
    struct sockaddr_in Server;
//    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0); //配置模式，
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //配置模式，
    //设置服务器地址addrSrv和监听端口
    Server.sin_family = AF_INET;
    Server.sin_addr.s_addr = inet_addr("192.168.1.21"); //设置服务器主机ip地址（与接收方客户端的IP对应）
    Server.sin_port = htons(8001);

//    bind(serverSocket, (sockaddr *)&Server, sizeof(sockaddr));

    //*************************************************************************************************************//
    struct sockaddr_in Client;				//创建客户端sockaddr_in结构体
//    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0); //配置模式，
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); //配置模式，
    Client.sin_family = AF_INET;
    Client.sin_addr.s_addr = inet_addr("127.0.0.1");
    Client.sin_port = htons(8002);
    if(connect(clientSocket, (sockaddr *)&Server, sizeof(sockaddr))<0){
        //printf("连接失败:%d", WSAGetLastError());
 	perror("connect error");
 	exit(1);
        return 0;
    }
    char send_info_buf[200];
    feature_t f1= {0};
    f1.total_byte = 888;
    f1.total_pkts = 2222222;
    f1.max_bytes = 333333;
    f1.win_size = 0;
    f1.aver_bytes = 666666;
    f1.dst_port =2222;
    f1.src_port = 444;
    f1.max_twin = 0;
    f1.key = 3003456789;
    f1.min_twin = 999999;
    int i=0;
    high_resolution_clock::time_point t1, t2;
    clock_t start, finish;
    double  duration;
    start = clock();
t1 = high_resolution_clock::now();

    //发送长度
    int f_num = 10000;
    int l = f_num*80 + 4;
    memcpy(send_info_buf, &l, sizeof(l));
    send(clientSocket, send_info_buf, 4, 0);
    memcpy(send_info_buf, &f1, sizeof(feature_t));
    while (i<f_num){
        i++;
        send(clientSocket, send_info_buf, sizeof(feature_t), 0);
//        int res = sendto(clientSocket, send_info_buf, sizeof(feature_t), 0, (sockaddr *)&Server, sizeof(sockaddr));
//        cout << "sendto_len:  "<<res << endl<< endl;//若发送失败。则返回-1

    }
    sprintf(send_info_buf,"exit");
    send(clientSocket, send_info_buf, 4, 0);

    char len[4];
    recv(clientSocket, len, 4, 0);
    uint32_t length = *(uint32_t *)len;
    printf("len = %d\n",length);
    char *buf = (char*)malloc(length+4);
//    length
    int ret = readn(clientSocket, buf, length);
    if(ret != length)
    {
        close(clientSocket);
        free(buf);
        return -1;
    }
    uint32_t * keys = (uint32_t*)buf;
    int int_len = length/sizeof(int);
//    for (int j = 0; j < int_len/10; ++j) {
//        key = *keys;
//        keys++;
//        printf("%u\n",*keys);
//        keys++;
//    }
free(buf);



    //closesocket(clientSocket);
close(clientSocket);
    finish   = clock();
    chrono::milliseconds ms;
    t2 = high_resolution_clock::now();
    ms = duration_cast<chrono::milliseconds>(t2 - t1);
    printf("%d milliseconds\n", ms.count());
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf("%f seconds\n", duration);
    return 0;
}

