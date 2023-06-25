//
// Created by momo on 2022/5/30.
//

#include "DivSketch.h"

template<int bucket_num, int tot_memory_in_bytes>
DivSketch<bucket_num, tot_memory_in_bytes>::~DivSketch(){}

template<int bucket_num, int tot_memory_in_bytes>
void DivSketch<bucket_num, tot_memory_in_bytes>::clear()
{
    insertTimes = 0;
    heavy_part.clear();
    light_part.clear();
}

template<int bucket_num, int tot_memory_in_bytes>
int DivSketch<bucket_num, tot_memory_in_bytes>::insert(uint8_t *key, struct Val f)
{
    uint8_t swap_key[KEY_LENGTH_4];
    uint32_t swap_val = 0;
    // 0: match | empty  1: 驱逐小流  2: 在light part中记录小流
//    int result = heavy_part.insert(key, swap_key, swap_val, f);
    int type = f.type;
    if (type==0){
        light_part.insert(key,f.pkts);
    }else if(type == 1){
//        printf("type==1 !\n");
        if(heavy_part.insert(key,f)==1){
            light_part.insert(key,f.pkts);
        }
    } else{
        printf("error type !\n");
        exit(1);
    }
//    switch (type) {
//        case 0:
//            light_part.insert(key,f.pkts);break;
//        case 1:
//            if(heavy_part.insert(key,f))break;
//
//        default:
//            printf("error type !\n");
//            exit(1);
//    }

    return ++insertTimes;
}

//template<int bucket_num, int tot_memory_in_bytes>
//void DivSketch<bucket_num, tot_memory_in_bytes>::quick_insert(uint8_t *key, int f)
//{
//    heavy_part.quick_insert(key, f);
//}

template<int bucket_num, int tot_memory_in_bytes>
int DivSketch<bucket_num, tot_memory_in_bytes>::query(uint8_t *key)
{
    int heavy_result = heavy_part.query(key);
    if(heavy_result==-1) {
        heavy_result =  light_part.query(key);
    } else{
        heavy_result += light_part.query(key);
    }
    return heavy_result;
}
template<int bucket_num, int tot_memory_in_bytes>
void DivSketch<bucket_num, tot_memory_in_bytes>::get_heavy_hitters(int threshold, vector<pair<string, int>> & results)
{
    for (int i = 0; i < bucket_num; ++i) {
        for (int j = 0; j < COUNTER_PER_BUCKET; ++j) {
            uint32_t key = heavy_part.buckets[i].key[j];
            int val = query((uint8_t *) &key);
            if (val >= threshold) {
                results.push_back(make_pair(string((const char *) &key, 4), val));
            }
        }
    }
//    for (unordered_map<string, int>::iterator it = heavy_part.extra_big.begin();
//        it != heavy_part.extra_big.end(); ++it){
//        uint8_t key[4];
//        memcpy(key, (it->first).c_str(), 4);
//        int val = query((uint8_t *) &key);
//        if (val >= threshold) {
//            results.push_back(make_pair(string((const char *) &key, 4), val));
//        }
//
//    }

//    for (unordered_map<string, int>::iterator it = heavy_part.extra_big.begin(); it != heavy_part.extra_big.end(); ++it) {
//        uint8_t key[4];
//        memcpy(key, (it->first).c_str(), 4);
//        int val = query(key);
//        if (it->second > threshold) {
//            if (val >= threshold) {
//                results.push_back(make_pair(string((const char *)key, 4), val));
//            }
//        }
//    }



}

//template<int bucket_num, int tot_memory_in_bytes>
//struct Val DivSketch<bucket_num, tot_memory_in_bytes>::query(uint8_t *key, int keylen)
//{
//    struct Val heavy_result = heavy_part.query(key);
//    if(heavy_result.pkts == 0 || HIGHEST_BIT_IS_1(heavy_result.pkts))
//    {
//        int light_result_counters;
//        int light_result_sizes;
//        light_part.query(key, &light_result_counters, &light_result_sizes);
//        // return (int)GetCounterVal(heavy_result) + light_result;
//        heavy_result.pkts += light_result_counters;
//        heavy_result.sizes += light_result_sizes;
//    }
//    return heavy_result;
//}

