#ifndef __UDS_COMMON_H__
#define __UDS_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#define SOCK_FILE_NAME_LEN 64
#define RECV_BUF_SIZE 512

typedef struct {
    int fd;
    struct sockaddr_un addr;
    socklen_t addr_len;
    int sigstop;
    struct sigaction sa_stop_signal;
    volatile int is_server_work;
    char sock_file_name[SOCK_FILE_NAME_LEN];
    void (*recv_handler)(struct sockaddr *from, socklen_t *fromlen, char *recv_buffer, int recv_buf_len);
} uds_config_t;

void uds_stop_handler(int signum, siginfo_t *info, void *context);
int uds_init_config(uds_config_t *config, int sig_stop, const char *sock_name, void (*recv_handler)(struct sockaddr *, socklen_t *, char *, int));
void uds_destroy_config(uds_config_t *config);
int uds_send(uds_config_t *config, char *buffer, size_t len, struct sockaddr *dst_addr, socklen_t dst_addr_len);
void *uds_server(void *arg);

#endif