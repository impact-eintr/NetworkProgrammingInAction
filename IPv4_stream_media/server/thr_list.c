#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "../include/proto.h"
#include "thr_list.h"
#include "server_conf.h"
#include "medialib.h"

static pthread_t tid_list;

//节目单包含的节目数量
static int nr_list_ent;

//节目单信息数组，每一条存储一个节目频道信息
static struct mlib_listentry_st *list_ent;

//线程处理函数
static void* thr_list(void* p)
{
    int totalsize;
    struct msg_list_st *entlistp;//0号专用
    struct msg_listentry_st *entryp;//每个节目单的结构
    int ret;
    int size;
    totalsize = sizeof(uint8_t);
    for(int i = 0; i < nr_list_ent;i++)
    {
        totalsize += sizeof(struct msg_listentry_st) + strlen(list_ent[i].desc);
    }

    entlistp = malloc(totalsize);
    if(entlistp == NULL)
    {
        perror("malloc()");
        //syslog(LOG_ERR, "malloc():%s", strerror(errno));
        exit(1);
    }
    entlistp->channelid = LISTCHANNELID;
    entryp = entlistp->entry;
    syslog(LOG_DEBUG, "nr_list_entn:%d\n", nr_list_ent);
    for(int i = 0; i < nr_list_ent; i++)
    {
        size = sizeof(struct msg_listentry_st)+strlen(list_ent[i].desc);
        entryp->channelid = list_ent[i].chnid;
        entryp->len = htons(size);
        strncpy(entryp->desc, list_ent[i].desc,strlen(list_ent[i].desc));
        fprintf(stdout, "entryp len:%d\n", entryp->len);
        entryp = (void*)(((char*)entryp) + size);
        //syslog(LOG_DEBUG, "entryp len:%d\n", entryp->len);
    }

    while(1)
    {
        //syslog(LOG_INFO, "thr_list sndaddr :%d\n", sndaddr.sin_addr.s_addr);
        fprintf(stdout, "\nthr_list sndaddr :%d\n", sndaddr.sin_addr.s_addr);
        ret = sendto(serversd, entlistp, totalsize, 0, (void*)&sndaddr, sizeof(sndaddr));
        //syslog(LOG_DEBUG, "sent content len:%d\n", entlistp->entry->len);
        fprintf(stderr, "sent content len:%d\n", entlistp->entry->len);
        if(ret < 0)
        {
            syslog(LOG_WARNING, "sendto(serversd, enlistp...:%s", strerror(errno));
        }
        else
        {
            syslog(LOG_DEBUG, "sendto(serversd, enlistp....):success");

        }
        sleep(1);
        
    }
}
//创建节目单线程
int thr_list_create(struct mlib_listentry_st *listp, int nr_ent)
{
    int err;
    list_ent = listp;
    nr_list_ent = nr_ent;
    syslog(LOG_DEBUG, "list content: chnid:%d, desc:%s\n", listp->chnid, listp->desc);
    err = pthread_create(&tid_list, NULL, thr_list, NULL);
    if(err)
    {
        syslog(LOG_ERR, "pthread_create():%s", strerror(errno));
        return -1;
    }
    return 0;
}

int thr_list_destroy(void)
{
    pthread_cancel(tid_list);
    pthread_join(tid_list, NULL);
    return 0;
}
