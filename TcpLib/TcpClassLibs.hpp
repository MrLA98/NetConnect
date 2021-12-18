#include "TcpHead.h"

// ---------------------- 防止没有完成收发 --------------------- //
// 发消息
bool Send_func(const int sockfd, const char* buf, const size_t n){
    // 没发出的、 单次发出的、 当前下标
    int rest = n, sent_once = 0, index = 0;
    // 确保全部发出
    while(rest > 0){
        sent_once = send(sockfd, buf + index, rest, 0);
        index += sent_once;
        rest -= sent_once;
    }
    return true;
}

// 收消息
bool Recv_func(const int sockfd, char* buf, const size_t n){
    // 没收到的、 单次收到的、 当前下标
    int rest = n, got_once = 0, index = 0;
    // 确保全部发出
    while(rest > 0){
        got_once = recv(sockfd, buf + index, rest, 0);
        index += got_once;
        rest -= got_once;
    }
    return true;
}

// ---------------------- 解决粘包分包问题 --------------------- //
// 发消息
bool TcpSend(const int sockfd, const char* buf, const int ibuflen = 0){
    // 区字符串和二进制流
    int litLen = ibuflen == 0 ? strlen(buf) : ibuflen;
    int bigLen = htonl(litLen); // 网络字节序

    // 设置字符串
    char strTBuf[4 + litLen]; // 数字长度，内容长度
    memset(strTBuf, 0, sizeof(strTBuf)); // 初始化
    memcpy(strTBuf, &bigLen, 4); // 把大端化的数字放进去
    memcpy(strTBuf + 4, buf, litLen); // 把内容放到后移四位的位置
    
    // 交给“写函数”
    bool ret = Send_func(sockfd, strTBuf, litLen + 4);
    return ret;
}

// 收消息
bool TcpRecv(const int sockfd, char* buf, int * ibuflen){
    // 读长度
    (*ibuflen) = 0;
    bool ret = Recv_func(sockfd, (char*)ibuflen, 4);
    if(ret == false) {
        cout << "#[error]: get data length error"<<endl;
        return false;
    }
    (*ibuflen) = ntohl(*ibuflen); // 内容大端转小端
    
    // 读内容
    ret = Recv_func(sockfd, buf, (*ibuflen));
    return ret;
}

// -------------------------- Client ------------------------- //

class TcpClient
{
private:
    int m_toServer; // 用于和服务端通信的
    sockaddr_in m_serv_addr; // 目标服务端地址
public:
    TcpClient(){
        m_toServer = -1;
    }

    // 连接服务端
    bool ConnectToServer(const char *ip, const int port){
        // 已经初始化过了
        if(m_toServer > 0){
            close(m_toServer);
            m_toServer = -1;
        }

        // 获得socket, ipv4, tcp协议
        m_toServer = socket(AF_INET, SOCK_STREAM, 0);
        if(m_toServer < 0){
            perror("socket");
            return false;
        }

        // 初始化地址结构体
        memset(&m_serv_addr, 0, sizeof(m_serv_addr)); // 初始化
        m_serv_addr.sin_family = AF_INET;
        m_serv_addr.sin_port = htons(port);

        // 解析域名并放入结构体中
        hostent *h = gethostbyname(ip);
        if(h == 0){
            std::cout << "gethostbyname faild!\n";
            close(m_toServer);
            return false;
        }
        memcpy(&m_serv_addr.sin_addr, h->h_addr, h->h_length);

        // 和服务端连接
        int ret = connect(m_toServer, (sockaddr*)&m_serv_addr, sizeof(m_serv_addr));
        if(ret != 0){
            perror("connect");
            close(m_toServer);
            return false;
        }
        cout << "# connect successfully! ip :[" << ip << "] -port[" << port <<"]" << endl;
        return true;
    }

    // 接收消息
    bool Recv_Msg(char* buf){
        if(m_toServer < 0){
            cout << "#[error]: no connection!" << endl;
            return false;
        }
        int buflen = 0;
        bool ret = TcpRecv(m_toServer, buf, &buflen);
        if(ret == false){
            cout << "#[error]: failed to recieve!"<<endl;
        }
        return ret;
    }

