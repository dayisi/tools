#ifndef COMMON_TCPPROTOCOL_H
#define COMMON_TCPPROTOCOL_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "errhandle.h"

typedef struct myData_
{
    int msgType;
    std::string msg;
}myData;

class Protocol
{
    private:
    int _port;
    int _listenNum;
    const int _sizeOfIntInBytes = 4;
    const int _maxPayloadSize = 1024 * 4;
    const int _packetLimitSize = 65535;

    bool _trulySend(int fd, char* buf, int msgSize, int offset = 0);
    bool _sendNum(int num);
    bool _sendString(const std::string &str);
    void _recvNum(int& num);
    bool _recvString(int len, std::string &str);
    bool _myConnect(int skfd, char* hostIP, int port);

    std::mutex sendLock; // Prevent simultaneous send
    std::mutex recvLock; // Prevent simultaneous recv
    
    public:
    int mySocket;

    Protocol();
    Protocol(int port, int listenNum);
    ~Protocol();
    void myListen();
    int myAccept();
    void myConnect(char* hostIP, int port);
    bool mySend(int msgType, const std::string &msg);
    bool mySend(int msgType);
    myData myRecv(bool blocking = true);
    bool myClose();
};

#endif
