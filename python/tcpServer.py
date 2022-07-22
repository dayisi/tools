#!/usr/bin/env python
from asyncio.streams import _ClientConnectedCallback
from tcpSock import TcpSock
import socket

class TcpServer:
    sock = None
    connectedClients = []
    host = '127.0.0.1'
    port = 4009
    maxNum = 0

    def __init__(self, host = '127.0.0.1', port=4009):
        self.host = host
        self.port = port
        self.sock = socket.socket()
        self.sock.bind((host, port))

    def filterClients(self):
        for client in self.connectedClients:
            try:
                client.sendNum(0)
            except socket.error as e:
                print(e)
                self.connectedClients.remove(client)
                del client
        return len(self.connectedClients) < self.maxNum


    def waitForClients(self, maxNum):
        self.maxNum = maxNum
        self.sock.listen(maxNum)
        while len(self.connectedClients) <= maxNum:
            clientSock, addr = self.sock.accept()
            if len(self.connectedClients) == maxNum:
                if self.filterClients():
                    self.connectedClients.append(TcpSock(clientSock))
    
    '''
        @param index: 
    '''
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

    def recvNum(self, index):
        try:
            return self.connectedClients[index].recvNum()
        except ValueError.error as e:
            print('failed to recv num')
            print(e)
        return None

    def recvString(self, index):
        try:
            return self.connectedClients[index].recvString()
        except ValueError.error as e:
            print('failed to receive string')
            print(e)
        return None