#include <stdio.h>
#include <pthread.h>

#include "uds_common.h"

// #define RECV_BUF_SIZE 128

#define SELF_SOCK_NAME	"client.sock"
#define SRV_SOCK_NAME	"server.sock"

int is_work;

uds_config_t uds_config;

void handle_sigstop(int sig) 
{
	is_work = 0;
}

void recv_handler(struct sockaddr *from, socklen_t *fromlen, char *recv_buffer, int recv_buf_len)
{
	printf("From [%s] received [%s]\n", from->sa_data, recv_buffer);
}

int main(int argc, char *argv[])
{
	/* For uds_server */
	pthread_t uds_server_thr;

	/* For handling SIGINT */
	struct sigaction sa_stop_work;
	int uds_stop_signal = SIGRTMIN;

	/* Map of receivers */
	const int num_uds_receivers = 1;
	struct sockaddr receivers_list[num_uds_receivers];

	int programm_stop_signal = SIGINT;

	int ret;
	char buffer[RECV_BUF_SIZE];
	
	/* Allow to work */
	is_work = 1;

	/* Set handler for SIGINT */
	sa_stop_work.sa_handler = handle_sigstop;
	sigemptyset(&(sa_stop_work.sa_mask));
	sa_stop_work.sa_flags = 0;
	ret = sigaction(programm_stop_signal, &sa_stop_work, NULL);
	if (ret == -1) 
	{
		perror("sigaction()");
		return -1;
	}

	/* Init config for uds server */
	uds_init_config(&uds_config, uds_stop_signal, SELF_SOCK_NAME, recv_handler);

	/* Start uds server */
	pthread_create(&uds_server_thr, NULL, uds_server, (void *)(&uds_config));

	/* Form map of other uds_servers in other programs */
	receivers_list[0].sa_family = AF_UNIX;
	strcpy(receivers_list[0].sa_data, SRV_SOCK_NAME);

	/* Read from console and send to server */
	while (is_work) {
        printf("%s # ", argv[0]);
        // Используем fgets для безопасности
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0; // Убираем символ новой строки

        // Исправлено: Передаем &uds_config (указатель)
        uds_send(&uds_config, buffer, strlen(buffer), (struct sockaddr *)&receivers_list[0], sizeof(struct sockaddr_un));
    }

	/* End work */
	union sigval thr_data;
    thr_data.sival_ptr = (void *)&uds_config;
    pthread_sigqueue(uds_server_thr, uds_stop_signal, thr_data);

	/* Wait end of uds_server */
	pthread_join(uds_server_thr, NULL);

	/* Destroy uds_config */
	uds_destroy_config(&uds_config);	

	printf("\n%s successfult end work\n", argv[0]);

	return 0;
}