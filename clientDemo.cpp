#include "TcpLib/TcpClassLibs.hpp"

int main(){
    TcpClient myClient;
    bool ret = myClient.ConnectToServer("nameserver", 5002);
    if(ret == false){
        return -1;
    }
    cout << "---------- One Message Once ----------"<<endl;
    cout << "[chat now ~]" << endl;
    char buf[SENTENCE_LENGTH];
    while(1){
        // 发消息
        string input = "input nothing!"; 
        cout << BLUE <<"[input]==> ";
        cin >> input;
        strcpy(buf,input.c_str()); 
        ret = myClient.Send_Msg(buf);
        if(ret == false){
            cout << RESET <<"# Send Msg Error!" << endl;
        }
        cout << RESET << "[sent!]" << endl; 
        // 收消息
        memset(buf, 0, SENTENCE_LENGTH);
        ret = myClient.Recv_Msg(buf);
        if(ret == false){
            cout << RESET << "# Recv Msg Error!" << endl;
            break;
        }
        cout << RESET << "[You have a message]:\n";
        cout << GREEN << buf << endl;
   }
   cout << RESET << "---------- Chat Ends -----------" << endl;
   return 0;
}