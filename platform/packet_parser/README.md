# packet_parser设计

## 模块功能

packet_parser模块旨在提供对网络接口经过流量的统计分析

## 模块设计

### 流量捕获

捕获接口流量的方式有多种，比如使用原始套接字、libpacp库接口等，这里选择使用原始套接字，在运行程序时需要`sudo`运行

### 数据统计

按照经验，需要统计展示的数据条目有如下几个方面

- tx/rx方向流量大小：报文数量，字节数
- 流量目的：单播、广播、组播
- 流量类型：ARP、IPV4等

为了便于扩展，考虑支持应用模块向packet_parser注册过滤条件，添加对应数据。注册包括：注册条件名、报文字段（etherType, ipProtocol等），packet_parser维护一个注册链表，在进行报文解析时进行遍历匹配。比如ARP模块注册统计ARP报文。

`packet_parser_register_filter`，模块调用这个接口将关注的报文字段注册到packet_parser中