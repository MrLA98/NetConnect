#ifndef TCP_HEAD_FILE
#define TCP_HEAD_FILE

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::cin;
using std::string;

#define LISTEN_QUEUE 5
#define SENTENCE_LENGTH 1024
#define BLUE    "\033[34m"
#define GREEN   "\033[32m"
#define RESET   "\033[0m"

#endif