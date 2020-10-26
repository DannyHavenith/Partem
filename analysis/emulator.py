#!/usr/bin/python3

""" Simple emulator for Skywatcher/Synscan telescope control

    This script listens to a serial port or to upd port 11880 and 
    provides just enough interaction to keep communications with 
    the SynScan app going.
"""
import socket
import serial


UDP_IP = "0.0.0.0"
UDP_PORT = 11880



# some pre-defined answers to commands. The following table has a per-channel 
# response for each command, e.g. the respons to command ':a1' is found in values['a'][0]
# and the response for ':j2' can be found in values['j'][1], etc.
values = {
        'a' : ['005046', '00D345'],
        'b' : ['958A00'],
        'e' : ['02071A', '02071A'],
        'E' : ['',''],
        'g' : ['10','10'],
        's' : ['006400', '006400'],
        'j' : ['726682', 'CD2A91'],
        'f' : ['301', '301'],
        'F' : ['',''],
        'G' : ['',''],
        'I' : ['',''],
        'J' : ['',''],
        'K' : ['',''],
        'O' : ['',''],
        'P' : ['',''],
    }


print( "listening on address {}:{}".format( UDP_IP,UDP_PORT))

def SendReply( stream, reply, type = '='):
    message = "{}{}\r".format( type, reply)
    print( "sending reply: {}".format( message))
    stream.write( message)
    
    
def ShowVersion( message, stream):
    SendReply( stream, "02071A")
    
class UdpChannel:
    def __init__(self):
        self.sock = socket.socket(
                socket.AF_INET, # Internet
                socket.SOCK_DGRAM) # UDP
        self.sock.bind((UDP_IP, UDP_PORT))
        
    def read(self):
        data, self.lastAddress = self.sock.recvfrom(1024)
        return data.decode('ascii')
    
    def write(self, message):
        print( "Sending message '{}' to address {}".format(message, self.lastAddress))
        self.sock.sendto( message.encode('ascii'), self.lastAddress)
        
class SerialChannel:
    def __init__(self, portName):
        self.serial = serial.Serial( portName, 9600)
    
    def read(self):
        
        c = self.readChar()
        while c != ':':
            c = self.readChar()
        
        while c != '\r':
            if c == ':':
                message = c
            else:
                message += c
            c = self.readChar()
        
        message += c
        
        return message

    def write(self, message):
        self.serial.write( message.encode('ascii'))
        
    def readChar(self):
        return self.serial.read().decode( 'ascii')
        

handlers = {
    }

#stream = UdpChannel()
stream = SerialChannel( '/dev/ttyUSB0')
while True:
    message = stream.read()
    command = message[1:2]
    print("received message: {} {}".format( command, message))
    try:
        handlers[command]( message, stream)
    except:
        try:
            channel = int( message[2:3])
            SendReply( stream, values[command][channel-1])
        except:
            SendReply( stream, '0', '!')
    