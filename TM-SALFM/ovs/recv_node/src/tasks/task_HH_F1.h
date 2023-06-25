#ifndef _TASK_HH_F1_H_
#define _TASK_HH_F1_H_

#include <assert.h>
#include "task.h"

class TaskHHF1: public Task
{
    typedef unordered_map<string,int> Freq;
public:
    TaskHHF1();
    ~TaskHHF1();

    void run(Algorithm *algo, DataManager *dataManager, DataType DataType);
    void getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum);
    void getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum);
    double computeF1(unordered_map<string, int> &real_heavy_hitters, vector<pair<string, int>> &est_heavy_hitters);
};

#endif //_TASK_HH_F1_H_