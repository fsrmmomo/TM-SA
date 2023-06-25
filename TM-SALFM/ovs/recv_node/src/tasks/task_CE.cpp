#include <iostream>
#include "task_CE.h"
#include <unordered_set>
using std::unordered_set;

TaskCE::TaskCE(): Task("CE") {}
TaskCE::~TaskCE() {}

void TaskCE::run(Algorithm *algo, DataManager *dataManager, DataType dataType)
{
     assert(algo && dataManager);
    /* get data */
    int itemNum = dataManager->getDataLen();
    int keylen = dataManager->getDataSize(dataType);
    uint8_t *data = (uint8_t*)malloc(itemNum * keylen);
    dataManager->getData(data, dataType, itemNum);

    /* get real flow size */
    // 获取真实流大小
    vector<string> flowIDs;
    int real_n = this->getKeys(flowIDs, data, keylen, itemNum);
    free(data);

    double RE = abs(algo->get_cardinality() - real_n) * 1.0 / real_n;
    // printf("real_n= %d, est = %d, RE = %.6lf\n", real_n, algo->get_flowquantity(), RE);
    printf("%.6lf\t", RE);
}

// 获取所有的key
int TaskCE::getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum)
{
    unordered_set<string> keys; // keys是一个set,这样就确保了插入的数据是无重复的
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){  // 遍历所有数据
        string str((const char*)(&data[offset]), keylen); // 获取当前数据包的头部信息作为str(key)
        keys.insert(str); // 插入key
    }
    flowIDs.clear();
    for(auto it : keys) // 把keys(1个set)中数据放到flowIDs(1个array)中
        flowIDs.push_back(it); // 把 it 放入flowIDs尾部
    // printf("TaskFlowSize::getKeys have %lu keys\n", flowIDs.size());
    return flowIDs.size();
}