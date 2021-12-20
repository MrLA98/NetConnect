#include "../TcpLib/TcpClassLibs.hpp"

int main(){
    // 屏蔽子进程退出信号，防止产生僵尸进程
    signal(SIGCHLD, SIG_IGN); 
    // 创建服务端类
    TcpServer server;
    // 初始化
    if(server.Init(5002) == false){
        return -1;
    }
    // 循环产生多进程
    while(1){
        // 接收连接请求
        if(server.Accept() == false){
            continue;
        }
        // 大于0表示父进程，等于0表示子进程
        if(fork() > 0){ 
            server.closeConnect(); // 父进程关掉新建连接
            continue;
        }
        // 到这说明已经到子进程了
        server.closeListen(); // 子进程关闭监听
        char buf[SENTENCE_LENGTH];
        while(true){
            // 收消息
            memset(buf, 0, sizeof(buf));
            if(server.Recv_Msg(buf) == false){
                break;
            }
            cout << RESET <<"[You have a new message!]:\n";
            cout << GREEN << buf <<endl;

            // 发消息
            strcat(buf, "[okok!]");
            cout << RESET << "[sent]:" << buf << endl;
            if(server.Send_Msg(buf) == false){
                break;
            }
        }
        cout << "[server exit!]\n";
        exit(0); // 子进程退出
    }
    return 0;
}