    // 发送消息
    bool Send_Msg(const char* buf){
        if(m_toServer < 0){
            cout << "#[error]: no connection!" <<endl;
            return false;
        }
        bool ret = TcpSend(m_toServer, buf);
        if(ret == false){
            cout << "#[error]: failed to recieve!" <<endl;
            return false;
        }
        return true;
    }

    ~TcpClient(){
        if(m_toServer > 0){
            close(m_toServer);
        }
        cout << "# connection closed!"<<endl;
    }
};

// -------------------------- Server ------------------------- //

class TcpServer
{
private:
    int m_listenfd; // 用于监听的socket
    int m_toClientfd; // 用于和用户通信的socket
    sockaddr_in m_serv_addr; // 服务器地址
    sockaddr_in m_cli_addr; // 用户地址
public:
    TcpServer(){
        m_listenfd = -1;
        m_toClientfd = -1;
    }

    // 初始化服务端，并设置为监听模式
    bool Init(const unsigned int port){
        int ret;

        // 已经初始化过了
        if(m_listenfd > 0){ 
            close(m_listenfd);
            m_listenfd = -1;
        }

        // 新建socket -- ipv4流对象
        m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if(m_listenfd == -1){
            perror("socket");
            return false;
        }
        
        // 设置端口占用
        int opt = 1;
        ret = setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));
        if(ret != 0){
            cout << "#[error]: setsockopt -- SO_REUSEADDR"<<endl;
            return false;
        }

        // 设置监听socket地址
        memset(&m_serv_addr, 0, sizeof(m_serv_addr)); // 初始化
        m_serv_addr.sin_family = AF_INET; // ipv4
        m_serv_addr.sin_port = htons(port); // 指定端口
        m_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意网卡

        // 和监听socket绑定
        ret = bind(m_listenfd, (sockaddr*)&m_serv_addr, sizeof(m_serv_addr));
        if(ret != 0){
            perror("bind");
            closeListen();// 关闭m_listenfd
            return false;
        }
        
        // 设置监听模式
        ret = listen(m_listenfd, LISTEN_QUEUE);
        if(ret != 0){
            perror("listen");
            closeListen();// 关闭m_listenfd
            return false;
        }
        cout << "# listening mode" << endl;
        return true;
    }

    // 接收连接
    bool Accept(){
        if(m_listenfd == -1){
            cout << "#[error]: listen does not start!" << endl;
            return false;
        }
        int len = sizeof(sockaddr_in);
        m_toClientfd = accept(m_listenfd, (sockaddr*)&m_cli_addr, (socklen_t*)&len);
        if(m_toClientfd < 0){
            perror("accept");
            return false;
        }
        cout << "# connet to client[" << inet_ntoa(m_cli_addr.sin_addr) << "]"<<endl;
        return true;
    }

    // 获得连接得到的IP
    char* GetIP(){
        if(m_toClientfd < 0){
            throw std::runtime_error("no connected clients!");
        }
        return inet_ntoa(m_cli_addr.sin_addr);
    }

    // 接收消息
    bool Recv_Msg(char* buf){
        if(m_toClientfd < 0){
            cout << "#[error]: no connection!" << endl;
            return false;
        }
        int buflen = 0;
        bool ret = TcpRecv(m_toClientfd, buf, &buflen);
        if(ret == false){
            cout << "#[error]: failed to recieve!"<<endl;
        }
        return ret;
    }

    // 发送消息
    bool Send_Msg(const char* buf){
        if(m_toClientfd < 0){
            cout << "#[error]: no connection!" <<endl;
            return false;
        }
        bool ret = TcpSend(m_toClientfd, buf);
        if(ret == false){
            cout << "#[error]: failed to recieve!" <<endl;
            return false;
        }
        return true;
    }


    // 关闭监听socket
    void closeListen(){
        if(m_listenfd > 0){
            close(m_listenfd);
        }
        cout << "# server-listen closed!"<<endl;
    }

    // 关闭客户端链接
    void closeConnect(){
        if(m_toClientfd > 0){
            close(m_toClientfd);
        }
        cout << "# conection closed!" << endl;
    }

    ~TcpServer(){
        closeListen();
        closeConnect();
    }
};


