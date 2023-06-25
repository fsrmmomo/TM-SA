// 该文件定义了如何处理和获取数据

#include "dataManager.h"
#include <algorithm>
#include <string>
#include <arpa/inet.h>
using std::pair;

// 加载数据
void DataManager::loadData(const char *dataFile, int maxItemNum)
{
    data.clear();
    FILE *fin = fopen(dataFile, "rb");
    if(fin == nullptr){
        printf("DataManager::loadData failed to open file %s\n", dataFile);
        exit(EXIT_FAILURE);
    }

    FIVE_TUPLE tmp_five_tuple;
    uint8_t buffer[15];
    int itemCnt = 0;
    while(fread(buffer, 1, 15, fin) == 15 && itemCnt++ < maxItemNum){
        memcpy(&tmp_five_tuple, buffer, sizeof(FIVE_TUPLE));
        data.push_back(tmp_five_tuple);
    }
    
    fclose(fin);
    // printf("DataManager::loadData successfully read in %s, %ld packets\n", dataFile, data.size());
}

// 获取数据
int DataManager::getData(void *outData, DataType type, int maxItemNum)
{
    int itemNum = std::min(maxItemNum, (int)data.size());
    switch(type)
    {
        case DATA_TYPE_FIVE_TUPLE:
            return this->get_fiveTuples((FIVE_TUPLE*)outData, itemNum); // 返回五元组类型的key集合
        case DATA_TYPE_SRC_IP:
            return this->get_srcIPs((int*)outData, itemNum); // 返回源IP类型的key集合
        case DATA_TYPE_SRC_AND_DST_IP:
            return this->get_srcAndDstIPs((uint64_t*)outData, itemNum); // 返回源和目的IP类型的key集合
        default:
            printf("DataManager::getData wrong type of data\n");
    }
    return EXIT_FAILURE;
}

// 获取数据大小
int DataManager::getDataSize(DataType type)
{
    switch (type)
    {
        case DATA_TYPE_FIVE_TUPLE:
            return sizeof(FIVE_TUPLE);
        case DATA_TYPE_SRC_IP:
            return sizeof(int);
        case DATA_TYPE_SRC_AND_DST_IP:
            return sizeof(uint64_t);
        default:
            printf("DataManager::getDataSize wrong type of data\n");
    }
    return EXIT_FAILURE;
}

// 获取data数组长度
int DataManager::getDataLen()
{
    return data.size();
}

// 获取5元组
int DataManager::get_fiveTuples(FIVE_TUPLE *tuples, int maxItemNum)
{
    int itemNum = std::min(maxItemNum, (int)data.size());
    for(int i = 0; i < itemNum; ++i)
        memcpy(&tuples[i], &data[i], sizeof(FIVE_TUPLE));
    return itemNum;
}

int DataManager::get_srcIPs(int *srcIPs, int maxItemNum)
{
    int itemNum = std::min(maxItemNum, (int)data.size());
    for(int i = 0; i < itemNum; ++i)
        memcpy(&srcIPs[i], &data[i].srcIP, sizeof(int));
    return itemNum;
}

int DataManager::get_srcAndDstIPs(uint64_t *IPs, int maxItemNum)
{
    int itemNum = std::min(maxItemNum, (int)data.size());
    for(int i = 0; i < itemNum; ++i)
        memcpy(&IPs[i], (uint64_t*)(&data[i]), sizeof(uint64_t));
    return itemNum;
}

// 加载数据
void DataManager::loadBigData(const char *dataFile)
{
    big_data.clear();
    FILE *fin = fopen(dataFile, "rb");
    if(fin == nullptr){
        printf("DataManager::loadBigData failed to open file %s\n", dataFile);
        exit(EXIT_FAILURE);
    }

    BIG_FLOW tmp_big_flow;
    uint8_t buffer[8];
    while(fread(buffer, 1, 8, fin) == 8){
        memcpy(&tmp_big_flow, buffer, sizeof(BIG_FLOW));
        string str((const char*)(&tmp_big_flow.srcIP), 4);
        big_data.push_back(pair<string, int>(str, tmp_big_flow.cnt));
    }
    
    fclose(fin);
    // printf("DataManager::loadData successfully read in %s, %ld packets\n", dataFile, data.size());
}

// 获取大流
void DataManager::getBigFlow(vector<pair<string, int>> *elephants)
{
    for (auto it : big_data) {
        elephants->push_back(pair<string, int>(it.first, it.second));
    }   
}