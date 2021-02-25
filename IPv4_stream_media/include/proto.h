#ifndef PROTO_H__
#define PROTO_H__

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define DEFAULT_MGROUP      "224.2.2.2" //多播组地址
#define DEFAULT_RECVPORT  "2333" //服务端口

#define CHANNEL_NUM         100//频道个数
#define MINCHANNELID        1//最小频道号 0 号负责发送节目单
#define MAXCHANNELID        (MINCHANNELID+CHANNEL_NUM-1)//最大频道号

#define MSG_CHANNEL_MAX     (65536-20-8)
#define MSG_DATA            (MSG_CHANNEL_MAX-sizeof(uint8_t))

#define MSG_LIST_MAX        (65536-20-8)
#define MSG_ENTRY           (MSG_LIST_MAX-sizeof(uint8_t))

//频道包的数据结构
struct msg_channel_st{
    uint8_t channelid; //[MINCHANNELID,MAXCHANNELID)
    uint8_t data[0];
}__attribute__((packed));

//每个频道的数据结构
struct msg_listentry_st{//entry 条目
    uint8_t channelid;
    uint8_t desc[0];//每个频道的描述文件
}__attribute__((packed));

//节目单的数据结构 0号频道专用 定时发送节目单
struct msg_list_st{
    uint8_t channelid;
    struct msg_listentry_st entry[0];
}__attribute__((packed));


#endif
