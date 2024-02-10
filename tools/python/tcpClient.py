#!/usr/bin/env python
from tcpSock import TcpSock
import socket

class TcpClient:
    sock = None
    host = None
    port = None
    tempSock = None

    def __init__(self):
        self.tempSock = socket.socket()
        self.sock = TcpSock(self.tempSock)

    def __del__(self):
        del self.sock

    def connectServer(self,host='127.0.0.1', port=4009):
        self.host = host
        self.port = port
        self.sock.sock.connect((host, port))

    # return true or false
    def sendNum(self, num):
        return self.sock.sendNum(num)

    # return true or false
    def sendString(self, msg):
        return self.sock.sendString(msg)

    # return received num
    def recvNum(self):
        return self.sock.recvNum()

    #return received string
    def recvString(self):
        return self.sock.recvString()

'''
def main():
    client = TcpClient()
    client.connectServer()
    print('server is connected')
    msg = client.recvString()
    if not msg is None:
        print(client.sendString('hello, server'))
    num = client.recvNum()
    if not num is None:
        print(client.sendNum(1))
    del client
    return True

if __name__ == '__main__':
    main()
'''