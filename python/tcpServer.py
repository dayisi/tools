#!/usr/bin/env python
from tcpSock import TcpSock
import socket

class TcpServer:
    sock = None
    connectedClients = []
    host = None
    port = None
    maxNum = 0

    def __init__(self, host = '127.0.0.1', port=4009):
        self.host = host
        self.port = port
        self.sock = socket.socket()
        self.sock.bind((host, port))
    
    def __del__(self):
        for client in self.connectedClients:
            try:
                self.connectedClients.remove(client)
                del client
            except ... as e:
                print(e)
        self.sock.close()

                
    # return true or false
    def filterClients(self):
        for client in self.connectedClients:
            try:
                client.sendNum(0)
            except socket.error as e:
                print(e)
                self.connectedClients.remove(client)
                del client
        return len(self.connectedClients) < self.maxNum


    def waitForClients(self, maxNum=1):
        self.maxNum = maxNum
        self.sock.listen(maxNum)
        print('listening...')
        while len(self.connectedClients) <= maxNum:
            if len(self.connectedClients) == maxNum:
                print('connected clients reach limit, filter disconnected client first!')
                if not self.filterClients():
                    break
            clientSock, addr = self.sock.accept()
            print('one client connected')
            self.connectedClients.append(TcpSock(clientSock))
    
    # return true or false
    def sendNum(self, num, index = -99):
        res = True
        if index == -99:
            for i in range(len(self.connectedClients)):
                if not self.connectedClients[i].sendNum(num):
                    print('failed to send num to client ' + str(i))
                    res = False
        else:
            try:
                return self.connectedClients[index].sendNum(num)
            except ValueError.error as e:
                print('failed to send num')
                print(e)
                res = False
        return res
    
    # return true or false
    def sendString(self, msg, index = -99):
        res = True
        if index == -99:
            for i in range(len(self.connectedClients)):
                if not self.connectedClients[i].sendString(msg):
                    print('failed to send msg to client ' + str(i))
                    res = False
        else:
            try:
                return self.connectedClients[index].sendString(msg)
            except ValueError.error as e:
                print('failed to send string')
                print(e)
                res = False
        return res

    # return received num
    def recvNum(self, index=0):
        try:
            return self.connectedClients[index].recvNum()
        except ValueError.error as e:
            print('failed to recv num')
            print(e)
        return None

    # return received string
    def recvString(self, index=0):
        try:
            return self.connectedClients[index].recvString()
        except ValueError.error as e:
            print('failed to receive string')
            print(e)
        return None

'''
def main():
    server = TcpServer()
    server.waitForClients(1)
    print('client connected')
    if server.sendString('hello, client'):
        msg = server.recvString()
        if not msg is None:
            print(msg)
    if server.sendNum(1):
        num = server.recvNum()
        if not num is None:
            print(num)
    del server
    return True

if __name__ == '__main__':
    main()
'''