//template<int bucket_num, int tot_memory_in_bytes>
//int DivSketch<bucket_num, tot_memory_in_bytes>::query_compressed_part(uint8_t *key, uint8_t *compress_part, int compress_counter_num)
//{
//    uint32_t heavy_result = heavy_part.query(key);
//    if(heavy_result == 0 || HIGHEST_BIT_IS_1(heavy_result))
//    {
//        int light_result = light_part.query_compressed_part(key, compress_part, compress_counter_num);
//        return (int)GetCounterVal(heavy_result) + light_result;
//    }
//    return heavy_result;
//}
//
//template<int bucket_num, int tot_memory_in_bytes>
//int DivSketch<bucket_num, tot_memory_in_bytes>::get_compress_width(int ratio)
//{
//    return light_part.get_compress_width(ratio);
//}
//
//template<int bucket_num, int tot_memory_in_bytes>
//void DivSketch<bucket_num, tot_memory_in_bytes>::compress(int ratio, uint8_t *dst)
//{
//    light_part.compress(ratio, dst);
//}
//
//template<int bucket_num, int tot_memory_in_bytes>
//int DivSketch<bucket_num, tot_memory_in_bytes>::get_bucket_num(){
//    return heavy_part.get_bucket_num();
//}
//
//template<int bucket_num, int tot_memory_in_bytes>
//double DivSketch<bucket_num, tot_memory_in_bytes>::get_bandwidth(int compress_ratio)
//{
//    int result = heavy_part.get_memory_usage();
//    result += get_compress_width(compress_ratio) * sizeof(uint8_t);
//    return result * 1.0 / 1024 / 1024;
//}
//


template<int bucket_num, int tot_memory_in_bytes>
int DivSketch<bucket_num, tot_memory_in_bytes>::get_cardinality()
{
    int card = light_part.get_cardinality();
    return card;
}
//
//template<int bucket_num, int tot_memory_in_bytes>
//double DivSketch<bucket_num, tot_memory_in_bytes>::get_entropy()
//{
//    int tot = 0;
//    double entr = 0;
//
//    light_part.get_entropy(tot, entr);
//
//    for(int i = 0; i < bucket_num; ++i)
//        for(int j = 0; j < MAX_VALID_COUNTER; ++j)
//        {
//            uint8_t key[KEY_LENGTH_4];
//            *(uint32_t*)key = heavy_part.buckets[i].key[j];
//            int val = heavy_part.buckets[i].val[j];
//
//            int ex_val = light_part.query(key);
//
//            if(HIGHEST_BIT_IS_1(val) && ex_val)
//            {
//                val += ex_val;
//
//                tot -= ex_val;
//
//                entr -= ex_val * log2(ex_val);
//            }
//            val = GetCounterVal(val);
//            if(val)
//            {
//                tot += val;
//                entr += val * log2(val);
//            }
//        }
//    return -entr / tot + log2(tot);
//}
//
//template<int bucket_num, int tot_memory_in_bytes>
//void DivSketch<bucket_num, tot_memory_in_bytes>::get_distribution(vector<double> &dist)
//{
//    light_part.get_distribution(dist);
//
//    for(int i = 0; i < bucket_num; ++i)
//        for(int j = 0; j < MAX_VALID_COUNTER; ++j)
//        {
//            uint8_t key[KEY_LENGTH_4];
//            *(uint32_t*)key = heavy_part.buckets[i].key[j];
//            int val = heavy_part.buckets[i].val[j];
//
//            int ex_val = light_part.query(key);
//
//            if(HIGHEST_BIT_IS_1(val) && ex_val != 0)
//            {
//                val += ex_val;
//                dist[ex_val]--;
//            }
//            val = GetCounterVal(val);
//            if(val)
//            {
//                if(val + 1 > dist.size())
//                    dist.resize(val + 1);
//                dist[val]++;
//            }
//        }
//}

template<int bucket_num, int tot_memory_in_bytes>
void* DivSketch<bucket_num, tot_memory_in_bytes>::operator new(size_t sz)
{
    constexpr uint32_t alignment = 64;
    size_t alloc_size = (2 * alignment + sz) / alignment * alignment;
    void *ptr = ::operator new(alloc_size);
    void *old_ptr = ptr;
    void *new_ptr = ((char*)std::align(alignment, sz, ptr, alloc_size) + alignment);
    ((void **)new_ptr)[-1] = old_ptr;

    return new_ptr;
}

template<int bucket_num, int tot_memory_in_bytes>
void DivSketch<bucket_num, tot_memory_in_bytes>::operator delete(void *p)
{
    ::operator delete(((void**)p)[-1]);
}


template<int bucket_num, int tot_memory_in_bytes>
void DivSketch<bucket_num, tot_memory_in_bytes>::get_flowsize(vector<string> &flowIDs, unordered_map<string,int> &freq)
{
    freq.clear();
    for(auto flowid : flowIDs)
        freq[flowid] = this->query((uint8_t*)flowid.c_str());
}
