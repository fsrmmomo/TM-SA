#ifndef _HEAVYPART_H_
#define _HEAVYPART_H_

#include "param.h"
#include <unordered_map>
//#define USING_SIMD_ACCELERATION

template<int bucket_num>
class HeavyPart
{
public:
    alignas(64) Bucket buckets[bucket_num];
    unordered_map<string, int> extra_big;
    HeavyPart();
    ~HeavyPart();

    void clear();

    int insert(uint8_t *key, Val f);
//    int quick_insert(uint8_t *key, uint32_t f = 1);

    int query(uint8_t *key);

//    int get_memory_usage();
//    int get_bucket_num();
private:
    int CalculateFP(uint8_t *key, uint32_t &fp);
};

#endif //_HEAVYPART_H_
