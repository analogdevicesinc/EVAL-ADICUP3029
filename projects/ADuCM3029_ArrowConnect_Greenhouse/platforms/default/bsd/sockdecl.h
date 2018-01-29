#if !defined(_DEFAULT_SOCK_DECL_H_)
#define _DEFAULT_SOCK_DECL_H_

#include <bsd/struct_hostent.h>
#include "stddef.h"

typedef int ssize_t;

struct hostent *gethostbyname(const char *host);
int socket(int protocol_family, int socket_type, int protocol);
void soc_close(int socket);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t recvfrom(int sock, void *buf, size_t size, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int setsockopt(int sockfd, int level, int optname,
             const void *optval, socklen_t optlen);
int bind(int sock, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sock, int backlog);
int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);

#endif // _DEFAULT_SOCK_DECL_H_

