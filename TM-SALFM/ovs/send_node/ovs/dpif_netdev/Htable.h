//
// Created by momo on 2022/6/8.
//

#ifndef C_SERVER_HTABLE_H
#define C_SERVER_HTABLE_H

#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include "BoBHash32.h"

#define HTABLE_W 5000
#define HTABEL_D 5
#define KEY_LENGTH 4
typedef struct Bucket {
    uint32_t key[HTABEL_D];
}Bucket_t;
typedef struct Htable{
    uint32_t w ;
//    int d ;
    Bucket_t* buckets;
    BoBHash32 bobhash;
    void (*Constructor)(struct Htable *,uint32_t);
    void (*Destructor)(struct Htable *);   //析构函数
    void (*Insert)(struct Htable *,uint32_t);
    void (*Clear)(struct Htable *);
    int (*Check)(struct Htable *,uint32_t);

} Htable;

typedef struct parameter{
    int fd;
    Htable * htable;
} parameter;



void _constructor_Htabel(struct Htable * htable,uint32_t w);

void _destructor_Htable(struct Htable * htable);

void _insert_Htable(struct Htable * htable,uint32_t key);
int _check_Htable(struct Htable * htable,uint32_t key);
void _clear_Htable(struct Htable * htable);
void Init_Htable(Htable *htable, int w);
uint32_t get_a_key(struct Htable * htable);


#endif //C_SERVER_HTABLE_H
