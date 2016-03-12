/**
 * Copyright (c) 2014 panStamp <contact@panstamp.com>
 * 
 * This file is part of the panStamp project.
 * 
 * panStamp  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 * 
 * panStamp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with panStamp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Daniel Berenguer
 * Creation date: 07/23/2014
 */

/**
 * Basic beacon example
 *
 * This application sleeps (ultra-low-current consumption) most of the time and
 * wakes-up every five seconds to transmit a packet and then enters the sleep
 * mode again
 */

#include "HardwareSerial.h"
#include "thermistor.h"

#define RFCHANNEL        0       // Let's use channel 0
#define SYNCWORD1        0xB5    // Synchronization word, high byte
#define SYNCWORD0        0x47    // Synchronization word, low byte
#define SOURCE_ADDR      5       // Device address

// Macros
#define powerThermistorOn()   digitalWrite(NTC_POWER_PIN, HIGH)
#define powerThermistorOff()  digitalWrite(NTC_POWER_PIN, LOW)

// Analog pin used to read the NTC
//#define NTC_PIN               A5
//
// Thermistor object
THERMISTOR thermistor(NTC_PIN,        // Analog pin
                      10000,          // Nominal resistance at 25 
                      3950,           // thermistor's beta coefficient
                      10000);         // Value of the series resistor

// Global temperature reading
uint16_t temp;

int DOOR_SENSOR_E       = 0;
int DOOR_SENSOR_A       = 1;
#define PACKET_LEN       26+4
CCPACKET packet;

uint32_t counter  = 0;
uint8_t  d        = 0x41;

void setup()
{
  byte i;

  // Setup LED output pin
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);
  Serial.flush();
  Serial.println("");

  // set up NTC
  pinMode(NTC_POWER_PIN, OUTPUT);    // Configure power pin. This pin will be used to power the thermistor
  powerThermistorOn();        // Power thermistor

  pinMode(DOOR_SENSOR_E, INPUT_PULLUP);  // set pin to input for Everett's Door

  pinMode(DOOR_SENSOR_A, INPUT_PULLUP);  // set pin to input for Allyn's Door

  panstamp.radio.setChannel(RFCHANNEL);
  panstamp.radio.setSyncWord(SYNCWORD1, SYNCWORD0);
  panstamp.radio.setDevAddress(SOURCE_ADDR);
  panstamp.radio.setCCregs();
  panstamp.radio.setTxPowerAmp(PA_LongDistance);

  packet.length = PACKET_LEN; // A-Z + 32bit counter
 
  d = 0x41; // The letter A
  for(i=4 ; i<packet.length ; i++) 
  {
    packet.data[i] = d;
    d = d + 0x01;
  }
}

void loop()
{
  temp = thermistor.read();   // Read temperature
  //Serial.print("Temp F : ");
  Serial.println(temp*0.18+32);

  digitalWrite(LED, HIGH);
  packet.data[0] = 0xFF & (counter >> 24);
  packet.data[1] = 0xFF & (counter >> 16);
  packet.data[2] = 0xFF & (counter >>  8);
  packet.data[3] = 0xFF & (counter >>  0);
  packet.data[4] = 0x00 | (digitalRead(DOOR_SENSOR_E));
  packet.data[5] = 0x00 | (digitalRead(DOOR_SENSOR_A));
  packet.data[6] = 0xFF & (temp >>  8);
  packet.data[7] = 0xFF & (temp >>  0);

  panstamp.radio.sendData(packet);
  digitalWrite(LED, LOW);
  counter = counter + 1;
  panstamp.sleepSec(5);

}

