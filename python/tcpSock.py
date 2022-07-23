#/usr/bin/env python
import struct, socket

class TcpSock:
    sock = None

    def __init__(self, sock):
        self.sock = sock
    
    def __del__(self):
        self.sock.close()

    #return received num
    def recvNum(self):
        num = None
        try:
            num = self.sock.recv(4)
            num, = struct.unpack('>l', num)
        except socket.error as e:
            print('failed to receive num')
            print(e)
            return None
        return num

    #return received string
    def recvString(self):
        payload = self.recvNum()
        if payload is None:
            return None
        msg = None
        try:
            msg = self.sock.recv(payload)
            msg = msg.decode()
        except socket.error as e:
            print('failed to receive string')
            print(e)
            return None
        return msg

    #return True or False
    def sendNum(self, num):
        buf = struct.pack('>l', num)
        try:
            self.sock.send(buf)
        except socket.error as e:
            print('failed to send num')
            print(e)
            return False
        return True

    #return True or False
    def sendString(self, msg):
        payload = len(msg)
        if not self.sendNum(payload):
            return False
        buf = msg.encode()
        try:
            self.sock.send(buf)
        except socket.error as e:
            print('failed to send string')
            print(e)
            return False
        return True
    
    def closeSock(self):
        try:
            self.sock.close()
        except ... as e:
            print(e)