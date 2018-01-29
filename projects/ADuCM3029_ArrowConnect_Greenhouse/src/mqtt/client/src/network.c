#include "mqtt/client/network.h"

#include <bsd/socket.h>
#include <debug.h>
#include <bsd/inet.h>
#include <arrow/mem.h>
#include <ssl/ssl.h>
#if defined(__USE_STD__)
# include <errno.h>
#endif
#if defined(__XCC__)
#define WOLFSSL SSL
#define WOLFSSL_CTX SSL_CTX
#define IPPROTO_TCP 0
#define SSL_SUCCESS 0
#define SSL_INBUF_SIZE               6000
#define SSL_OTA_INBUF_SIZE           20000
#define SSL_OUTBUF_SIZE              3500
# define wolfSSL_write qcom_SSL_write
# define wolfSSL_read qcom_SSL_read
#define strerror(...) "xcc error"
#endif

static int _read(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    if ((int)interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0)) {
        interval.tv_sec = 0;
        interval.tv_usec = 100;
    }

    setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    int bytes = 0;
    int rc;
    while (bytes < len) {
//    	if (rc) DBG("mqtt recv ---%d", timeout_ms);
#if defined(MQTT_CIPHER)
    	rc = ssl_recv(n->my_socket, (char*)(buffer + bytes), (uint16_t)(len - bytes));
#else
    	rc = recv(n->my_socket, (char*)(buffer + bytes), (uint16_t)(len - bytes), 0);
#endif
//      if (rc) DBG("mqtt recv %d/%d", rc, len);
//      char * hexBuffer;
//      hexBuffer = (char*) malloc((len * 3) + 1);
//      memset(hexBuffer, 0, (len*3) + 1);
//      char * hex[4];
//      for (int i=0; i<len; i++) {
//     	 sprintf(hex, "%x ", buffer[i]);
//     	 strcat(hexBuffer, hex);
//      }
//      DBG("mqtt recv (%d): %s", len, (char*) hexBuffer)
//      free(hexBuffer);
        if (rc < 0) {
#if defined(errno) && defined(__linux__) && defined(MQTT_DEBUG)
            DBG("error(%d): %s", rc, strerror(errno));
#endif
            bytes = -1;
            break;
        } else
            bytes += rc;
    }
    return bytes;
}


static int _write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    struct timeval tv = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    int rc = 0;
#if defined(MQTT_CIPHER)
//    DBG("mqtt send %d", len);
     rc = ssl_send(n->my_socket, (char*)buffer, len);
#else
     rc = send(n->my_socket, (char*)buffer, len, 0);
//     char * hexBuffer;
//     hexBuffer = (char*) malloc((len * 3) + 1);
//     memset(hexBuffer, 0, (len*3) + 1);
//     char * hex[4];
//     for (int i=0; i<len; i++) {
//    	 sprintf(hex, "%x ", buffer[i]);
//    	 strcat(hexBuffer, hex);
//     }
//     DBG("mqtt send (%d): %s", len, (char*) hexBuffer)
//     free(hexBuffer);
#endif
    return rc;
}


void NetworkInit(Network* n) {
    n->my_socket = -1;
    n->mqttread = _read;
    n->mqttwrite = _write;
}

void NetworkDisconnect(Network* n) {
# if defined(MQTT_CIPHER)
    ssl_close(n->my_socket);
#endif
    soc_close(n->my_socket);
}

int NetworkConnect(Network* n, char* addr, int port) {
    struct sockaddr_in serv;
    struct hostent *serv_resolve;
    DBG("resolve %s", addr);
    serv_resolve = gethostbyname(addr);
    if (serv_resolve == NULL) {
        DBG("MQTT ERROR: no such host %s", addr);
        return -1;
    }
    memset(&serv, 0, sizeof(serv));

    if (serv_resolve->h_addrtype == AF_INET) {
        serv.sin_family = PF_INET;
        bcopy((char *)serv_resolve->h_addr,
                (char *)&serv.sin_addr.s_addr,
                (size_t)serv_resolve->h_length);
        serv.sin_port = htons((uint16_t)port);
    } else
        return -1;

    n->my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( n->my_socket < 0 ) {
        DBG("MQTT connetion fail %d", n->my_socket);
        return n->my_socket;
    }
    if ( connect(n->my_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        soc_close(n->my_socket);
        return -2;
    }
    DBG("cipher socket %d", n->my_socket);
#if defined(MQTT_CIPHER)
    if ( ssl_connect(n->my_socket) < 0 ) {
    	soc_close(n->my_socket);
    	return -3;
    }
#endif
    return n->my_socket;
}
