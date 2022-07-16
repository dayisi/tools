#include "common/tcpprotocol.h"


//for server socket
Protocol::Protocol(int port, int listenNum)
{
    _port = port;
    _listenNum = listenNum;
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket < 0)
    {
        printErrorMsg("tcp socket create failed");
        exit(0);
    }

    int option = 1;
    if(setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0)
    {
        printErrorMsg("set tcp sockopt failed");
        myClose();
        exit(1);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);

    int ret = bind(mySocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret == -1)
    {
        //std::cout<<"failed to bind server socket: "<<std::strerror(errno)<<std::endl;
        printErrorMsg("failed to bind server socket");
        myClose();
        exit(1);
    }	
};

// for client socket
Protocol::Protocol()
{
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket < 0)
    {
        printErrorMsg("tcp socket create failed");
        exit(0);
    }
    int option = 1;
    if(setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0)
    {
        printErrorMsg("set tcp sockopt failed");
        myClose();
        exit(1);
    }
};

Protocol::~Protocol(){
    myClose();	
}


void Protocol::myListen()
{
    std::cout<<"listen...\n";
    while(listen(mySocket, _listenNum) == -1)
    {
        std::cout<<"failed, try again...\n";
        sleep(3);
    }
}

int Protocol::myAccept()
{
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);
    int addrLen = sizeof(serverAddr);
    std::cout<<"wait one client to connect...\n";
    int clientSkt = -1;
    clientSkt = accept(mySocket, (struct sockaddr*) &serverAddr, (socklen_t*) &addrLen);
    while(clientSkt < 0) // -1 returned if accept() fails
    {
        std::cout<<"failed to accept, try again...\n";
        sleep(1);
        clientSkt = accept(mySocket, (struct sockaddr*) &serverAddr, (socklen_t*) &addrLen);
    }
    return clientSkt;
}



bool Protocol::_myConnect(int skfd, char* hostIP, int port)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if(inet_pton(AF_INET, hostIP, &serverAddr.sin_addr) <=0)
    {
        printErrorMsg("invalid address/address not supported");
        return false;
    }
    
    if(connect(skfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        printErrorMsg("failed to connect server");
        return false;
    }
    return true;
}

void Protocol::myConnect(char* hostIP, int port)
{
    std::cout<<"connect to server...\n";
    while(!_myConnect(mySocket, hostIP, port))
    {
        std::cout<<"failed, try again...\n";
        sleep(3);
    }
}

bool Protocol::_trulySend(int fd, char* buf, int msgSize, int offset)
{
    char* startPtr = buf + offset;
    int sentSize = 0;
    while(sentSize != msgSize)
    {
        int sizeToSent = msgSize - sentSize;
        if(sizeToSent > _maxPayloadSize)
        {
            sizeToSent = _maxPayloadSize;	
        }
        int tempSentSize = write(fd, startPtr + sentSize, sizeToSent);
        if(tempSentSize < 0)
        {	
            return false;
        }
        sentSize += tempSentSize;
    }
    if(sentSize > msgSize)
    {
        //printErrorMsg("tcp skt unexpected error: data more than msg send");	
        return false;
    }
    return true;
}

bool Protocol::_sendNum(int num)
{
    int temp = htonl(num);
    int msgSize = _sizeOfIntInBytes;
    char* tempPtr = (char*)&temp;
    return _trulySend(mySocket, tempPtr, msgSize);
}

bool Protocol::_sendString(const std::string &msg)
{
    int msgSize = msg.length();
    char* buf = (char*)malloc(sizeof(char) * msgSize + 1);
    for(int i = 0; i < msgSize; i++)
    {
        buf[i] = msg[i];
    }
    buf[msgSize] = '\0';

    if(! _trulySend(mySocket, buf, msgSize))
    {
        free(buf);
        return false;
    }
    return true;
}

bool Protocol::mySend(int msgType)
{
    return mySend(msgType, "");
}

