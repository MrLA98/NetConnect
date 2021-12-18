# Socket

## 1. 基本概念

### 1.1 socket是什么

- socket -- 插座
  - 运行在计算机中的两个程序通过socket建立起一个通道，传输数据
  - 其包装好了TCP/IP协议族
- 两种socket
  - stream -- TCP
    - 需要建立和维持连接，双向、可靠
  - datagram -- UDP
    - 不需要建立和维持连接，不可靠、但效率高
  - 主要讲stream -- 流socket

### 1.2 基本的通信方法

- 流程：（clint/server端）

```cpp
// 客户端
socket(); // 1. 创建流socket
connect(); // 2. 服务器发起连接请求
send()/recv(); // 3. 发送接收数据
close(); // 4. 关闭socket链接，释放资源
// 服务端
socket(); // 1. 创建流socket
bind(); // 2. 指定用于通信的ip地址和端口
listen(); // 3. 把socket设置为监听模式
accept(); // 4. 接受客户端的连接
send()/recv(); // 5. 发送接收数据
close(); // 6. 关闭socket链接，释放资源
// 两者通过 `send()/recv()` 来通信
// 注意防火墙要开放端口
```

### 1.3 注意事项

- unix系统中，一切输入输出设备都是文件
  - socket函数返回值本质是一个__文件描述符__
- ipv4是啥？
  - 如`192.168.190.134`
  - 四个 `0~255 `的数字组成 -- 4个字节 -- 32位
  - `0.0.0.0` ~ `255.255.255.255`
- ipv6是啥？
  - ipv4不够用(2^32^~4.295x10^9^个地址)
  - 8个16进制数组成 -- 16字节 -- 128位，如：
  - `2001:DB8:0:23:8:800:200C:417A`
  - `0:0:0:0:0:0:0:` ~ `ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff`
  - 约有(2^128^~3.4x10^38^)

## 2. socket 函数

> 头文件 -- `<sys/socket.h>`

### 2.1 函数声明：

```cpp
int socket(int domain, int type, int protocol);
```

- domain - 协议域/族
  - 常用的有`AF_INET、AF_INET6、AF_LOCAL` -- ipv4, ipv6, local commuication, 
  - 决定了socket的地址类型，在通信中必须采用对应地址
- type - 指定socket类型
  - 常用的有`SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_PACKET, SOCK_SEQPACKET`
  - 主要用`SOCK_STREAM` -- 基于tcp，面向连接
- protocol - 指定协议
  - 常用协议：`IPPROTO_TCP, IPPROTO_UDP, IPPROTO_STCP, IPPROTO_TIPC` -- tcp、udp、stcp、tipc协议
- 返回值：
  - 成功返回一个socket
  - 失败返回-1，错误原因存在errno中

一般的使用方法

```cpp
// 常用写法
int listenfd; // 本质：文件描述符
1
if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    // 创建失败
    perror("socket"); // 标识错误
    return -1;
}
```

### 2.2 注意事项

- 单个进程一共可以打开多少个socket？
  - 单个进程最多可以打开，1024个
  - 系统定义的限制 -- 单个进程以及整个系统能打开多少个都有限制
  - 修改方法：
    - `[user@Ubuntu ~]$ ulimit -a` 查看限制
    - `[user@Ubuntu ~]$ ulimit -HSn 200 0` 改成2000个

## 3. 设置ip和端口

### 3.1 主机子节序和网络子节序

- 字节顺序 -- 如`0x12345678`
  - 大端 -- `12:34:56:78`
  - 小端 -- `78:56:34:12`
  - 低地址 ---> 高地址
  - 大于一个字节就需要考虑字节顺序
- 主机字节顺序 -- 不同机器不同、不同语言不同
  - x86、arm -- 小端
  - Power PC、MIPS UNIX -- 大端
  - JVM -- 大端
  - C/C++ -- 取决于cpu
- 网络字节顺序 -- 大端
  - TCP/IP规定的，与操作系统、cpu无关
- 转换函数
  - `htons(), ntohs()` -- 16位无符号数
  - `htonl(), ntohl()` -- 32位无符号数
  - h - host，n - network，s - short，l - long
- 端口和地址

```markdown
// 网络字节 -- 大端
ipv4 : 192.168.190.134
二进制: 11000000.10101000.10111110.10000110
十进制: 3232284292

// 主机字节 -- 小端
二进制: 10000110.10111110.10101000.11000000
十进制: 2260641984
等效于: 134.190.168.190
```

### 3.2 sockaddr 结构体

> 头文件：
>
> sockaddr -- `<sys/socket>`
>
> sockaddr_in, in_addr -- `<netinet/in>` 或 `<arpa/inet.h>` 两个里面都有
>
> inet_addr() -- `<arpa/inet.h>`

- 三个结构体的细节

