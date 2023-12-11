#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "client_req.h"
#include "dns_map.h"
#define DEFAULT_PORT 53
int main(int argc, char *argv[]) {
  long err;
  int port;
  unsigned int clilen;
  int sockfd, dns_server_sock_fd;
  struct sockaddr_in dns_server_addr, serv_addr, cli_addr;
  if (argc < 2)
    port = DEFAULT_PORT;
  else {
    err = strtol(argv[1], NULL, 10);
    port = errno != ERANGE ? (int)err : -1;
    if (port == 0) {
      printf("wrong port number");
      exit(1);
    }
  }
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  int ret = 0;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (ret != 0) {
    printf("yes");
  }
  clilen = sizeof(cli_addr);
  unsigned char *buf = (unsigned char *)malloc(1024);
  std::map<std::string, struct host_list *> records;
  int response_buf_len = 0;
  while (true) {
    ret = recvfrom(sockfd, buf, 1024, MSG_WAITALL, (struct sockaddr *)&cli_addr,
                   &clilen);

    DNS_HEADER *new_header = NULL;
    QUERY *new_query = NULL;
    parse_dns_request(buf, &new_header, &new_query);
    struct host_list *host_list = get_host_list(&records, new_query->name);
    unsigned char *response_buf;
    if (new_query->name == NULL && host_list == NULL) {
      new_query->name = (unsigned char *)malloc(2);
      sprintf((char *)new_query->name, "%s", ".");
      create_simple_record(&new_header, &new_query, &host_list);
      add_dns_record(&records, &new_query->name, &host_list);
    }
    host_list = get_host_list(&records, new_query->name);
    if (host_list != NULL) {
      response_buf = create_dns_response(new_header, new_query, host_list,
                                         &response_buf_len);
    } else {
      struct hostent *server = gethostbyname("8.8.8.8");
      bzero((char *)&dns_server_addr, sizeof(dns_server_addr));
      dns_server_addr.sin_family = AF_INET;
      bcopy(server->h_addr, (char *)&dns_server_addr.sin_addr.s_addr,
            (size_t)server->h_length);
      dns_server_addr.sin_port = htons(DEFAULT_PORT);
      dns_server_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
      unsigned int sin_size = sizeof(dns_server_addr);
      sendto(dns_server_sock_fd, buf, ret, 0,
             (struct sockaddr *)&dns_server_addr, sizeof(dns_server_addr));
      recvfrom(dns_server_sock_fd, buf, 1024, 0,
               (struct sockaddr *)&dns_server_addr, &sin_size);
      DNS_HEADER *response_header = NULL;
      QUERY *response_query = NULL;
      parse_dns_response(buf, &response_header, &response_query, &host_list);
      add_dns_record(&records, &(response_query->name), &host_list);
      response_buf = create_dns_response(response_header, response_query,
                                         host_list, &response_buf_len);
    }
    sendto(sockfd, response_buf, response_buf_len, 0,
           (struct sockaddr *)&cli_addr, sizeof(cli_addr));
    response_buf_len = 0;
  }
}

//haha
int test(){
    retutn 1;

}
int test_3(){
    retutn 3;

}