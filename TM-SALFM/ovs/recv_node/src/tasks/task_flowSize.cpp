#include "task_flowSize.h"
#include <unordered_set>
#include "../algorithms/saCounter/saCounter.h"
using std::unordered_set;
using std::pair;
using std::sort;

TaskFlowSize::TaskFlowSize(): Task("FlowSize"){}
TaskFlowSize::~TaskFlowSize(){}

bool cmp(pair<string, int>a, pair<string, int>b) {
    return a.second > b.second;
}

void TaskFlowSize::run(Algorithm *algo, DataManager *dataManager, DataType dataType)
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

    // vector<pair<string, int>> vecs;
    // for (auto it : real_freq) {
    //     vecs.push_back(pair<string, int>(it.first, it.second));
    // }
    // sort(vecs.begin(), vecs.end(), cmp);
    // vector<pair<string, int>>::const_iterator first = vecs.begin();
    // vector<pair<string, int>>::const_iterator second = vecs.begin() + vecs.size() * 10 / 100;
    // vector<pair<string, int>> elephants;
    // elephants.assign(first, second);

    int lossRate = 100;
   
    if (strcmp(algo->name.c_str(), "saCounter") == 0) {
        vector<pair<string, int>> elephants;
        dataManager->getBigFlow(&elephants);

        saCounter *sa = (saCounter*)algo;
        sa->recover();
        for (auto it : elephants) {
            // if (rand() % 100 + 1 > lossRate)
            sa->insert_elephant((uint8_t*)(&it.first), it.second, keylen);
        }
    }

    /* get estimated flow size */
    Freq est_freq;
    algo->get_flowsize(flowIDs, est_freq);

    /* get performance */
    double ARE = this->computeARE(real_freq, est_freq);
    // printf("(Task:%s, Algo:%s): itemNum=%d, keylen=%d, ARE=%.6lf\n",
    //     this->name.c_str(), algo->name.c_str(), itemNum, keylen, ARE);
    printf("%.6lf\t", ARE);
}

void TaskFlowSize::getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum)
{
    freq.clear();
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){
        string str((const char*)(&data[offset]), keylen); 
        freq[str]++; 
    }
}

void TaskFlowSize::getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum)
{
    unordered_set<string> keys; 
    for(int i = 0, offset = 0; i < itemNum; i++, offset += keylen){ 
        string str((const char*)(&data[offset]), keylen); 
        keys.insert(str);
    }
    flowIDs.clear();
    for(auto it : keys) 
        flowIDs.push_back(it); 
    // printf("TaskFlowSize::getKeys have %lu keys\n", flowIDs.size());
}

/* 计算ARE
 * input: 真实值集合，估计值集合
 * output: ARE     
*/
double TaskFlowSize::computeARE(Freq &real_freq, Freq &est_freq)
{
    int packet_cnt = 0;
    vector<pair<string, int>> vecs;
    for (auto it : real_freq) {
        vecs.push_back(pair<string, int>(it.first, it.second));
        packet_cnt += it.second;
    }
    int threshold = packet_cnt / 5000;
    double big_are = 0;
    int big_cnt = 0;
    double small_are = 0;
    int small_cnt = 0;


    double ARE = 0;
    for(auto it : real_freq)
    {
        int est_val = 0;
        if(est_freq.count(it.first))
            est_val = est_freq[it.first];

        ARE += std::abs(it.second - est_val) * 1.0 / it.second; 


        if (it.second >= threshold){
            big_are += std::abs(it.second - est_val) * 1.0 / it.second;
            big_cnt++;
        }
        else {
            small_are += std::abs(it.second - est_val) * 1.0 / it.second;
            small_cnt++;
        }
    }
    printf("%.6lf\t", big_are / big_cnt);
    printf("%.6lf\t", small_are / small_cnt);


    ARE /= real_freq.size(); 
    return ARE;
}
