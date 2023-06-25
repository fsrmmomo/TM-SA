#include "task_HH_ARE.h"
#include <unordered_set>
using std::unordered_set;
using std::pair;
using std::sort;

TaskHHARE::TaskHHARE(): Task("HH_ARE"){}
TaskHHARE::~TaskHHARE(){}

void TaskHHARE::run(Algorithm *algo, DataManager *dataManager, DataType dataType)
{
    assert(algo && dataManager);
    /* get data */
    int itemNum = dataManager->getDataLen();
    int keylen = dataManager->getDataSize(dataType);
    uint8_t *data = (uint8_t*)malloc(itemNum * keylen);
    dataManager->getData(data, dataType, itemNum);

    /* get real flow size */
    Freq real_freq;
    this->getRealFreq(real_freq, data, keylen, itemNum);
    vector<string> flowIDs;
    this->getKeys(flowIDs, data, keylen, itemNum);
    free(data);

    /* get estimated flow size */
    Freq est_freq;
    algo->get_flowsize(flowIDs, est_freq);

    int packet_cnt = 0;
    vector<pair<string, int>> vecs;
    for (auto it : real_freq) {
        vecs.push_back(pair<string, int>(it.first, it.second));
        packet_cnt += it.second;
    }
    int threshold = packet_cnt / 5000;
    
    // 获取 真实的 heavy_hitters
    unordered_map<string, int> real_heavy_hitters;
    for (auto it : real_freq) {
        if (it.second >= threshold) {
            real_heavy_hitters[it.first] = it.second;
        }
    }

    /* get performance */
    double ARE = this->computeARE(real_heavy_hitters, est_freq);
    printf("%.6lf\t", ARE);
}

void TaskHHARE::getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum)
{
    freq.clear();
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){
        string str((const char*)(&data[offset]), keylen);  
        freq[str]++; 
    }
}

void TaskHHARE::getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum)
{
    unordered_set<string> keys; 
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){  
        string str((const char*)(&data[offset]), keylen); 
        keys.insert(str); 
    }
    flowIDs.clear();
    for(auto it : keys) 
        flowIDs.push_back(it); 
    // printf("TaskHHARE::getKeys have %lu keys\n", flowIDs.size());
}

double TaskHHARE::computeARE(unordered_map<string, int> &real_heavy_hitter, Freq &est_freq)
{
    double ARE = 0;
    for (auto it : real_heavy_hitter) {
        int est_val = 0;
        if (est_freq.count(it.first))
            est_val = est_freq[it.first];

        ARE = abs(it.second - est_val) * 1.0 / it.second;
    }
    return ARE;
}
