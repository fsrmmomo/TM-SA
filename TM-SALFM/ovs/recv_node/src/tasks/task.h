#ifndef _TASK_H_
#define _TASK_H_

#include "../algorithms/algorithms.h"
#include "../data/dataManager.h"
#include <string>
using std::string;

class Task
{
public:
    string name;
    Task(string name): name(name) {}
    virtual ~Task() = 0;
    virtual void run(Algorithm *algo, DataManager *dataManager, DataType dataType) = 0;
};

typedef enum{
    TASK_TYPE_FLOW_SIZE = 0,
    TASK_TYPE_THROUGHPUT,
    TASK_TYPE_CE,
    TASK_TYPE_WMRE,
    TASK_TYPE_HH_ARE,
    TASK_TYPE_HH_F1
}TaskType;

#endif //_TASK_H_