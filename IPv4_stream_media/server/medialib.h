#ifndef MEDIALIB_H_
#define MEDIALIB_H_
#include <unistd.h>
#include "../include/proto.h"
//记录每一条节目单信息：频道号chnid，描述信息char* desc
struct mlib_listentry_st
{
    uint8_t chnid;
    char *desc;//description
};

int mlib_getchnlist(struct mlib_listentry_st **, int *);

int mlib_freechnlist(struct mlib_listentry_st *);

ssize_t mlib_readchn(uint8_t, void *, size_t);


#endif
