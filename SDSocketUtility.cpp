#include "SDSocketUtility.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

namespace hpf
{

using namespace std;

int SDSocketUtility::listen_to(const char * ip,int port, struct sockaddr_in* addr, bool is_block)
{
	int res;

	int fd = create_socket();
	if (fd < 0) 
    {
        fprintf(stderr,"create socket failed!\n");
		return -1;
	}

    bool result = false;
    if (is_block)
    {
        result = set_block(fd);
    }
    else
    {
        result = set_nonblock(fd);
    }

    if (result==-1)
    {
        fprintf(stderr,"set block failed!\n");
        return -1;
    }

	res = set_reuse(fd);
	if (res < 0) 
    {
        fprintf(stderr,"set reuse failed!\n");
		close_socket(fd);
		return -1;
	}

	make_address(addr, ip, port);

	res = bind(fd, (const struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in));
	if ( res < 0) 
    {
        fprintf(stderr,"bind failed!\n");
		close_socket(fd);
		return -1;
    }

	res = listen(fd, SOMAXCONN);
    if ( res < 0) 
    {
        fprintf(stderr,"listen failed!\n");
		close_socket(fd);
		return -1;
    }

	return fd;
}

int SDSocketUtility::bind_to(const char * ip,int port, struct sockaddr_in* addr)
{
	int res;

	int fd = create_socket_udp();
	if (fd < 0) 
    {
		return -1;
	}

	res = set_reuse(fd);
	if (res < 0) 
    {
		close_socket(fd);
		return -1;
	}

	make_address(addr, ip, port);

	res = bind(fd, (const struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr_in));
	if ( res < 0) 
    {
		close_socket(fd);
		return -1;
    }

	return fd;
}

int SDSocketUtility::connect_to(const char * ip,int port, struct sockaddr_in* addr)
{
	int res;

	int fd = create_socket();
	if (fd < 0) 
    {
		return -1;
	}


	make_address(addr, ip, port);

	res = connect(fd, (const struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr));
	if ( res < 0) 
    {
		close_socket(fd);
		return -1;
    }

	return fd;
}

int SDSocketUtility::nonblock_connect_to(const char * ip,int port, struct sockaddr_in* addr)
{
	int res;

	int fd = create_socket();
	if (fd < 0) 
    {
		return -1;
	}

	int flags = fcntl(fd,F_GETFL,0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	   
	make_address(addr, ip, port);

	res = connect(fd, (const struct sockaddr*)addr, (socklen_t)sizeof(struct sockaddr));
	if ( res < 0) 
    {
        if(errno == EINPROGRESS)
        {
            return fd;
        }

        close_socket(fd);
        return -1;
    }
	return fd;
}


int SDSocketUtility::accept_client(int fd, struct sockaddr_in* addr)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	socklen_t addr_len = sizeof(struct sockaddr_in);

	int client_fd = accept(fd, (struct sockaddr*)addr, &addr_len);
    if (client_fd < 0) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
        } 
        else if (errno == EMFILE) 
        {
            fprintf(stderr, "Too many open connections\n");
        } 
        else 
        {
            perror("accept()");
        }

        return -1;
    }

	return client_fd;
}

void SDSocketUtility::make_address(struct sockaddr_in* addr, const char* ip, int port)
{
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);
}

int SDSocketUtility::recv_data(int fd,char * buffer, int buffer_len)
{
	int total_recv_bytes = buffer_len;
	while (buffer_len > 0) 
    {
		int recv_bytes = recv(fd, buffer, buffer_len, 0);
		if(recv_bytes <= 0) 
        {
			if (errno == EINTR) 
            {
				continue;
			}
			return recv_bytes;
		}

		buffer += recv_bytes;
		buffer_len -= recv_bytes;
	}

	return total_recv_bytes;
}

