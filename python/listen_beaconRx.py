#!/usr/bin/env python

import serial
import signal
import sys
import time
import datetime

RSSI_OFFSET = 74;

# If True, hub will poll nodes for status
TRIGGER_STATUS = False

# Open up COM4 as Slave
slave = serial.Serial()
#slave.port = "/dev/ttyAMA0"  # RPi test header
slave.port = "/dev/ttyUSB0"  # RPi USB Connector
#slave.port = "COM8"           # Windows Machine
#slave.port = "/dev/ttyACM0"
slave.baudrate = 9600
slave.timeout = 1


class PacketStruct():
  def __init__(self):
    self.src_address  = 0
    self.src_rssi     = 0
    self.src_lqi      = 0
    self.des_address  = 0
    self.des_rssi     = 0
    self.node_temp    = 0 
    self.command      = 0
    self.status       = 0
    self.counter      = 0
    self.door1        = 0
    self.door2        = 0
    self.msg          = 0 


def ProcessPacket(b,a):
  #temperature   = (1.0*(ord(a[4])*2**24 + ord(a[5])*2**16 + ord(a[6])*2**8 + ord(a[7])))/10;
  #b.src_address = ord(a[0])
  #b.src_rssi    = -1*ord(a[1])  # dBm
  #b.des_address = ord(a[2])
  #b.des_rssi    = -1*ord(a[3]) # dBm
  #b.node_temp   = temperature  # F
  #b.command     = ord(a[8])
  #b.status      = ord(a[9])

  #print 
  #print "Raw Message            = " + str(a)
  #print "Length of Raw Message  = " + str(len(a))

  #for i in range(0,len(a)):
  #  print "a[" + str(i) + "] = " + str(ord(a[i]))


  rssi = int(a[0:2],16);
  #print "Raw RSSI Value         = " + str(rssi)
  # convert to dBm
  if(rssi < 128):
    rssi = rssi/2-RSSI_OFFSET;
  else:
    rssi = (rssi-256)/2-RSSI_OFFSET;


  b.src_rssi    = rssi;  # dBm
  b.src_lqi     = int(a[2:4],16);
  b.counter     = int(a[4:12],16);
  b.door1       = int(a[12:14],16); 
  b.door2       = int(a[14:16],16); 
  b.msg         = a[20:len(a)] 

  temp          = int(a[16:20],16);
  b.node_temp   = temp*0.18+32

  n = chr(int(a[12:14],16))
  for i in range(1,26):
    n = n + chr(int(a[2*i+12:2*i+14],16))

  #print "ASCII Message          = " + n
  #print " "
  #print " "

  return b
  


def signal_handler(signal, frame):
  print 'You Pressed Ctrl-C!'
  slave.close()
  sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)
print 'Press CTRL-C to exit'
print ' '
print ' '


# open serial port
slave.open()

iteration = 0;
node = 0;
packet =  PacketStruct()

while(True):

  a = slave.readline()
  if (len(a) > 10):

    #------------------------------------
    # Process Packet
    #------------------------------------
    packet = ProcessPacket(packet,a)
    

    #------------------------------------
    # get timestamp
    #------------------------------------
    ts = time.time()
    st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
    #print "--------------------------------------------------------------"
    print ">>>"
    print "Count = " + str(packet.counter)
    print "Door 1 = " + str(packet.door1)
    print "Door 2 = " + str(packet.door2)
    print "RSSI: " + str(packet.src_rssi) + "dBm, LQI: " + str(packet.src_lqi) + ", Time: " + str(st)
    print "Far End Temperature: " + str(packet.node_temp) + " F"
    print "Message: " + str(packet.msg)
    print "<<<"

    #------------------------------------
    # Print data to terminal
    #------------------------------------
    #print "The Number of Bytes in Received Packet: " + str(len(a))
    #print " "
    #print "Address of Sensor (Src): " + str(hex(packet.src_address)) + ", RSSI: " + str(packet.src_rssi) + "dBm"
    #print "Address of Hub (Des): " + str(hex(packet.des_address)) + ", RSSI: " + str(packet.des_rssi) + "dBm"
    #print "Temperature of Node: " + str(packet.node_temp) + "F"
    #print "Command: " + str(hex(packet.command))
    #print "Status: " + str(hex(packet.status))
    #print "Received Message: " + str(packet.msg)
    #print " "
    #print " "
    #print " "
    #print "   Receiver RSSI    = " + str(packet.src_rssi) + "dBm"
    #print "   Receiver LQI     = " + str(packet.src_lqi) 
    #print "   Counter (32-bit) = " + str(packet.counter) 
    #print "   Message          = " + str(packet.msg) 
    #print "--------------------------------------------------------------"
    #print " "
    #print " "
    #print " "


  #------------------------------------
  # Now select a different node
  #------------------------------------
  if(TRIGGER_STATUS == True):
    #------------------------------------
    # Sleep for a second
    #------------------------------------
    time.sleep(1);

    if(node == 0):
      node = 1;
      command = bytearray([0xA1,0x10]);
    elif(node == 1):
      node = 2;
      command = bytearray([0xA1,0x20]);
    elif(node == 2):
      node = 3;
      command = bytearray([0xA1,0x30]);
    elif(node == 4):
      node = 5;
      command = bytearray([0xA1,0x40]);
    elif(node == 5):
      node = 0;
      command = bytearray([0xA1,0x50]);

    #------------------------------------
    # Increment the counter
    #------------------------------------
    iteration = iteration + 1;

