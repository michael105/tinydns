// @url RFC 1035 https://tools.ietf.org/html/rfc1035

#include "common.h"

#define DNS_PORT 53

#define version "0.3.1"



void loop(int sockfd)
{
	int16_t  i, n;
	uint16_t  id;
	uint16_t *ans = NULL;
	unsigned char buf[0xFFF];

	int                 inaddr_len;
	struct sockaddr_in inaddr;
	//struct sockaddr_in6 in_addr;

	int                out_socket;
	int                out_addr_len;
	struct sockaddr_in out_addr;

	memset((char *) &out_addr, 0, sizeof(out_addr));
	out_addr.sin_family = AF_INET;
	out_addr.sin_port   = htons(config.dns_port);
	inet_aton(config.dns, (struct in_addr *)&out_addr.sin_addr.s_addr);
	out_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (out_socket < 0) error("ERROR opening socket out");

	while (1)
	{
		memset(buf, 0, sizeof(buf));

		log_s("Listening\n");
		// receive datagram
		inaddr_len = sizeof(inaddr);
		n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &inaddr, &inaddr_len);
		log_s("Connect.\n");
		if (n < 1) continue;

		// clear Additional section, becouse of EDNS: OPTION-CODE=000A add random bytes to the end of the question
		// EDNS: https://tools.ietf.org/html/rfc2671
		THeader* ptr = (THeader*)buf;
		if (ptr->ARCOUNT > 0)
		{
			ptr->ARCOUNT = 0;
			i = sizeof(THeader);
			while (buf[i] && i < n) i += buf[i] + 1;
			n = i + 1 + 4; // COMMENT: don't forget end zero and last 2 words
		}
		// also clear Z: it's strange, but dig util set it in 0x02
		ptr->Z = 0;

		// logs the domain
		//parse_buf((THeader*)buf);

		id = *((uint16_t*)buf);

		log_b("Q-->", buf, n);

		if ( (ans = (uint16_t *)cache_search(buf, &n)) )
		{
			ans[0] = id;
			log_b("<--C", ans, n);
		}
		else
			cache_question(buf, n);

		// resend to parent
		if (!ans)
		{
			out_addr_len = sizeof(out_addr);
			n = sendto(out_socket, buf, n, 0, (struct sockaddr *) &out_addr,  out_addr_len);
			if (n < 0) { log_s("ERROR in sendto");  }

			int ck = 0;
			uint32_t pow, i;
			while (++ck < 13)
			{
				pow = 1; for (i=0; i<ck; i++) pow <<= 1;
				//printf("Sleep: %d\n",pow);
				usleep(pow * 1000);
				n = recvfrom(out_socket, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr *) &out_addr, &out_addr_len);
				if (n < 0) continue;

				cache_answer(buf, n);

				log_b("<--P", buf, n);
				if (id != *((uint16_t*)buf)) continue;

				ans = (uint16_t*)buf;
				break;
			}
			if (!ck) log_s("<--P no answer");
		}

		// send answer back
		if (ans)
		{
			n = sendto(sockfd, ans, n, 0, (struct sockaddr *) &inaddr, inaddr_len);
			if (n < 0) log_s("ERROR in sendto back");
		}
	}
}

#ifdef NETDB
#include <netdb.h>
int hostname_to_ip(char *hostname, char *ip, int len)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	if (getaddrinfo(hostname, NULL, &hints, &servinfo) != 0) return 0;

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
#ifdef IPV6
		if (p->ai_family == AF_INET6)
		{
			struct sockaddr_in6 *serveraddr = (struct sockaddr_in6 *)p->ai_addr;
			inet_ntop(AF_INET6, (struct in_addr *)&serveraddr->sin6_addr, ip, len);
			break;
		}
		else
#endif
			if (p->ai_family == AF_INET)
			{
				struct sockaddr_in *serveraddr = (struct sockaddr_in *)p->ai_addr;
				inet_ntop(AF_INET, (struct in_addr *)&serveraddr->sin_addr, ip, len);
			}
	}
	freeaddrinfo(servinfo);
	return 1;
}
#endif

int server_init()
{
	int sock;

#ifdef NETDB
	// convert domain to IP
	char buf[0xFF];
	if (hostname_to_ip(config.server_ip, buf, sizeof(buf)))
		config.server_ip = buf;
#endif

#ifdef IPV6
	// is ipv6?
	int is_ipv6 = 0, i = 0;
	while (config.server_ip[i]) if (config.server_ip[i++] == ':') { is_ipv6 = 1; break; }

	// create socket
	sock = socket(is_ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
#else
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif

	if (sock < 0) error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets
	 * us rerun the server immediately after we kill it;
	 * otherwise we have to wait about 20 secs.
	 * Eliminates "ERROR on binding: Address already in use" error.
	 */
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(optval));

	// bind
#ifdef IPV6
	if (is_ipv6)
	{
		struct sockaddr_in6 serveraddr;  /* server's addr */
		memset((char *) &serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin6_family = AF_INET6;
		serveraddr.sin6_port   = htons(config.server_port);
		inet_pton(AF_INET6, config.server_ip, (struct in_addr *)&serveraddr.sin6_addr.s6_addr);
		if (bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
			error("ERROR on binding ipv6");
	}
	else
#endif
	{
		struct sockaddr_in serveraddr;  /* server's addr */
		memset((char *) &serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port   = htons(config.server_port);
		inet_aton(config.server_ip, (struct in_addr *)&serveraddr.sin_addr.s_addr);
		printf("bind on %s:%d; parent: %s:%d\n", config.server_ip, config.server_port,config.dns,config.dns_port);
		if (bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
			error("ERROR on binding ipv4");
	}

	char s[0xFF];
	sprintf(s, "bind on %s:%d", config.server_ip, config.server_port);
	log_s(s);

	return sock;
}

int main(int argc, char **argv)
{
	for ( *argv++; *argv && argv[0][0]=='-'; *argv++ ){
		for ( char *c = argv[0]+1; *c != 0; c++ ){
			switch ( *c ){
				case 'v': 
					writes("tinydns " version "\nAuthor: CupIvan <mail@cupivan.ru>\nLicense: MIT\n" );
					exit(0);
				case 'd':{
								pid_t pid = fork();
								if (pid < 0)
								{
									if (pid < 0) error("Can't create daemon!");
									exit(1);
								}
								if (pid > 0) exit(0); // exit from current process 
							};
							break;
				case 'D':
							config.debug_level = 1;
							break;
				default:
							help();
			}
		}
	}



	config_load();

	int sockfd = server_init();

	loop(sockfd);
}