```cpp
// 以前的，用起来不方便
struct sockaddr{
    unsigned short sa_family; // 地址类型
    char sa_data[14]; // 14字节端口和地址
};
// 现在常用的
struct sockaddr_in{
    short int sin_family; // 地址类型
    unsigned short int sin_port; // 端口号
    struct in_addr sin_addr; // 地址
    unsigned char sinzero[8]; // 保持长度一致
};
struct in_addr{
	unsigned long s_addr; // 地址    
};
```

- 用来存放地址类型、地址、端口号

__使用方法__

```cpp
// 服务端
// 2.1 创建地址结构体
sockaddr_in servAddr; 
// 2.2 初始化结构体
memset(&servAddr, 0, sizeof(servAddr));
// 2.3 协议族
servAddr.sin_family = AF_INET; 
// 2.4 给定ip地址
servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
// 2.4 给定ip地址
// servAddr.sin_addr.s_addr = inet_addr("192.168.90.134"); 
// 2.5 指定通信端口，并且小端转大端
servAddr.sin_port = htonl(atoi(argv[1])); 
```

### 3.3 解析域名

- 结构体 `hostent`
  - 头文件 -- `<unistd.h>`

```cpp
struct hostent{
    char *h_name; // 主机名
    char **h_aliases; // 主机所有别名
    int h_addrtype; // 地址类型
    int h_length; // ip地址长度
    char **h_addr_list; // 主机ip地址，网络字节序 
}
#define h_addr h_addr_list[0]
```

- 使用

 ```cpp
 hostent *h;
 h = gethostbyname(argv[1]);
 ```

- 可以将ip地址、域名、主机名变成地址
  - 失败返回 `0`

### 3.4 其他注意事项

- 修改ip
  - `[root@Ubuntu ~]# vi /etc/hosts`
  - `192.168.190.134 Ubuntu www.lh.com`
  - ping哪个都行
- 三个常用函数

```cpp
// 字符ip地址转换为网络字节序ip地址
int inet_aton(const char *cp, in_addr *inp);
// 网络字节序地址转化成字符串ip地址
char* inet_ntoa(in_addr in);
// 字符ip地址转化成网络字节序ip地址
in_addr_t inet_addr(const char *cp);
```

## 4. bind() 函数

- socket端口范围[0~65535]：
  - 1024以下是系统保留，需要root权限
  - 65535以上就没了(16位表示)
- 声明

```cpp
int bind(int, const struct sockaddr *, socklen_t);
```

- 使用

```cpp
if(bind(listenfd, (sockaddr*)&servAddr, sizeof(servAddr)) != 0){
    perror("bind");
    // 关闭socket -- 该函数在<unistd.h>中
    close(listenfd); 
    return -1;
}
```

## 5. listen(), accept() 和 connect() 函数

### 5.1 listen()

- 把服务端的socket设置为监听模式

- 声明

```cpp
int listen(int, int);
```

- 使用

```cpp
// 服务端
if(listen(listenfd, 3) != 0){
    perror("listen");
    close(listenfd); 
    return -1;
}
```

### 5.2 accept 和connect

- 声明

```cpp
int accept(int, sockaddr*, socklen_t*);

int connect(int, const sockaddr *, socklen_t);
```

- 使用

```cpp
// 服务端 -- accept
int clientfd;
sockaddr_in cliAddr;
int sAddrLen = sizeof(sockaddr_in);
clientfd = accept(listenfd, (sockaddr*)&cliAddr, (socklen_t*)&sAddrLen);

// 客户端 -- connect
if(connect(sockfd, (sockaddr*)&servAddr, sizeof(servAddr)) != 0){
    perror("connect");
    close(sockfd); 
    return -1;
}
```

### 5.3 注意的点

- 监听时，connect请求socket会排成队列
  - accept从队列中取出一个，即为其返回值
    - 用于和客户端通信
  - 如果队列为空，accept就阻塞等待
- 服务端调用listen()之前，客户端不能发起连请求
- 客户端用connect()函数发起连接请求
- listen的socket只用于建立连接

- 查看当前队列状态`$ netstat -na|grep 5005`
  - ESTABLISH
  - SYN_RECV

- accept()取到的地址是客户端的出口地址

## 6. send和recv

### 6.1 声明

```cpp
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

- sockfd为建立好连接的socket
- buf为需要发送数据的内存地址
- len为数据长度
- flag填0即可
- 返回已发送字符数，错误时反回-1

```cpp
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

- sockfd为建立好连接的socket
- buf为需要发送数据的内存地址
- len为需要接受的数据长度，不能超过buf大小
- flag填0即可
- 返回已接受的字符数，错误时返回-1

### 6.2 使用方法

```cpp
// 发
if((iret = send(sockfd, buf, strlen(buf), 0)) <= 0){
    perror("send");
    break;
}
// 收
if((iret = recv(sockfd, buf, sizeof(buf), 0)) <= 0){
    std::cout<< "iret = " << iret <<std::endl;
    break;
}
```

- 缓存区大概能存100,000量级的数据，之后再发可能就发不出去了
