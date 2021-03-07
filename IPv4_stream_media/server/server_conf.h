#ifndef SERVER_CONF_H_
#define SERVER_CONF_H_

#define DEFAULT_MEDIADIR "/home/yixingwei/音乐/Media"
#define DEFAULT_IF "wlp7s0"

enum
{
    RUN_DAEMON = 1,
    RUN_FOREGROUND
};
struct server_conf_st
{
    char *rcvport;
    char *mgroup;
    char *media_dir;
    char runmode;
    char *ifname;
};
extern struct server_conf_st server_conf;
extern int serversd;
extern struct sockaddr_in sndaddr;
#endif
