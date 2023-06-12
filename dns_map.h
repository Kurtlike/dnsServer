//
// Created by kurtlike on 12/2/22.
//

#ifndef LAB2_DNS_MAP_H
#define LAB2_DNS_MAP_H
#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include "dns_protocol.h"
struct host_list{
  struct RES_RECORD* record;
  struct host_list* next;
};

unsigned char* parse_dns_request(unsigned char* buf, DNS_HEADER **h, QUERY **q){
  unsigned char* local_buf = buf;
  int header_size = sizeof(DNS_HEADER);
  DNS_HEADER *header = (DNS_HEADER*)malloc(header_size);
  memcpy(header,local_buf,header_size);
  local_buf+=header_size;
  QUERY *query = (QUERY*)malloc(sizeof(QUERY));
  uint8_t root_len = local_buf[0];
  local_buf++;
  if(root_len != 0){
    query->name = (unsigned char*)malloc(root_len+1);
    memcpy(query->name, local_buf, root_len);
    local_buf+=root_len;
    uint8_t next_len = local_buf[0];
    if(next_len != 0) {
      query->name[root_len] = '.';
      local_buf++;
      query->name =
          (unsigned char *)realloc(query->name, root_len + next_len + 2);
      memcpy(query->name + root_len + 1, local_buf, next_len);
      *(query->name + root_len + 1 + next_len) = '\0';
      local_buf += next_len;
    }
    local_buf[0]=0;
    local_buf++;
  }
  query->ques = (QUESTION*)malloc(sizeof(QUESTION));
  memcpy(&query->ques->qtype,local_buf,sizeof(query->ques->qtype));
  local_buf+=sizeof(query->ques->qtype);
  memcpy(&query->ques->qclass,local_buf,sizeof(query->ques->qclass));
  local_buf+=sizeof(query->ques->qclass);
  *h = header;
  *q = query;
  return local_buf;
}
int parse_dns_response(unsigned char* buf, DNS_HEADER **h, QUERY **q, struct host_list **r){
  unsigned char* local_buf = parse_dns_request(buf, h, q);
  struct host_list *root= (struct host_list*)malloc(sizeof(struct host_list));
  struct host_list *host_iter = root;
  unsigned short count = ntohs((*h)->ans_count);
  if((*h)->rcode == 0) {
    for (uint32_t i = 0; i < count; i++) {
      struct RES_RECORD *record =
          (struct RES_RECORD *)malloc(sizeof(struct RES_RECORD));
      memcpy(&record->beg_mark, local_buf, sizeof(record->beg_mark));
      local_buf += sizeof(record->beg_mark);
      memcpy(&record->offset, local_buf, sizeof(record->offset));
      local_buf += sizeof(record->offset);
      record->resource = (R_DATA *)malloc(sizeof(R_DATA));
      memcpy(&record->resource->type, local_buf,
             sizeof(record->resource->type));
      local_buf += sizeof(record->resource->type);
      memcpy(&record->resource->_class, local_buf,
             sizeof(record->resource->_class));
      local_buf += sizeof(record->resource->_class);
      memcpy(&record->resource->ttl, local_buf, sizeof(record->resource->ttl));
      local_buf += sizeof(record->resource->ttl);
      memcpy(&record->resource->data_len, local_buf,
             sizeof(record->resource->data_len));
      local_buf += sizeof(record->resource->data_len);
      unsigned short datalen = ntohs(record->resource->data_len);
      record->rdata = (unsigned char *)malloc(datalen);
      memcpy(record->rdata, local_buf, datalen);
      local_buf += datalen;
      host_iter->record = record;
      host_iter->next = (struct host_list *)malloc(sizeof(struct host_list));
      host_iter = host_iter->next;
    }
  }
  (*h)->rcode = 0;
  (*h)->auth_count = 0;
  (*h)->add_count = 0;
  count++;
  (*h)->ans_count = htons(count);
  struct RES_RECORD *record = (struct RES_RECORD*)malloc(sizeof(struct RES_RECORD));
  record->beg_mark = 192;
  record->offset = 12;
  record->resource = (R_DATA*)malloc(sizeof(R_DATA));
  record->resource->data_len = htons(4);
  record->resource->_class = htons(1);
  record->resource->ttl =  htonl(64);
  record->resource->type =  htons(1);

  record->rdata = (unsigned char *)malloc(4);
  char b[8];
  int name_len = strlen((char*)(*q)->name);
  sprintf( b,"0.0.0.%d", name_len);
  struct in6_addr addr;
  inet_pton(AF_INET,b,&addr);
  memcpy((char*)record->rdata,&addr,sizeof(struct in6_addr));
  host_iter->record = record;
  host_iter->next = NULL;
  (*r) = root;
  return 0;
}
int create_simple_record(DNS_HEADER **h, QUERY **q, struct host_list **r){
  struct host_list *root= (struct host_list*)malloc(sizeof(struct host_list));
  struct RES_RECORD *record = (struct RES_RECORD*)malloc(sizeof(struct RES_RECORD));
  record->beg_mark = 192;
  record->offset = 12;
  record->resource = (R_DATA*)malloc(sizeof(R_DATA));
  record->resource->data_len = htons(4);
  record->resource->_class = htons(1);
  record->resource->ttl =  htonl(64);
  record->resource->type =  htons(1);
  record->rdata = (unsigned char *)malloc(4);
  char b[8];
  int name_len = strlen((char*)(*q)->name);
  sprintf( b,"0.0.0.%d", name_len);
  struct in6_addr addr;
  inet_pton(AF_INET,b,&addr);
  memcpy((char*)record->rdata,&addr,sizeof(struct in6_addr));
  root->record = record;
  root->next = NULL;
  (*r) = root;
  (*h)->ans_count = htons(1);
  (*h)->qr = 1;
  (*h)->add_count = htons(0);
  return 1;
}
int add_dns_record(std::map <std::string,struct host_list*> *records, unsigned char** host, struct host_list **res_list){
  std::map <std::string,struct host_list*> :: iterator it;
  std::string host_name((char*)*host);
  it = (*records).find(host_name);
  if(it == (*records).end()){
    (*records).insert(std::make_pair(host_name,*res_list));
  }
  return 0;
}
unsigned char*  create_dns_response(DNS_HEADER *header,QUERY *query, struct host_list *res_list, int* len){
  int buf_len= 0;
  unsigned char* res_buf = (unsigned char*)malloc(1024);
  unsigned char* local_buf = res_buf;
  struct host_list *c = res_list;
  int q_c = 0;
  while(c!= NULL){
    c=c->next;
    q_c++;
  }
  header->ans_count = htons(q_c);
  int header_size = sizeof(DNS_HEADER);
  memcpy(local_buf, header, header_size);
  local_buf+=header_size;
  buf_len+=header_size;
  int name_len;
  if(query->name[0]=='.'){
    name_len = 1;
    memcpy(local_buf, "\0", name_len);
  }else{
    name_len = strlen((char*)query->name)+2;
    memcpy(local_buf, createQNAME((char*)query->name), name_len);
  }
  local_buf+=name_len;
  buf_len+=name_len;
  unsigned short qtype =htons(1);
  memcpy(local_buf,&qtype,sizeof(query->ques->qtype));
  local_buf+=sizeof(query->ques->qtype);
  buf_len+=sizeof(query->ques->qtype);
  memcpy(local_buf,&qtype,sizeof(query->ques->qclass));
  local_buf+=sizeof(query->ques->qclass);
  buf_len+=sizeof(query->ques->qclass);
  struct host_list *res = res_list;
  while(res != NULL){
    memcpy(local_buf,&res->record->beg_mark,sizeof(res->record->beg_mark));
    local_buf+=sizeof(res->record->beg_mark);
    buf_len+=sizeof(res->record->beg_mark);
    memcpy(local_buf,&res->record->offset,sizeof(res->record->offset));
    local_buf+=sizeof(res->record->offset);
    buf_len+=sizeof(res->record->offset);
    buf_len++;
    memcpy(local_buf,res->record->resource,sizeof(R_DATA));
    local_buf+=sizeof(R_DATA);
    buf_len+=sizeof(R_DATA);
    unsigned short datalen = ntohs(res->record->resource->data_len);
    memcpy(local_buf,res->record->rdata,datalen);
    local_buf+=datalen;
    buf_len+=datalen;
    res = res->next;
  }
  *len = buf_len;
  return res_buf;
}

struct host_list* get_host_list(std::map <std::string,struct host_list*> *records, unsigned char* host){
  if(host == NULL) return NULL;
  std::map <std::string,struct host_list*> :: iterator it;
  std::string host_name((char*)host);
  it = (*records).find(host_name);
  if(it == (*records).end()) return  NULL;
  return it->second;
}

#endif  // LAB2_DNS_MAP_H
