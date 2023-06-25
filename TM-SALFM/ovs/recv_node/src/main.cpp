#include <stdio.h>
#include "controller/controller.h"

#define HEAVY_MEM (150 * 1024)
#define BUCKET_NUM (HEAVY_MEM / 64)
#define TOT_MEM_IN_BYTES (600 * 1024)
typedef ElasticSketch<BUCKET_NUM,TOT_MEM_IN_BYTES> Elastic;
// typedef MYSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> mysketch

int main()
{
char dataFile[] = "notime/slice/0-access1.dat";
char bigFile[] = "notime/big_flow/0-access1.dat";
    Controller controller;
    int maxItemNum = 3000000;

    Elastic *elastic = new Elastic();
    // int tot_mem_in_bytes = 3000 * 1024;
    CMSketch *cm = new CMSketch(TOT_MEM_IN_BYTES, 3);
    saCounter *sa = new saCounter(TOT_MEM_IN_BYTES, 3);
    // typedef MYSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> mysketch;
    // mysketch* my = new mysketch(3);
    // controller.myTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (mysketch*)my, TASK_TYPE_FLOW_SIZE);
    //controller.myTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (MYSketch*)my, TASK_TYPE_THROUGHPUT, false);
    // controller.myTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (MYSketch*)my, TASK_TYPE_FLOW_SIZE, false);
    // delete my;


    // ARE
    controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_FLOW_SIZE);
    controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)cm, TASK_TYPE_FLOW_SIZE);
    controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_FLOW_SIZE);
    printf("\t");
    
    
    // // Throughput
    // // controller.performTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_THROUGHPUT, false);
    // // controller.performTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)cm, TASK_TYPE_THROUGHPUT, false);
    // // controller.performTask(dataFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_THROUGHPUT, false);
    
    
    // // CE
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_CE, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)cm, TASK_TYPE_CE, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_CE, false);
    // printf("\t");

    
    // // WMRE
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_WMRE, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)cm, TASK_TYPE_WMRE, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_WMRE, false);
    // printf("\t");


    // // HHD_ARE
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_HH_ARE, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_HH_ARE, false);
    // printf("\t");
    

    // // HHD_F1
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)elastic, TASK_TYPE_HH_F1, false);
    // controller.performTask(dataFile, bigFile, DATA_TYPE_SRC_IP, maxItemNum, (Algorithm*)sa, TASK_TYPE_HH_F1, false);
    // printf("\t");


    delete elastic;
    delete cm;
    delete sa;
}