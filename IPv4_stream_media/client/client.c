#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/proto.h"
#include "client.h"

/*
 *  -M -- mgroup 指定多播组
 *  -P --port    指定接收端口
 *  -p --player  指定播放器
 *  -H --help    help
 *
 * */

struct client_conf_st client_conf = {
    .rcvport = DEFAULT_RECVPORT,
    .mgroup  = DEFAULT_MGROUP,
    .player  = DEFAULT_PLAYER
};

static void help(){
    printf("-M -- mgroup 指定多播组\n");
    printf("-P --port    指定接收端口\n");
    printf("-p --player  指定播放器\n");
    printf("-H --help    help\n");
}

int main(int argc,char **argv){
    
    //初始化级别
    //1 命令行传参
    //2 环境变量
    //3 配置文件
    //4 默认值
    struct option argarr[] = {
        {"port",1,NULL,'p'},
        {"mgroup",1,NULL,'M'},
        {"player",1,NULL,'p'},
        {"help",0,NULL,'H'},
        {NULL,0,NULL,0}
    };
    int index = 0;
    char op;
    ;
    while(1){
        op = getopt_long(argc,argv,"P:M:p:H:",argarr,&index);
        if (op < 0){
            break;
        }
        
        switch (op){
            case 'P':
                client_conf.rcvport = optarg;
                break;
            case 'M':
                client_conf.mgroup = optarg;
                break;
            case 'p':
                client_conf.player = optarg;
                break;
            case 'H':
                help();
                exit(0);
                break;
            default:
                abort();
                break;
        }
    }

    int sfd;//socket文件描述符
    sfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sfd < 0){
        perror("socket()");
        exit(1);
    }

    struct sockaddr_in laddr;//local addr
    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(client_conf.rcvport));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    //设置socket的属性
    struct ip_mreqn mreqn;
    inet_pton(AF_INET,"0.0.0.0",&mreqn.imr_address);
    inet_pton(AF_INET,client_conf.mgroup,&mreqn.imr_multiaddr);
    mreqn.imr_ifindex = if_nametoindex("wlp7s0");

    if (setsockopt(sfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreqn,sizeof(mreqn)) < 0){
        perror("setsockopt()");
        exit(1);
    }//打开广播属性

    struct sockaddr_in laddr;//local addr
    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(SERVERPORT));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数

    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }
    pipe();

    fork();

    if (pid < 0){
        perror("fork()");
        exit(1);
    }
    if (pid == 0){
        //
    }

    exit(0);
}

