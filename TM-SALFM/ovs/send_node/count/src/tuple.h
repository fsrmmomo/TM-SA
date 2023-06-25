#ifndef __TUPLE_H__
#define __TUPLE_H__

typedef struct __attribute__ ((__packed__)) FlowKey {
	// 8 (4*2) bytes
    uint32_t src_ip;  // source IP address
    uint32_t dst_ip;
	// 4 (2*2) bytes
    uint16_t src_port;
    uint16_t dst_port;
    // 1 bytes
    uint8_t proto;
} flow_key_t;

#define TUPLE_NORMAL 0
#define TUPLE_PUNC   1
#define TUPLE_TERM   2
#define TUPLE_START  3

typedef struct Tuple {

    /**************************************
     * keys
     *************************************/
    flow_key_t key;
    // 1 bytes
    // only used in multi-thread environment
    uint8_t flag;



    /**************************************
     * values 
     *************************************/

    uint8_t tos;
	// 8 bytes
	uint64_t pkt_ts;				// timestamp of the packet

    // 8 bytes
    uint64_t seq;
    // 8 bytes
	int32_t size;			// inner IP datagram length (header + data)
//    uint32_t diff_time;
//    double loss;
//    double loss_sum;
} tuple_t;


typedef struct statistic{
    int total_byte;     //0
    int total_pkts;     //1
    int max_bytes;      //2
    int min_bytes;      //3
    int aver_bytes;     //4
    int start_win;      //5
    int end_win;        //6
    uint64_t start_time;     //7
    uint64_t end_time;       //8
    int max_twin;       //9
    int min_twin;       //10
    int aver_twin;      //11
    int src_port;       //12
    int dst_port;       //13
} statistic_t;

typedef struct feature{
    double total_byte;     //0  0
    double total_pkts;     //1  1
    double start_win;      //5  2
    double end_win;        //6  3
    double win_size;       //7  4
    int max_bytes;      //2     5
    int min_bytes;      //3     6
    int aver_bytes;     //4     7
    int time_win_size;  //8     8
    int max_twin;       //9     9
    int min_twin;       //10    10
    int aver_twin;      //11    11
    int dst_port;       //12    12
    int src_port;       //13    13
    uint32_t key;       //14    14
} feature_t;

#endif
