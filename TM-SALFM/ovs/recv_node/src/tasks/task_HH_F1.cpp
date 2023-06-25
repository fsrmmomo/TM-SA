#include "task_HH_F1.h"
#include <iostream>
#include <unordered_set>
#include <unordered_set>
using std::unordered_set;
using std::pair;
using std::sort;

TaskHHF1::TaskHHF1(): Task("HH_F1"){}

TaskHHF1::~TaskHHF1(){}

void TaskHHF1::run(Algorithm *algo, DataManager *dataManager, DataType dataType)
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
    
    unordered_map<string, int> real_heavy_hitters;
    vector<pair<string, int>> est_heavy_hitters;
    for (auto it : real_freq) {
        if (it.second >= threshold) {
            real_heavy_hitters[it.first] = it.second;
        }
    }
    algo->get_heavy_hitters(threshold, est_heavy_hitters);

    /* get performance */
    double F1 = this->computeF1(real_heavy_hitters, est_heavy_hitters);
    printf("%.6lf\t", F1);
}

void TaskHHF1::getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum)
{
    freq.clear();
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){
        string str((const char*)(&data[offset]), keylen);  
        freq[str]++; 
    }
}

void TaskHHF1::getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum)
{
    unordered_set<string> keys; 
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){  
        string str((const char*)(&data[offset]), keylen); 
        keys.insert(str); 
    }
    flowIDs.clear();
    for(auto it : keys) 
        flowIDs.push_back(it); 
    // printf("TaskHHF1::getKeys have %lu keys\n", flowIDs.size());
}


double TaskHHF1::computeF1(unordered_map<string, int> &real_heavy_hitters, vector<pair<string, int>> &est_heavy_hitters)
{
    double TP = 0.0;
    for (auto it : est_heavy_hitters) {
        if (real_heavy_hitters.count(it.first)) {
            TP++;
        }
    }
    double precesion = TP / (double) est_heavy_hitters.size();
    double recall = TP / (double) real_heavy_hitters.size();
    double F1_Score = (2 * precesion * recall) / (precesion + recall);
    return F1_Score;
}
