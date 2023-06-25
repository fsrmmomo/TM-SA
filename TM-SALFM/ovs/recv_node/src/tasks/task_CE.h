#ifndef _TASK_CE_H_
#define _TASK_CE_H_

#include <assert.h>
#include "task.h"

class TaskCE: public Task
{
public:
    TaskCE();
    ~TaskCE();

    void run(Algorithm *algo, DataManager *dataManager, DataType DataType);
    int getKeys(vector<string> &flowIDs, uint8_t *data, int keylen, int itemNum);
};

#endif //_TASK_CE_H_