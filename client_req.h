//
// Created by kurtlike on 11/24/22.
//

#ifndef LAB2_CLIENT_REQ_H
#define LAB2_CLIENT_REQ_H
#include <stdio.h>
#include "dns_protocol.h"
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
struct DNS_REQ{
  struct DNS_HEADER dnsHeader;
  QUERY query;
};
char* createQNAME(char* url){
  int qname_len = 0;
  int pre_len = 0;
  char* label;
  char* QNAME =  (char*)malloc(1);
  label = strtok(url,".");
  while(label != NULL){
    char len = (char)strlen(label);
    qname_len+= len + 1;
    QNAME = (char*)realloc(QNAME,sizeof(char) * qname_len );
    sprintf(QNAME + pre_len,"%c", len);
    strcpy(QNAME + pre_len + 1,label);
    pre_len+= qname_len;
    label = strtok(NULL,".");
  }
  return QNAME;
}
QUERY create_req_query(char* url){
  struct QUESTION *question = (struct QUESTION*)malloc(sizeof(struct QUESTION));
  question->qtype = htons(1);
  question->qclass = htons(1);
  QUERY query;
  query.name = (unsigned char*)createQNAME(url);
  query.ques = question;
  return query;

}
struct DNS_HEADER create_req_header(){
  struct DNS_HEADER header{};
  header.id = 1;
  header.qr = 0;
  header.opcode = 0;
  header.tc = 0;
  header.rd = 1;
  header.q_count = htons(1);
  return  header;
}
#endif  // LAB2_CLIENT_REQ_H