int SDSocketUtility::send_data(int fd, const char* buffer, int buffer_len)
{
	int total_send_bytes = buffer_len;
	while (buffer_len > 0)
	{
		int send_bytes = send(fd, buffer, buffer_len, MSG_NOSIGNAL);
		if(send_bytes <= 0) 
        {
			if (errno == EINTR) 
            {
				continue;
			}
			return send_bytes;
		}

		buffer += send_bytes;
		buffer_len -= send_bytes;
	}

	return total_send_bytes;
}

int SDSocketUtility::recv_data(int fd,char * buffer,int buffer_len, struct sockaddr* from, socklen_t* fromlen)
{
	return recvfrom(fd, buffer, buffer_len, 0, from, fromlen);
}

int SDSocketUtility::send_data(int fd, const char* buffer, int buffer_len, struct sockaddr* to, socklen_t tolen)
{
	return sendto(fd, buffer, buffer_len, 0, to, tolen);
}

string SDSocketUtility::ip_to_string(uint32_t ip)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = ip;
	return string(inet_ntoa(ip_addr));
}

int SDSocketUtility::set_block(int fd)
{
	int flag = 0;
	if (( flag = fcntl( fd, F_GETFL ) ) < 0
		|| fcntl( fd, F_SETFL, (flag & (~O_NONBLOCK)) ) < 0) 
    {
		return -1;
	}

	return 0;
}

int SDSocketUtility::set_nonblock(int fd)
{
	int flag = 0;
	if (( flag = fcntl( fd, F_GETFL ) ) < 0
		|| fcntl( fd, F_SETFL, (flag | (O_NONBLOCK)) ) < 0) 
    {
		return -1;
	}

	return 0;
}

int SDSocketUtility::set_reuse(int fd)
{
	int reuse = 1;
	return setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) );
}

int SDSocketUtility::set_recv_timeout(int fd, int timeout_ms)
{	
	struct timeval time;
	time.tv_sec = timeout_ms / 1000;
	time.tv_usec = (timeout_ms % 1000) * 1000;

	return setsockopt( fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time) );	
}

int SDSocketUtility::set_send_timeout(int fd, int timeout_ms)
{	
	struct timeval time;
	time.tv_sec = timeout_ms / 1000;
	time.tv_usec = (timeout_ms % 1000) * 1000;

	return setsockopt( fd, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof(time) );	
}

int SDSocketUtility::set_recv_bufsize(int fd, int bufsize)
{
	return setsockopt( fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize) );
}

int SDSocketUtility::set_send_bufsize(int fd, int bufsize)
{
	return setsockopt( fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize) );
}

int SDSocketUtility::set_linger(int fd, int onoff, int linger)
{
	struct linger my_linger;
	my_linger.l_onoff = onoff;
	my_linger.l_linger = linger;

	return setsockopt( fd, SOL_SOCKET, SO_LINGER, &my_linger, sizeof(my_linger) );
}

int SDSocketUtility::create_socket()
{
	return socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
}

int SDSocketUtility::create_socket_udp()
{
	return socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
}

int SDSocketUtility::close_socket(int fd)
{
	return close( fd );
}

void SDSocketUtility::get_local_ipport(int sock_fd, unsigned int& ip, unsigned int& port)
{
	ip = 0;
	port = 0;
	struct sockaddr_in local_addr;
	int len = sizeof(local_addr);
	if(!getsockname(sock_fd, (struct sockaddr*)&local_addr, (socklen_t*)&len))
	{
		ip   = (unsigned int)(local_addr.sin_addr.s_addr);
		port = (unsigned int)(local_addr.sin_port);
	}
}

void SDSocketUtility::get_peer_ipport(int sock_fd, unsigned int& ip, unsigned int& port)
{
	ip = 0;
	port = 0;
	struct sockaddr_in peer_addr;
	int len = sizeof(peer_addr);
	if(!getpeername(sock_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&len))
	{
		ip	 = (unsigned int)(peer_addr.sin_addr.s_addr);
		port = (unsigned int)(peer_addr.sin_port);
	}
}

}
