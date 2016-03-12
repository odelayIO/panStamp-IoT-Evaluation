/*
 * modem.pde
 *
 * Copyright (c) 2014 panStamp <contact@panstamp.com>
 * 
 * This file is part of the panStamp project.
 * 
 * panStamp  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * panStamp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with panStamp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Daniel Berenguer
 * Creation date: 15/02/2011
 *
 * Device:
 * Serial gateway or modem
 *
 * Description:
 * This is not a proper SWAP gateway but a simple transparent UART-RF
 * interface. This device can be used from a serial console as hyperterminal
 * or minicom. Wireless packets are passed to/from the host computer in ASCII
 * format whilst configuration is done via AT commands.
 *
 * Visit our wiki for details about the protocol in case you want to develop
 * your own PC library for this device.
 */

#include "Arduino.h"
#include "HardwareSerial.h"

#define SERIAL_BUF_LEN           128     // Maximum length for any serial string
int DOOR_SENSOR_E = 0;
int DOOR_SENSOR_A = 1;
uint32_t counter  = 0;
uint8_t  d        = 0x41;

boolean packetAvailable = false;
CCPACKET *rxPacket;
char strSerial[SERIAL_BUF_LEN];          // Serial buffer
byte ch;
int len = 0;

byte charToHex(byte ch);

/**
 * This function is called whenever a wireless packet is received
 */
void rfPacketReceived(CCPACKET *packet)
{
  if (packet->length > 5)
  {
    rxPacket = packet;
    packetAvailable = true;
  }
}


/**
 * setup
 *
 * Arduino setup function
 */
void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
 
  //setting put pins for doors
  pinMode(DOOR_SENSOR_E, OUTPUT);
  pinMode(DOOR_SENSOR_A, OUTPUT);
  digitalWrite(DOOR_SENSOR_E, HIGH);
  digitalWrite(DOOR_SENSOR_A, HIGH);

  // Reset serial buffer
  memset(strSerial, 0, sizeof(strSerial));

  Serial.begin(9600);
  Serial.flush();
  Serial.println("");
  
  // Default mode is COMMAND 
  Serial.println("Modem ready!");

  // Disable address check from the RF IC
  panstamp.radio.disableAddressCheck();

  // Declare RF callback function
  panstamp.attachInterrupt(rfPacketReceived);
  
  digitalWrite(LED, LOW);
}

/**
 * loop
 *
 * Arduino main loop
 */
void loop()
{
  // Read wireless packet?
  if (packetAvailable)
  {
    digitalWrite(LED, HIGH);
    // Disable wireless reception
    panstamp.rxOff();

    byte i; 
    packetAvailable = false;

    //Serial.print("(");
    if (rxPacket->rssi < 0x10)
      Serial.print("0");
    Serial.print(rxPacket->rssi, HEX);
    if (rxPacket->lqi < 0x10)
      Serial.print("0");
    Serial.print(rxPacket->lqi, HEX);
    //Serial.print(")");
    for(i=0 ; i<rxPacket->length ; i++) 
    {
      if (rxPacket->data[i] < 0x10)
        Serial.print(0, HEX);    // Leading zero
      Serial.print(rxPacket->data[i], HEX);
    }
    Serial.println("");

    // setting pins to match Tx side in garage
    if(rxPacket->data[4] == 0) 
    {
      digitalWrite(DOOR_SENSOR_E, LOW);
    }
    else
    {
      digitalWrite(DOOR_SENSOR_E, HIGH);
    }
    if(rxPacket->data[5] == 0)
    {
      digitalWrite(DOOR_SENSOR_A, LOW);
    }
    else
    {
      digitalWrite(DOOR_SENSOR_A, HIGH);
    }
  }

  // Enable wireless reception
  panstamp.rxOn();
  digitalWrite(LED, LOW);
}

/**
 * charToHex
 *
 * 'ch' Character to be converted to hexadecimal
 *
 * Returns:
 *  Hex value
 */
byte charToHex(byte ch)
{
  byte val;
  
  if (ch >= 'A' && ch <= 'F')
    val = ch - 55;
  else if (ch >= 'a' && ch <= 'f')
    val = ch - 87;
  else if (ch >= '0' && ch <= '9')
    val = ch - 48;
  else
    val = 0x00;

  return val;
}

