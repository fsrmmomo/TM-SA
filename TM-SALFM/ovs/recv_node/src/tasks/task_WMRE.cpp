#include "task_WMRE.h"
#include <unordered_set>
using std::unordered_set;
using std::pair;
using std::sort;

TaskWMRE::TaskWMRE(): Task("WMRE"){}
TaskWMRE::~TaskWMRE(){}


void TaskWMRE::run(Algorithm *algo, DataManager *dataManager, DataType dataType)
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

    /* get performance */
    double WMRE = this->computeWMRE(real_freq, est_freq);
    // printf("(Task:%s, Algo:%s): itemNum=%d, keylen=%d, WMRE=%.6lf\n",
    //     this->name.c_str(), algo->name.c_str(), itemNum, keylen, WMRE);
    printf("%.6lf\t", WMRE);
}

void TaskWMRE::getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum)
{
    freq.clear();
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){
        string str((const char*)(&data[offset]), keylen);  
        freq[str]++; 
    }
}

void TaskWMRE::getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum)
{
    unordered_set<string> keys;
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){  
        string str((const char*)(&data[offset]), keylen); 
        keys.insert(str); 
    }
    flowIDs.clear();
    for(auto it : keys) 
        flowIDs.push_back(it); 
    // printf("TaskWMRE::getKeys have %lu keys\n", flowIDs.size());
}

double TaskWMRE::computeWMRE(Freq &real_freq, Freq &est_freq)
{
    int real_fm [500000];
    int est_fm [500000];
    for (int i = 1; i < 500000; i++) {
        real_fm[i] = 0;
        est_fm[i] = 0;
    }
    for (auto it : real_freq)
    {
        int est_val = 0;
        if (est_freq.count(it.first))
            est_val = est_freq[it.first];
        real_fm[it.second]++;
        est_fm[est_val]++;   
    }

    double numerator = 0;
    double denominator = 0; 
    for (int i = 1; i < 500000; i++) {
        numerator += abs(est_fm[i] - real_fm[i]);
        denominator += abs(est_fm[i] + real_fm[i]);
    }
    return numerator * 1.0 / (denominator / 2);
}
