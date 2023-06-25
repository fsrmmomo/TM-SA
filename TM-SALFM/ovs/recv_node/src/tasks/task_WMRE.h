#ifndef _TASK_WMRE_H_
#define _TASK_WMRE_H_

#include <assert.h>
#include "task.h"
#include <unordered_map>
using std::unordered_map;


class TaskWMRE: public Task
{
    typedef unordered_map<string,int> Freq;
public:
    TaskWMRE();
    ~TaskWMRE();

    void run(Algorithm *algo, DataManager *dataManager, DataType DataType);
    void getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum);
    void getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum);
    double computeWMRE(Freq &real_freq, Freq &est_freq);
};

#endif //_TASK_WMRE_H_