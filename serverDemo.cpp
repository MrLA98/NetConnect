#include "TcpLib/TcpClassLibs.hpp"

int main(){
    TcpServer myServer;
    myServer.Init(5002);
    myServer.Accept();
    cout << "---------- One Message Once ----------"<<endl;
    cout << "[waiting message...]"<<endl;
    sleep(2);
    char buf[SENTENCE_LENGTH];
    bool ret = true;
    while(1){
        // 收消息
        memset(buf, 0, SENTENCE_LENGTH);
        ret = myServer.Recv_Msg(buf);
        if(ret == false){
            cout << "# Recv Msg Error!" << endl;
            break;
        }
        cout << RESET << "[You have a message]:\n";
        cout << BLUE << buf << endl;
        // 发消息
        string input = "input nothing!"; 
        cout << GREEN <<"[input]==> ";
        cin >> input;
        if(input == "q"){
            break;
        }
        strcpy(buf,input.c_str()); 
        ret = myServer.Send_Msg(buf);
        if(ret == false){
            cout << RESET <<"# Send Msg Error!" << endl;
        }
        cout << RESET << "[sent!]" << endl;  
   }
   cout << RESET << "---------- Chat Ends -----------" << endl;
   return 0;
}