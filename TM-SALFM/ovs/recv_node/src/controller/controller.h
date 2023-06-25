#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <map>
#include <vector>

#include "../data/dataManager.h"

#include "../tasks/task.h"
#include "../tasks/task_flowSize.h"
#include "../tasks/task_throughput.h"
#include "../tasks/task_CE.h"
#include "../tasks/task_WMRE.h"
#include "../tasks/task_HH_ARE.h"
#include "../tasks/task_HH_F1.h"

#include "../algorithms/algorithms.h"

#include "../algorithms/elasticSketch/ElasticSketch.h"
#include "../algorithms/elasticSketch/ElasticSketch.cpp"

#include "../algorithms/cmSketch/cmsketch.h"

#include "../algorithms/mySketch/mySketch.h"
#include "../algorithms/mySketch/mySketch.cpp"

#include "../algorithms/saCounter/saCounter.h"

// #define HEAVY_MEM (150 * 1024)
// #define BUCKET_NUM (HEAVY_MEM / 64)
// #define TOT_MEM_IN_BYTES (600 * 1024)
// typedef MYSketch<BUCKET_NUM, TOT_MEM_IN_BYTES> mysketch;
class Controller
{
public:
    Controller(){}
    ~Controller(){}

    // 执行任务
    void performTask(char *dataFile, char*bigFile, DataType dataType, int maxItemNum, Algorithm* algo, TaskType taskType, bool needInsert=true)
    {
        assert(dataFile && algo);
        // algo->clear();
        DataManager dataManager;
        dataManager.loadData(dataFile, maxItemNum);
        dataManager.loadBigData(bigFile);
 
        if(needInsert)
            this->insert(algo, &dataManager, dataType);

        Task *task = this->createTask(taskType);
        // printf("********* algo:%s ********* task:%s *********\n", algo->name.c_str(), task->name.c_str());

        task->run(algo, &dataManager, dataType);
        delete task;

        // printf("\n");
    }
    
    // void myTask(char* dataFile, DataType dataType, int maxItemNum, mysketch* algo, TaskType taskType, bool needInsert = true)
    // {
    //     assert(dataFile && algo);
    //     // algo->clear();
    //     DataManager dataManager;
    //     dataManager.loadData(dataFile, maxItemNum);

    //     // 插入数据
    //     if (needInsert)
    //         this->mysert(algo, &dataManager, dataType);

    //     Task* task = this->createTask(taskType);
    //     printf("********* algo:%s ********* task:%s *********\n", algo->name.c_str(), task->name.c_str());

    //     task->run(algo, &dataManager, dataType);
    //     delete task;

    //     printf("\n");
    // }

private:
    Task* createTask(TaskType type)
    {
        Task* task = nullptr;
        switch(type)
        {
            case TASK_TYPE_FLOW_SIZE:
                task = (Task*)new TaskFlowSize();
                break;
            case TASK_TYPE_THROUGHPUT:
                task = (Task*)new TaskThroughput();
                break;
            case TASK_TYPE_CE:
                task = (Task*)new TaskCE();
                break;
            case TASK_TYPE_WMRE:
                task = (Task*)new TaskWMRE();
                break;
            case TASK_TYPE_HH_ARE:
                task = (Task*)new TaskHHARE();
                break;
            case TASK_TYPE_HH_F1:
                task = (Task*)new TaskHHF1();
                break;
            default:
                printf("Controller::createTask wrong type of task\n");
        }
        return task;
    }

    // 插入数据
    void insert(Algorithm *algo, DataManager *dataManager, DataType dataType)
    {   
        // assert宏的原型定义在<assert.h>中，其作用是如果它的条件返回错误，则终止程序执行。
        assert(algo && dataManager); // 判断algo, dataManager实例是否存在
        
        /* get data */
        // 这里的data只有数据包包头，没有数据包内容
        int itemNum = dataManager->getDataLen(); // 获取data数组长度，即数据包个数
        int keylen = dataManager->getDataSize(dataType); // 获取指定类型的数据包头部大小
        uint8_t *data = (uint8_t*)malloc(itemNum * keylen); // 为data分配内存空间
        dataManager->getData(data, dataType, itemNum); // 获得所有的key, 存储到itemNum中

        int lossRate = 0;
        for(int i = 0, offset = 0; i < itemNum; ++i, offset += keylen) {
            // 网络中存在丢包的情况
            // if (rand() % 100 >= lossRate)
            //     algo->insert(&data[offset], keylen);  // input: 数据起始指针位置，key长度

            algo->insert(&data[offset], keylen);  // input: 数据起始指针位置，key长度
        }
        free(data);
        // printf("Controller:insert insertion ends\n");

        /**************************
            后置处理
        ***************************/ 
        // if (strcmp(algo->name.c_str(), "saCounter") == 0) {
        //     saCounter *sa = (saCounter*)algo;
        //     sa->recover();
        // }
    }

    bool cmp(pair<uint32_t, int>a, pair<uint32_t, int>b) {
        return a.second - b.second;
    }

    // 自定义插入数据
    // void mysert(mysketch* algo, DataManager* dataManager, DataType dataType)
    // {
    //     assert(algo && dataManager); // 判断algo, dataManager实例是否存在

    //     /* get data */
    //     int itemNum = dataManager->getDataLen();
    //     int keylen = dataManager->getDataSize(dataType);
    //     uint8_t* data = (uint8_t*)malloc(itemNum * keylen); // 存储所有数据
    //     dataManager->getData(data, dataType, itemNum); 
        
    //     // 插入数据
    //     map<uint32_t, int> all_flows;

    //     for (int i = 0, offset = 0; i < itemNum; ++i, offset += keylen) {
    //         algo->insert(&data[offset], keylen);
    //         // uint32_t mergeKey = 0;
    //         // for (int j = 0; j < 4; j++) {
    //         //     mergeKey = (mergeKey << 8) | data[offset + j * keylen];
    //         // }
    //         // printf("mergeKey: %d\n", mergeKey);

    //         uint32_t mergeKey = *((uint32_t*)&data[offset]);
    //         // printf("key: %d.%d.%d.%d\n", data[offset], data[offset + 1], data[offset + 2], data[offset + 3]);
    //         all_flows[mergeKey]++;
    //     }

    //     // 找到大象流
    //     vector<pair<uint32_t, int>> vecs;
    //     for (auto it = all_flows.begin(); it != all_flows.end(); it++) {
    //         vecs.push_back(pair<uint32_t, int>(it->first, it->second));
    //     }
    //     vector<pair<uint32_t, int>>::const_iterator first = vecs.begin();
    //     vector<pair<uint32_t, int>>::const_iterator second = vecs.begin() + vecs.size() / 10;
    //     vector<pair<uint32_t, int>> elephants;
    //     elephants.assign(first, second);
        
    //     // 插入大象流
    //     for (auto it = elephants.begin(); it != elephants.end(); it++) {
    //         algo->insert_elephant((uint8_t*)(&it->first), it->second, 32);
    //     }

    //     free(data);
    //     //printf("Controller:insert insertion ends\n");
    // }
};

#endif //_CONTROLLER_H_