#ifndef _TASK_HH_ARE_H_
#define _TASK_HH_ARE_H_

#include <assert.h>
#include <unordered_map>
#include "task.h"
using std::unordered_map;
using std::pair;

class TaskHHARE: public Task
{
    typedef unordered_map<string,int> Freq;
public:
    TaskHHARE();
    ~TaskHHARE();

    void run(Algorithm *algo, DataManager *dataManager, DataType dataType);
private:
    void getRealFreq(Freq &freq, uint8_t *data, int keylen, int itemNum);
    void getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum);
    double computeARE(unordered_map<string, int> &real_heavy_hitter, Freq &est_freq);
};

#endif //_TASK_FLOW_SIZE_H_