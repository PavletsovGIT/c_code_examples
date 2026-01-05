#include "uds_common.h"

void uds_stop_handler(int signum, siginfo_t *info, void *context) 
{
	uds_config_t *config = (uds_config_t *)(info->si_value.sival_ptr);
	if (config) config->is_server_work = 0;
}

int uds_init_config(uds_config_t *config, int sig_stop, const char *sock_name, void (*recv_handler)(struct sockaddr *, socklen_t *, char *, int)) 
{
	config->is_server_work = 1;
	config->sigstop = sig_stop;

	strncpy(config->sock_file_name, sock_name, SOCK_FILE_NAME_LEN - 1);

	config->sa_stop_signal.sa_sigaction = uds_stop_handler;
	sigemptyset(&(config->sa_stop_signal.sa_mask));
	config->sa_stop_signal.sa_flags = SA_SIGINFO;
	sigaction(config->sigstop, &(config->sa_stop_signal), NULL);

	config->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (config->fd < 0) return -1;

	memset(&(config->addr), 0, sizeof(struct sockaddr_un));
	config->addr.sun_family = AF_UNIX;
	strncpy(config->addr.sun_path, config->sock_file_name, sizeof(config->addr.sun_path) - 1);

	unlink(config->sock_file_name); 

	if (bind(config->fd, (struct sockaddr *)&(config->addr), sizeof(struct sockaddr_un)) < 0) {
		perror("bind()");
		return -1;
	}

	config->recv_handler = recv_handler;
	return 0;
}

int uds_send(uds_config_t *config, char *buffer, size_t len, struct sockaddr *dst_addr, socklen_t dst_addr_len) 
{
	return sendto(config->fd, buffer, len, 0, dst_addr, dst_addr_len);
}

void *uds_server(void *arg) 
{
	uds_config_t *config = (uds_config_t *)arg;
	char recv_buffer[RECV_BUF_SIZE];
	struct sockaddr_un src_addr;

	assert(config->recv_handler == NULL);

	while (config->is_server_work) {
		memset(recv_buffer, 0, RECV_BUF_SIZE);

		socklen_t src_addr_len = sizeof(struct sockaddr_un);
		ssize_t len = recvfrom(config->fd, recv_buffer, RECV_BUF_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);

		if (len < 0) {
			if (errno == EINTR) continue;
			perror("recvfrom()");
			break;
		}

		config->recv_handler((struct sockaddr *)&src_addr, &src_addr_len, recv_buffer, (int)len);
	}
	return NULL;
}

void uds_destroy_config(uds_config_t *config) {
	if (config->fd != -1) close(config->fd);
	unlink(config->sock_file_name);
}