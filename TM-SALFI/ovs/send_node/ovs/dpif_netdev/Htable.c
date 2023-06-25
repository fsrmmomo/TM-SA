//
// Created by momo on 2022/6/9.
//
#include "Htable.h"
void _constructor_Htabel(struct Htable * htable,uint32_t w){
    Init_BobHash32(&(htable->bobhash),739);
    htable->buckets =  (Bucket_t*) malloc(w * sizeof(Bucket_t));
    htable->w = w;
    memset(htable->buckets, 0, sizeof(Bucket_t) * htable->w);

};
void _destructor_Htable(struct Htable * htable){
    free(htable->buckets);
    htable->buckets = NULL;
    htable->w = 0;
    htable->bobhash.Destructor(&(htable->bobhash));
}

void _insert_Htable(struct Htable * htable,uint32_t key){
    uint32_t hash_val = (uint32_t)htable->bobhash.Run(&(htable->bobhash), (const char*)(&key), KEY_LENGTH);
    uint32_t pos = hash_val % (uint32_t)htable->w;
    Bucket_t* bucket = &(htable->buckets[pos]);
    for (int i = 0; i < HTABEL_D; ++i){
        if(bucket->key[i] == key){
            return;
        }
        if(bucket->key[i] == 0){
            bucket->key[i] = key;
            return;
        }
    }
}
int _check_Htable(struct Htable * htable,uint32_t key){
    uint32_t hash_val = (uint32_t)htable->bobhash.Run(&(htable->bobhash), (const char*)(&key), KEY_LENGTH);
    uint32_t pos = hash_val % (uint32_t)htable->w;
    Bucket_t bucket = htable->buckets[pos];
    for (int i = 0; i < HTABEL_D; ++i){
        if(bucket.key[i] == key){
            return 1;
        }
        if(bucket.key[i] == 0){
            return 0;
        }
    }
    return 0;
}
void _clear_Htable(struct Htable * htable){
    memset(htable->buckets, 0, sizeof(Bucket_t) * htable->w);
}

void Init_Htable(Htable *htable, int w){
    htable->Constructor = _constructor_Htabel;
    htable->Destructor = _destructor_Htable;
    htable->Insert = _insert_Htable;
    htable->Clear = _clear_Htable;
    htable->Check = _check_Htable;
    htable->Constructor(htable,w);
}
uint32_t get_a_key(struct Htable * htable){
    for (int i = 0; i < htable->w; ++i){
        Bucket_t bucket = htable->buckets[i];
        for (int j = 0; j < HTABEL_D; ++j){
            if(bucket.key[j] != 0){
                return bucket.key[j];
            } else{
                break;
            }
        }
    }
    return 0;
}