bool Protocol::mySend(int msgType, const std::string &msg)
{
    std::unique_lock<std::mutex> lk(sendLock);
    //std::cout<<"Client_main send: ["<<msgType<<"] ["<<msg.length()<<"]"<<std::endl;
    int posOfFirstChar = 0;
    int msgSize = msg.length();
    do
    {
        std::string payloadData = msg.substr(posOfFirstChar, _packetLimitSize - _sizeOfIntInBytes);
        int payloadSize = payloadData.length();
        int headerData = payloadSize + _sizeOfIntInBytes;
        if(!_sendNum(headerData))
        {
            //std::cout<<"TCP FUNC ERR: failed to send total msg size: "<<std::strerror(errno)<<std::endl;
            return false;		
        }	
        if(posOfFirstChar + payloadSize < msgSize)
        {
            if(!_sendNum(99))
            {
                //std::cout<<"TCP FUNC ERR: failed to send msg type:"<<std::strerror(errno)<<std::endl;
                return false;
            }
        }
        else
        {
            if(!_sendNum(msgType))
            {
                //std::cout<<"TCP FUNC ERR: failed to send msg type:"<<std::strerror(errno)<<std::endl;
                return false;
            }	
        }
        if(msg.length() == 0)
        {
            return true;
        }
        if(!_sendString(payloadData))
        {
            //std::cout<<"TCP FUNC ERR: send msg failed: "<<std::strerror(errno)<<std::endl;
            return false;
        }
        posOfFirstChar += payloadSize;
    } 
    while(posOfFirstChar < msgSize);
    return true;
}

void Protocol::_recvNum(int& num)
{
    int result = 0;
    char* data = (char*)&result;	
    int msgSize = _sizeOfIntInBytes;  //size of header
    int recvdSize = 0;
    while(recvdSize < msgSize)
    {
        //std::cout<<"recving...\n";
        int tempRecvdSize = read(mySocket, data+recvdSize, msgSize - recvdSize);
        //std::cout<<"recvd data size in byte is: "<<tempRecvdSize;
        if(tempRecvdSize <= 0)
        {
            //printErrorMsg("failed to recv a number msg");
            num = -1;
            return;	
        }	
        recvdSize += tempRecvdSize;
    }
    num = ntohl(result);
}

bool Protocol::_recvString(int len, std::string& str)
{
    int recvdSize = 0;
    while(recvdSize < len)
    {
        int tempRecvdSize = read(mySocket, &(str.at(0)) + recvdSize, len - recvdSize);
        if(tempRecvdSize < 0)
        {
            //printErrorMsg("failed to recv string msg");
              return false;
        }
        recvdSize += tempRecvdSize;
    }
    //std::cout<<"recvString: "<<(*str);
    return true;
}

myData Protocol::myRecv(bool blocking)
{
    //std::cout<<"in myRecv()\n";
    char currentMsgType = 'H';
    int payloadSize = 0;	

    myData result;	
    
    result.msgType = -1;

    while(true)
    {
        //prepare to recv header
        if(currentMsgType == 'H')
        {
//			std::cout<<"recving total data size\n";
            if(!blocking)
            {
                    fcntl(mySocket, F_SETFL, fcntl(mySocket, F_GETFL, 0) | O_NONBLOCK);
            }
            _recvNum(payloadSize);
            if(payloadSize == -1)
            {
//				printErrorMsg("Wrong total data size");
                    break;
            }
            currentMsgType = 'P';
        }
        //prepare to recv payload
        else if(currentMsgType == 'P')
        {
            int tempMsgType = -1;
            fcntl(mySocket, F_SETFL, fcntl(mySocket, F_GETFL, 0) & ~O_NONBLOCK);
            _recvNum(tempMsgType);			
            if(tempMsgType == -1)
            {
                //printErrorMsg("Wrong msg type");
                break;
            }
            result.msgType = tempMsgType;
            result.msg = std::string(payloadSize - _sizeOfIntInBytes + 1, '\0');

            if(!_recvString(payloadSize - _sizeOfIntInBytes, result.msg))
            {
                result.msg = "";
                return result;
            }
            std::string temp = result.msg.substr(0, payloadSize - _sizeOfIntInBytes);
            result.msg = temp;
            //std::cout<<"Client_main recv: ["<<result.msgType<<"] ["<<(*(result.msg))<<"]\n";
            return result;
        }
    }
//	std::cout<<"Client_main recv: failed to recv any msg\n";
    return result;
}

bool Protocol::myClose()
{
    if(close(mySocket) < 0)
    {
        //std::cout<<"failed to close socket: "<<std::strerror(errno)<<std::endl;
        printErrorMsg("failed to close tcp socket");
        return false;
    }
    return true;
}
