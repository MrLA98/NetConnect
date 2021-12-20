#include "../TcpLib/TcpClassLibs.hpp"

int main(){
    TcpClient client;
    if(client.ConnectToServer("nameserver", 5002) == false){
        return -1;
    }
    char buf[SENTENCE_LENGTH];
    while(1){
        // 发消息
        string input = "input nothing";
        cout << RESET << "[input]==> ";
        cin >> input;
        if(input == "q"){
            break;
        }
        strcpy(buf,input.c_str()); 
        if(client.Send_Msg(buf) == false){
            break;
        }
        cout <<RESET<< "[sent]!\n";

        // 收消息
        memset(buf, 0, SENTENCE_LENGTH);
        if(client.Recv_Msg(buf) == false){
            break;
        }
        cout << RESET << "[You have a new message]:\n";
        cout << GREEN << buf << endl;
    }
}