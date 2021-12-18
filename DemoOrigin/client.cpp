#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define BLUE "\033[34m"
#define RESET "\033[0m"
#define PORT 5002

int main(int args, char** argv){
// 1. 创建socket
    int sockfd; // 负责和客户端通信
    if((sockfd = socket(AF_INET, SOCK_STREAM , 0)) == -1){
        perror("socket");
        return -1;
    }
// 2. 向服务器发送链接请求
    // 2.1 创建地址结构体
    sockaddr_in servAddr;
    // 2.2 初始化地址结构体
    memset(&servAddr, 0, sizeof(servAddr));
    // 2.3 解析域名
        // 2.3.1 创建host entry结构体 -- hostent在<unistd.h>中
    hostent* h;
        // 2.3.2 从输入中得到域名 -- gethostbyname()在<unistd.h>中
    //char ipAddr[] = "nameserver";
    //char ipAddr[] = "192.168.2.231";
    if((h = gethostbyname(argv[1])) == 0){
        std::cout << "gethostbyname faild!\n";
        close(sockfd);
        return -1;
    }
        // 2.3.3 复制地址到地址结构体中
    memcpy(&servAddr.sin_addr, h->h_addr, h->h_length);
    // 2.4 设置地址结构体其他参数
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
// 3. 和服务器连接
    if(connect(sockfd, (sockaddr*)&servAddr, sizeof(servAddr)) != 0){
        perror("connect");
        close(sockfd); 
        return -1;
    }
// 4. 通信
    char buf[1024];
    for(int i = 0; i < 10; ++i){
        int iret;
        memset(buf, 0, sizeof(buf));
        // 发消息
        sprintf(buf, "this's the message number %010d", i+1);
        if((iret = send(sockfd, buf, strlen(buf), 0)) <= 0){
            perror("send");
            break;
        }
        std::cout << BLUE << " [client] Msg_out <-- [" << buf << "]\n";

        // 收消息
        memset(buf, 0, sizeof(buf));
        if((iret = recv(sockfd, buf, sizeof(buf), 0)) <= 0){
            std::cout<< "iret = " << iret <<std::endl;
            break;
        }
        std::cout << RESET << "#[server] Msg_in  --> [" << buf << "]\n";
        sleep(2);
    }
// 5. 关闭
    close(sockfd);
    return 0;
}