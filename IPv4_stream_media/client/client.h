#ifndef CLIENT_H__
#define CLIENT_H_

#define DEFAULT_PLAYER "/bin/mpg123 - > /dev/null"

//默认值结构体
struct client_conf_st{
    char *rcvport;
    char *mgroup;
    char *player;
};

extern struct client_conf_st client_conf;

#endif
