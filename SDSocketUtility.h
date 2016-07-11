#ifndef SD_SOCKET_UTILITY_H
#define SD_SOCKET_UTILITY_H

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "SDTypes.h"
#include <string>

namespace hpf
{

class SDSocketUtility
{
public:	
    static void change_bytes_order( char*buffer, int size /*bytes*/) { return ;}
	static int listen_to(const char* ip, int port, struct sockaddr_in* addr, bool is_block);
	static int bind_to(const char* ip, int port, struct sockaddr_in* addr);
	static int connect_to(const char* ip, int port, struct sockaddr_in* addr);
	static int nonblock_connect_to(const char * ip,int port, struct sockaddr_in* addr);
		
	static int accept_client(int fd, struct sockaddr_in* addr);
	static void make_address(struct sockaddr_in* addr, const char* ip, int port);

	static int recv_data(int fd, char* buffer, int buffer_len);
	static int send_data(int fd, const char* buffer, int buffer_len);

	static int recv_data(int fd, char* buffer, int buffer_len, struct sockaddr* from, socklen_t* fromlen);
	static int send_data(int fd, const char* buffer, int buffer_len, struct sockaddr* to, socklen_t tolen);

	static std::string ip_to_string(uint32_t ip);

	static int set_block(int fd);
	static int set_nonblock(int fd);
	static int set_reuse(int fd);
	static int set_recv_timeout(int fd, int timeout_ms);
	static int set_send_timeout(int fd, int timeout_ms);
	static int set_recv_bufsize(int fd, int bufsize);
	static int set_send_bufsize(int fd, int bufsize);
	static int set_linger(int fd, int onoff, int linger);

	static int create_socket();
	static int create_socket_udp();
	static int close_socket(int fd);

	static void get_peer_ipport(int sock_fd, unsigned int& ip, unsigned int& port);
	static void get_local_ipport(int sock_fd, unsigned int& ip, unsigned int& port);
};

}
#endif // SD_SOCKET_UTILITY_H

