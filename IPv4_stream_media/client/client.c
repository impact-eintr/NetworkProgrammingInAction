#include <stdint.h>
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
#include <errno.h>

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

static int writen(int fd,const char *buf,size_t len){
    int ret;
    while(len > 0){
        ret = write(fd,buf,len);
        if (ret < 0){
            if (errno == EINTR)
              continue;
        }

    }
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

    //设置socket的属性
    struct ip_mreqn mreqn;
    inet_pton(AF_INET,"0.0.0.0",&mreqn.imr_address);
    inet_pton(AF_INET,client_conf.mgroup,&mreqn.imr_multiaddr);
    mreqn.imr_ifindex = if_nametoindex("wlp7s0");


    if (setsockopt(sfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreqn,sizeof(mreqn)) < 0){
        perror("setsockopt()");
        exit(1);
    }//打开广播属性
    

    int val = 1;
    if (setsockopt(sfd,IPPROTO_IP,IP_MULTICAST_LOOP,&val,sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    struct sockaddr_in laddr;//local addr
    laddr.sin_family = AF_INET;//指定协议
    laddr.sin_port = htons(atoi(client_conf.rcvport));//指定网络通信端口
    inet_pton(AF_INET,"0.0.0.0",&laddr.sin_addr);//IPv4点分式转二进制数


    if(bind(sfd,(void *)&laddr,sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    //建立父子间通信管道
    int pipefd[2];//管道结构
    if (pipe(pipefd) < 0){
        perror("pipe()");
        exit(1);
    }

    pid_t pid;
    pid = fork();
    if (pid < 0){
        perror("fork");
        exit(1);
    }

    //子进程:调用解码器
    if (pid == 0){
        close(sfd);
        close(pipefd[1]);//关闭写端
        dup2(pipefd[0],0);//将管道的读端重定向到标准输入
        if (pipefd[0] > 0){
            close(pipefd[0]);
        }

        execl("/bin/sh","sh","-c",client_conf.player,NULL);
        perror("execl()");
        exit(1);
    }else{
        //父进程:从网络上收包发送给子进程
        //收节目单
        struct msg_list_st *msg_list;
        msg_list = malloc(MSG_LIST_MAX);
        if (msg_list == NULL){
            perror("malloc()");
            exit(1);
        }

        struct sockaddr_in serveraddr;//服务器地址
        socklen_t serveraddr_len = sizeof(serveraddr);
        int pkglen;//收到的包长度

        while(1){
            pkglen = recvfrom(sfd,msg_list,pkglen,0,(void *)&serveraddr,&serveraddr_len);//报式套接字每次通信都需要知道对方是谁
            if (pkglen < sizeof(struct msg_list_st)){
                fprintf(stderr,"message is too small.\n");
                continue;
            }

            if (msg_list->channelid != LISTCHANNELID){
                fprintf(stderr,"channelid is not no.0!\n");
                continue;
            }
            break;
        }

        //打印节目单选择频道
        struct msg_listentry_st *pos;

        for (pos = msg_list->entry;\
                    (char *)pos < (((char *)msg_list)+pkglen);\
                    pos = (void *)(((char *)pos)+ntohs(pos->len))){
            printf("频道 %d : %s \n",pos->channelid,pos->desc);
        }

        int chosenid;
        while(1){
            if(scanf("%d",&chosenid) != 1){
                exit(1);
            }
        }

        //收频道包 发送给子进程
        struct msg_channel_st *msg_channel;
        msg_channel = malloc(MSG_CHANNEL_MAX);
        if (msg_channel == NULL){
            perror("malloc()");
            exit(1);
        }

        //接收来自服务器的频道音乐数据
        struct sockaddr_in raddr;//服务器地址
        socklen_t raddr_len = sizeof(serveraddr);
        int rpkglen;//收到的包长度

        while(1){
            rpkglen = recvfrom(sfd,msg_channel,rpkglen,0,(void *)&raddr,&raddr_len);//报式套接字每次通信都需要知道对方是谁
            if (raddr.sin_addr.s_addr != serveraddr.sin_addr.s_addr ||\
                        raddr.sin_port != serveraddr.sin_port){
                fprintf(stderr,"Ignore: address not match\n");
                continue;
            }
            if (rpkglen < sizeof(struct msg_channel_st)){
                fprintf(stderr,"message is too small.\n");
                continue;
            }
            if (msg_channel->channelid == chosenid){
                close(pipefd[0]);
                fprintf(stdout,"accepted msg:%d recvievced.\n",msg_channel->channelid);
                write(pipefd[1],msg_channel->data,rpkglen-sizeof(uint8_t));
            }

            break;
        }

        exit(0);
    }
}
