#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "server_conf.h"
#include "medialib.h"
#include "thr_channel.h"
#include "thr_list.h"
#include "../include//proto.h"


int serversd;
struct sockaddr_in sndaddr;
struct server_conf_st server_conf = {\
    .rcvport = DEFAULT_RECVPORT,\
    .media_dir = DEFAULT_MEDIADIR,\
    .runmode = RUN_FOREGROUND,\
    .ifname = DEFAULT_IF,
    .mgroup = DEFAULT_MGROUP};
static struct mlib_listentry_st *list;//节目单
 
static void print_help()
{
    printf("-M 指定多播组\n");
    printf("-P 指定接收端口\n");
    printf("-F 前台运行\n");
    printf("-D 指定媒体库\n");
    printf("-I 指定网卡设备\n");
    printf("-H show help\n");
}

static void daemon_exit(int s)
{
    thr_list_destroy();
    thr_channel_destroyall();
    mlib_freechnlist(list);
    //syslog(LOG_WARNING, "signal-%d caught, exit now.", s);
    //closelog();
    exit(0);
}

static int daemonize()
{
    pid_t pid;
    int fd;
    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "fork() failed:%s", strerror(errno));
        return -1;
    }

    if(pid > 0)//parent
        exit(0);

    fd = open("/dev/null", O_RDWR);
    if(fd < 0)
    {
        syslog(LOG_ERR, "open() failed:%s", strerror(errno));
        return -2;
    }
    else
    {
        /*close stdin, stdout, stderr*/
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        if(fd > 2)
            close(fd);
    }
    chdir("/");
    umask(0);
    setsid();
    return 0;

}

static int socket_init()
{
    struct ip_mreqn mreq;
    inet_pton(AF_INET, server_conf.mgroup, &mreq.imr_multiaddr);
    inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);//local address
    mreq.imr_ifindex = if_nametoindex(server_conf.ifname);//net card

    serversd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serversd < 0)
    {
        perror("socket()");
        //syslog(LOG_ERR, "socket():%s", strerror(errno));
        exit(1);
    }
    if(setsockopt(serversd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt()");
        //syslog(LOG_ERR, "setsockopt(IP_MULTICAST_IF):%s", strerror(errno));
        exit(1);
    }

    sndaddr.sin_family = AF_INET;
    sndaddr.sin_port = htons(atoi(server_conf.rcvport));
    inet_pton(AF_INET, server_conf.mgroup, &sndaddr.sin_addr);
    
    return 0;
}

int main(int argc, char * const argv[])
{
    /*variable*/
    int c;
    struct sigaction sa;

    /*signal content*/
    /*daemon process receive these signal, go to daemon_exit function*/
    sa.sa_handler = daemon_exit;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    //openlog("netradio", LOG_PID | LOG_PERROR, LOG_DAEMON);

    /*analyse argument*/
    while(1)
    {
        c = getopt(argc, argv, "M:P:FD:I:H");//:-->has parameter
        #ifdef DEBUG
            fprintf(stdout, "here2!\n");
        #endif
        printf("get command c:%c\n", c);
        if(c < 0)
            break;
        switch(c)
        {
            case 'M':
                server_conf.mgroup = optarg;
                break;
            case 'P':
                server_conf.rcvport = optarg;
                break;
            case 'F':
                server_conf.runmode = RUN_FOREGROUND;
                break;
            case 'D':
                server_conf.media_dir = optarg;
                break;
            case 'I':
                server_conf.ifname = optarg;
                break;
            case 'H':
                print_help();
                exit(0);
                break;
            default:
                abort();
                break;
        }
        break;
    }

    /*daemon process*/
    if(server_conf.runmode == RUN_DAEMON){
        fprintf(stdout, "欢迎使用音乐广播系统,已进入后台运行\n");
        if(daemonize() != 0){
            perror("daemonize()");
        }
    }
    else if(server_conf.runmode == RUN_FOREGROUND){
        fprintf(stdout, "欢迎使用音乐广播系统\n");
    }else
    {
        fprintf(stderr, "EINVAL\n");
        //syslog(LOG_ERR, "EINVAL server_conf.runmode.");
        exit(1);
    }
    
    /*SOCKET初始化*/
    socket_init();

    /*获取节目单信息*/
    int list_size;
    int err;
    //list 频道的描述信息
    //list_size有几个频道
    err = mlib_getchnlist(&list, &list_size);
    if(err)
    {   
        fprintf(stderr,"mliib_getchnlist():%s",strerror(errno));
        //syslog(LOG_ERR, "mlib_getchnlist():%s", strerror(errno));
        exit(1);
    }

    /*创建节目单线程*/
    thr_list_create(list, list_size);
    /*if error*/

    //创建频道线程
    int i = 0;
    for( i = 0; i < list_size; i++)
    {
        err = thr_channel_create(list + i);
        if(err)
        {
            fprintf(stderr, "thr_channel_create():%s\n", strerror(err));
            exit(1);
        }
    }
    //syslog(LOG_DEBUG, "%d channel threads created.", i);
    while(1)
        pause();
}
