#include "HeavyPart.h"
#include "param.h"

#ifdef USING_SIMD_ACCELERATION
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif // USING_SIMD_ACCELERATION

template<int bucket_num>
HeavyPart<bucket_num>::HeavyPart()
{
    this->clear();
}

template<int bucket_num>
HeavyPart<bucket_num>::~HeavyPart(){}

template<int bucket_num>
void HeavyPart<bucket_num>::clear()
{
    memset(buckets, 0, sizeof(Bucket) * bucket_num);
    extra_big.clear();
}

template<int bucket_num>
int HeavyPart<bucket_num>::insert(uint8_t *key, Val f)
{
    uint32_t fp;
    // 选择哈希位置
    int pos = CalculateFP(key, fp);

#ifdef USING_SIMD_ACCELERATION
    /* find if there has matched bucket */
	const __m256i item = _mm256_set1_epi32((int)fp);
	__m256i *keys_p = (__m256i *)(buckets[pos].key);
	int matched = 0;

	__m256i a_comp = _mm256_cmpeq_epi32(item, keys_p[0]);
	matched = _mm256_movemask_ps((__m256)a_comp);

	/* if matched */
	if (matched != 0){
		//return 32 if input is zero;
		int matched_index = _tzcnt_u32((uint32_t)matched);
		buckets[pos].val[matched_index] += f;
		return 0;
	}
	
	/* find the minimal bucket */
	const uint32_t mask_base = 0x7FFFFFFF;
	const __m256i *counters = (__m256i *)(buckets[pos].val);
	__m256 masks = (__m256)_mm256_set1_epi32(mask_base);
	__m256 results = (_mm256_and_ps(*(__m256*)counters, masks));
	__m256 mask2 = (__m256)_mm256_set_epi32(mask_base, 0, 0, 0, 0, 0, 0, 0);
	results = _mm256_or_ps(results, mask2);

	__m128i low_part = _mm_castps_si128(_mm256_extractf128_ps(results, 0));
	__m128i high_part = _mm_castps_si128(_mm256_extractf128_ps(results, 1));

	__m128i x = _mm_min_epi32(low_part, high_part);
	__m128i min1 = _mm_shuffle_epi32(x, _MM_SHUFFLE(0,0,3,2));
	__m128i min2 = _mm_min_epi32(x,min1);
	__m128i min3 = _mm_shuffle_epi32(min2, _MM_SHUFFLE(0,0,0,1));
	__m128i min4 = _mm_min_epi32(min2,min3);
	int min_counter_val = _mm_cvtsi128_si32(min4);

	const __m256i ct_item = _mm256_set1_epi32(min_counter_val);
	int ct_matched = 0;

	__m256i ct_a_comp = _mm256_cmpeq_epi32(ct_item, (__m256i)results);
	matched = _mm256_movemask_ps((__m256)ct_a_comp);
	int min_counter = _tzcnt_u32((uint32_t)matched);

	/* if there has empty bucket */
	if(min_counter_val == 0){		// empty counter
		buckets[pos].key[min_counter] = fp;
		buckets[pos].val[min_counter] = f;
		return 0;
	}
#else // USING_SIMD_ACCELERATION
    /* find if there has matched bucket */
    // 查找是否有匹配的桶
    int matched = -1, empty = -1, min_counter = 0;
//	uint32_t min_counter_val = GetCounterVal(buckets[pos].val[0].pkts);
    for(int i = 0; i < COUNTER_PER_BUCKET; i++){
        // flow ID 一致
        if(buckets[pos].key[i] == fp){
            matched = i;
            break;
        }
        // 空桶
        if(buckets[pos].key[i] == 0 && empty == -1) {
            empty = i;
            break;
        }
    }

    /* if matched */
    // 如果要插入的流与 Heavy Part 中记录的流ID一致
    // matched: 匹配桶的索引
    if(matched != -1){
        buckets[pos].val[matched].pkts += f.pkts; // 记录 vote+
        return 0;
    }
//    for(int i = 0; i < COUNTER_PER_BUCKET ; i++){
//
//    }

    if(empty != -1){
        buckets[pos].key[empty] = fp;  // 记录 flow ID
        buckets[pos].val[empty].pkts += f.pkts; // 记录 vote+
        return 0;
    }

#endif // USING_SIMD_ACCELERATION

    string str((const char *) key, 4);
#if NO_DROP==1
    extra_big[str]++;
    return 0;
#else
    return 1;
#endif

}

template<int bucket_num>
int HeavyPart<bucket_num>::query(uint8_t *key)
{
    struct Val res = {0};
    uint32_t fp;
    int pos = CalculateFP(key, fp);

#ifdef USING_SIMD_ACCELERATION
    do{
        /* find if there has matched bucket */
        const __m256i item = _mm256_set1_epi32((int)fp);
        __m256i *keys_p = (__m256i *)(buckets[pos].key);
        int matched = 0;

        __m256i a_comp = _mm256_cmpeq_epi32(item, keys_p[0]);
        matched = _mm256_movemask_ps((__m256)a_comp);

        /* if matched */
        if (matched != 0){
            //return 32 if input is zero;
            int matched_index = _tzcnt_u32((uint32_t)matched);
            return buckets[pos].val[matched_index];
        }
	}while(0);
#else
    for(int i = 0; i < COUNTER_PER_BUCKET; ++i)
        if(buckets[pos].key[i] == fp)
            return buckets[pos].val[i].pkts;
    string str((const char *) key, 4);

    unordered_map<string, int>::iterator iter;
    iter = extra_big.find(str);
    if(extra_big.find(str)!=extra_big.end()) {
#if NO_DROP==1
        return iter->second;
#else
        return -1;
#endif
    }
#endif
    return -1;
}

//template<int bucket_num>
//int HeavyPart<bucket_num>::get_memory_usage()
//{
//    return bucket_num * sizeof(Bucket);
//}
//
//template<int bucket_num>
//int HeavyPart<bucket_num>::get_bucket_num()
//{
//    return bucket_num;
//}

template<int bucket_num>
int HeavyPart<bucket_num>::CalculateFP(uint8_t *key, uint32_t &fp)
{
    fp = *((uint32_t*)key);
    return CalculateBucketPos(fp) % bucket_num;
}