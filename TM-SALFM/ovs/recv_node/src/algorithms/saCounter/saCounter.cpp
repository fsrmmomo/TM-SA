#include "saCounter.h"
#include <string.h>

/* counters[width*d]
 * | [0,0] |  ...  | [0,w-1] | [1,0] | ... | [1,w-1] | ...  |
 * |       |       |         |       |     |         |      |
*/


saCounter::saCounter(int tot_mem_in_bytes, int d): Algorithm("saCounter")
{
    this->d = d;
    this->width = tot_mem_in_bytes / d / sizeof(uint8_t);
    counters = new uint8_t[width * d];
    recoverCounters = new uint32_t[width * d];
    memset(counters, 0, sizeof(uint8_t) * width * d);  
    memset(recoverCounters, 0, sizeof(uint32_t) * width * d);
    hash = new BOBHash32[d]; 
    for(int i = 0; i < d; ++i)
        hash[i].initialize(100 + i); 

    elephant_flow.clear();
}

saCounter::~saCounter()
{
    delete[] hash;
    delete[] counters;
}

void saCounter::clear()
{
    insertTimes = 0;
    memset(counters, 0, sizeof(uint8_t) * width * d);
    memset(recoverCounters, 0, sizeof(uint32_t) * width * d);
}


int saCounter::insert(uint8_t *key, int keylen)
{
    for(int i = 0; i < d; ++i) {  
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);

        int s = counters[pos] >> 5;
        int c = counters[pos] & 0x1f;
        int r = pow(2, s);
        int randomNum = rand() % r; 
        if (randomNum == 0) {
            if (c < 0x1f) {
                counters[pos]++;
            } else {
                counters[pos] = (s + 1) << 5;
            }
        }
    }
    return ++insertTimes;  
}

int saCounter::query(uint8_t *key, int keylen)
{
    string str((const char*)key, keylen);
    if (elephant_flow.find(str) != elephant_flow.end()) 
        return elephant_flow[str];

    int ret = 0x7FFFFFFF;
    for(int i = 0; i < d; ++i)
    {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        ret = ret > recoverCounters[pos] ? recoverCounters[pos] : ret;
    }
    return ret;
}

void saCounter::recover() {
    for (int i = 0; i < d; i++) {
        for (int j = 0; j < width; j++) {
            int pos = i * width + j;
            int s = counters[pos] >> 5;
            int c = counters[pos] & 0x1f;
            int r = pow(2, s);
            int stageS = 0;
            if (s != 0) {
                for (int k = 0; k < s; k++) {
                    stageS += pow(2, k);
                }
                stageS *= pow(2, 5);
            }
            recoverCounters[pos] = c * pow(2, s) + stageS;
        }
    }
}

void saCounter::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq)
{
    freq.clear();
    uint8_t key[100] = {0};
    for(auto id : flowIDs){
        memcpy(key, id.c_str(), id.size());
        freq[id] = this->query(key, id.size());
    }
}

void saCounter::insert_elephant(uint8_t* key, int size, int keylen)
{
    string str((const char*)key, keylen);
    elephant_flow[str] = size;
    for (int i = 0; i < d; ++i) {
        int pos = i * width + (hash[i].run((const char*)key, keylen) % width);
        recoverCounters[pos] = recoverCounters[pos] > size ? recoverCounters[pos] - size : 0;
    }
}

int saCounter::get_cardinality()
{
    int ns = 0x7FFFFFFF;
    for(int i = 0; i < d; ++i)
    {
        int w0 = 0;
        for (int j = 0; j < width; j++) {
            if (recoverCounters[i * width + j] == 0) {
                w0++;
            }
        }
        int temp_ns = (int)(width * (log(width) - log(w0)));
        ns = (ns <= temp_ns) ? ns : temp_ns;
    }
    return elephant_flow.size() + ns;
}

void saCounter::get_heavy_hitters(int threshold, vector<pair<string, int>> & results)
{
    for (auto it : elephant_flow) {
        if (it.second >= threshold) {
            results.push_back(make_pair(it.first, it.second));
        }
    }
}