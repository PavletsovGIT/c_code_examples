#include <stdio.h>
#include <pthread.h>

#include "uds_common.h"

// #define RECV_BUF_SIZE 128

#define SELF_SOCK_NAME	"server.sock"
#define CLIENT_SOCK_NAME "client.sock"

int is_work;

uds_config_t uds_config;

void handle_sigstop(int sig) 
{
	/* Send stop signal to self thread */
	siginfo_t siginfo;
	siginfo.si_value.sival_ptr = (void *)&uds_config;
	uds_stop_handler(0, &siginfo, NULL);
}

void recv_handler(struct sockaddr *from, socklen_t *fromlen, char *recv_buffer, int recv_buf_len)
{
    printf("From [%s] received [%s]\n", from->sa_data, recv_buffer); // Безопасный вывод пути

    uds_send(&uds_config, recv_buffer, recv_buf_len, (struct sockaddr *)from, *fromlen);
}

/* Echo server */
int main(int argc, char *argv[])
{
	/* For handling SIGINT */
	struct sigaction sa_stop_work;

	/* Map of receivers */
	const int num_uds_receivers = 1;
	struct sockaddr receivers_list[num_uds_receivers];

	int ret;
	int uds_stop_signal = SIGRTMIN;
	
	/* Allow to work */
	is_work = 1;

	/* Set handler for SIGINT */
	sigemptyset(&(sa_stop_work.sa_mask));
    sa_stop_work.sa_flags = 0;
    sa_stop_work.sa_handler = handle_sigstop;
    ret = sigaction(SIGINT, &sa_stop_work, NULL);
	if (ret == -1) 
	{
		perror("sigaction()");
		return -1;
	}

	/* Init config for uds server */
	uds_init_config(&uds_config, uds_stop_signal, SELF_SOCK_NAME, recv_handler);
	
	/* Form map of other uds_servers in other programs */
	receivers_list[0].sa_family = AF_UNIX;
	strcpy(receivers_list[0].sa_data, CLIENT_SOCK_NAME);

	/* Start uds server */
	uds_server((void *)&uds_config);

	uds_destroy_config(&uds_config);

	printf("\n%s succsessfuly end work", argv[0]);

	return 0;
}