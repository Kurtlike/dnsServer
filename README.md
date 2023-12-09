# Assignment **DNS**

The DNS client and server must be implemented. It is implied that the protocol will be implemented at a minimum working level. Allowed for the following restrictions:
 - Support only for Type A records.
 - The server address must be equal to the length of a domain name in a request. For example ya.ru. = 0.0.0.6 and test.ru. = 0.0.0.8
 - There is no need to specify authoritarian servers or additional records in the answer. Only directly the answers.

Description server options:
```sh
./server [port]
port by default 53
```

Descfiption client options:
```sh
./client dns_name [server] [port]
dns_name mandatory option
server by default localhost
port by default 53
```

By default, utilities must use 53 ports

To save time, use a sniffer-captured package as a query/response template. Wireshark has Export as C Arrays for this purpose.

To test a DNS server, use host, dig, or nslookup consoles, explicitly telling them to access the server on the same computer. Use any existing DNS server, such as Yandex DNS (77.88.8.8), to verify your DNS client.

[RFC DNS](https://tools.ietf.org/html/rfc1035) In addition, there are many simplified descriptions of the DNS protocol.
