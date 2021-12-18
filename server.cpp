#include <iostream>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BLUE "\033[34m"
#define RESET "\033[0m"
#define PORT 5002

int main(int args, char** argv){
// 1. 创建socket
    // 1.1 创建文件描述符
    int listenfd; // 负责处理listen处的连接请求
    // 1.2 创建流socket -- ipv4，流socket
    if((listenfd = socket(AF_INET, SOCK_STREAM , 0)) == -1){
        perror("socket");
        return -1;
    }
    // 1.3 设置SO_REUSEADDR
    int opt = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));

// 2. 通信用的地址和端口绑定到socket上
    // 2.1 创建地址结构体
    sockaddr_in servAddr;  // 该结构体在<netinet/in.h>和<arpa/inet.h>中都有
    // 2.2 初始化结构体
    memset(&servAddr, 0, sizeof(servAddr));
    // 2.3 协议族，选ipv4 和 socket保持一致
    servAddr.sin_family = AF_INET; 
    // 2.4 给定ip地址，任意ip地址，并且小端转大端
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
        // 2.4 给定ip地址，也可以指定地址，并将字符串转成32位ipv4
        // inet_addr()函数定义在<arpa/inet.h>中
        //servAddr.sin_addr.s_addr = inet_addr("192.168.2.231"); 
    // 2.5 指定通信端口，并且小端转大端
    servAddr.sin_port = htons(PORT); 

// 3. 地址结构体和socket绑定 -- bind()定义在<sys/socket.h>
    if((ret = bind(listenfd, (sockaddr*)&servAddr, sizeof(servAddr))) != 0){
        perror("bind");
        close(listenfd); // 关闭socket -- 该函数在<unistd.h>中
        return -1;
    }

// 4. 设置监听模式
    if(listen(listenfd, 3) != 0){
        perror("listen");
        close(listenfd); 
        return -1;
    }

// 5. 接受客户端连接
    // 5.1 准备客户端的socket
    int clientfd; // 负责后续和用户通信
    sockaddr_in cliAddr;
    int sAddrLen = sizeof(sockaddr_in);
    // 5.2 接受客户端的连接
    clientfd = accept(listenfd, (sockaddr*)&cliAddr, (socklen_t*)&sAddrLen);
        // 不关心客户端地址，也可以填空
        // clientfd = accept(listenfd, NULL, NULL);
    // 5.3 显示信息
    std::cout << "$ client[" << inet_ntoa(cliAddr.sin_addr) << "]已连接\n";

// 6. 通信
    char buf[1024];
    while(1){
        int iret;
        memset(buf, 0, sizeof(buf));
        // 收消息
        if((iret = recv(clientfd, buf, sizeof(buf), 0)) <= 0){
            std::cout << "iret = " << iret <<std::endl;
            break;
        }
        std::cout << BLUE <<"$[client] Msg_In  --> [" <<buf<<"]\n";

        // 发消息
        strcpy(buf, "ok!");
        if((iret = send(clientfd, buf, strlen(buf), 0)) <= 0){
            perror("send");
            break;
        }
        std::cout << RESET <<" [server] Msg_out <-- [" << buf <<"]\n";
    }
// 7. 关闭
    close(listenfd);
    close(clientfd);
    return 0;
}