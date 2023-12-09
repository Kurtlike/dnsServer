#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include "client_req.h"
#include "dns_protocol.h"
#define DEFAULT_PORT 53
#define DEFAULT_SERVER "localhost"

int main(int argc, char *argv[]) {
  struct sockaddr_in serv_addr;
  long err;
  int port;
  char *dns_name;
  struct hostent *server;
  if (argc < 2) {
    fprintf(stderr, "wrong dns_name number");
    exit(1);
  }
  dns_name = argv[1];
  if (argc > 2) {
    server = gethostbyname(argv[2]);
  } else
    server = gethostbyname(DEFAULT_SERVER);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  if (argc < 4)
    port = DEFAULT_PORT;
  else {
    err = strtol(argv[3], NULL, 10);
    port = errno != ERANGE ? (int)err : -1;
    if (port == 0) {
      printf("wrong port number");
      exit(1);
    }
  }

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }
  QUERY query = create_req_query(dns_name);
  struct DNS_HEADER header = create_req_header();

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        (size_t)server->h_length);
  serv_addr.sin_port = htons(port);

  struct DNS_REQ dnsReq {};
  dnsReq.dnsHeader = header;
  dnsReq.query = query;
  char a[1000];
  bzero(a, 1000);
  int header_len = sizeof(header);
  memcpy(a, &header, header_len);
  int name_len = strlen((char *)(query.name));
  sprintf(a + header_len, "%s", (char *)(query.name));
  sprintf(a + header_len + name_len, "%c", 0);
  int q_len = sizeof(struct QUESTION);
  memcpy(a + header_len + name_len + 1, query.ques, q_len);
  int buf_size = header_len + name_len + 1 + q_len;
  sendto(sockfd, a, buf_size, 0, (struct sockaddr *)&serv_addr,
         sizeof(serv_addr));
  struct sockaddr_storage their_addr;
  unsigned int sin_size = sizeof(their_addr);
  char b[1000];
  bzero(b, 1000);
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
             sizeof timeout);

  recvfrom(sockfd, b, 100, 0, (struct sockaddr *)&their_addr, &sin_size);
  if (errno == (EAGAIN | EWOULDBLOCK)) {
    printf("timeout\n");
    return 1;
  } else {
    unsigned int shift = header_len + name_len + 1 + q_len;
    char *c = b + shift + 12;
    printf("%hhu.%hhu.%hhu.%hhu\n", c[0], c[1], c[2], c[3]);
  }
  return 0;
}
