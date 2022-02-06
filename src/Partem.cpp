//
//  Copyright (C) 2020 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "WiFiConfiguration.hpp"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

#include <algorithm>

namespace {
	constexpr auto reverseSwitches = false;
	constexpr auto pinHigh = reverseSwitches?0:1;
	constexpr auto pinLow = reverseSwitches?1:0;
	
	constexpr auto switch1 = D1;
	constexpr auto switch2 = D2;
	
	IPAddress ip{ 192, 168, 4, 1};
	IPAddress subnet{ 255, 255, 255, 0};

	constexpr auto baudRate = 9600;
	constexpr auto bufferSize = 4096;
	constexpr auto udpPort = 11880;

	IPAddress remoteIp = {};
	int remoteUdpPort = {};

	char bufferFromSerial[ bufferSize];
	uint8_t serialBufferIndex = 0;

	char bufferFromUdp[ bufferSize];

	WiFiUDP udp;

	// either use HW or SW serial...

	//constexpr auto softRx = D2;
	//constexpr auto softTx = D1;
	//SoftwareSerial serial( softRx, softTx);

	auto &serial = Serial;


	bool setupAccessPoint()
	{

		WiFi.disconnect();
		WiFi.softAPConfig( ip, ip, subnet);
		WiFi.mode(WIFI_AP);

		auto result =  WiFi.softAP( myName, myPassword);

		return result;
	}

	void connectToAccessPoint()
	{
		// Connect WiFi
		WiFi.hostname( myName);
		WiFi.begin( networkSID, networkPassword);
	}

	/**
	 * Wait timeOutMs milliseconds for the connection to the access point to come up.
	 *
	 * return true if we have a connection, false if no connection was
	 * established before timeout.
	 *
	 * This function will also flash the builtin LED while waiting for the
	 * connection to come up.
	 */
	bool waitForConnectedToAccessPoint( uint16_t timeOutMs)
	{
		const auto starttime = millis();
		auto status = WiFi.status();
		while ( status != WL_CONNECTED and millis() - starttime < timeOutMs )
		{
			delay( 500);
			status = WiFi.status();
			digitalWrite( LED_BUILTIN, not digitalRead( LED_BUILTIN));
		}

		// the LED is low-active.
		digitalWrite( LED_BUILTIN, status != WL_CONNECTED);
		return status == WL_CONNECTED;
	}

	/**
	 * First try to connect to a configured access point and if we're not connected within a timeout period
	 * set up an access point for ourselves.
	 */
	void setupNetwork()
	{
		connectToAccessPoint();

		if (not waitForConnectedToAccessPoint( 30000))
		{
			setupAccessPoint();
		}
	}

	/**
	 * Send an empty acknowledgement response to the sender of the last
	 * UDP packet.
	 * 
	 * An acknowledgement has the form "=\r". 
	 */
	void SendAck()
	{
		static const char ack[] = {'=', '\r'};
		udp.beginPacket( remoteIp, remoteUdpPort);
		udp.write( ack, sizeof ack / sizeof ack[0]);
		udp.endPacket();
	}

	/**
	 * Detect messages that are to be processed by this controller and should
	 * not be forwarded to the serial port. Return `true` if the message was handled.
	 * 
	 * The command ':Oxy\r' is handled by this processor. This will set switch x to state y.
	 * Switches normally operate camera remotes.
	 */
	bool HandleLocally( char *buffer, uint16_t count)
	{
		// count will typically be 5, but we're not interested in the
		// carriage return character.
		if (count >= 4 and buffer[1] == 'O')
		{
			const auto pin = buffer[2] == '1'?switch1:switch2;
			const auto value = buffer[3] == '0'?pinLow:pinHigh;
			digitalWrite( pin, value);
			
			// immediately send an acknowledgement
			SendAck();
			return true;
		}
		
		return false;
	}


	/**
	 * Forward data that is received from the UDP port to the
	 * serial device.
	 */
	bool FromUdpToSerial()
	{
		  auto packetSize = udp.parsePacket();
		  if (packetSize > 0)
		  {
			packetSize = std::min( packetSize, bufferSize);
		    remoteIp = udp.remoteIP();
		    remoteUdpPort = udp.remotePort();

		    udp.read( bufferFromUdp, bufferSize);

		    if (not HandleLocally( bufferFromUdp, packetSize))
		    {
		    	serial.write( bufferFromUdp, packetSize);
		    }

		    return true;
		  }

		  return false;
	}

	/**
	 * Forward data that is received from the serial device to the UDP port
	 */
	bool FromSerialToUdp()
	{
		bool receivedReturn = false;
		while (not receivedReturn and serial.available())
		{
			const auto character = serial.read();

			if (serialBufferIndex < bufferSize)
			{
				bufferFromSerial[serialBufferIndex++] = character;
			}

			receivedReturn = (character == '\n' or character == '\r');
		}

		if (receivedReturn)
		{
			udp.beginPacket( remoteIp, remoteUdpPort);
			udp.write( bufferFromSerial, serialBufferIndex);
			serialBufferIndex = 0;
			udp.endPacket();
		}

		return receivedReturn;
	}
}

void setup()
{

  serial.begin( baudRate);
  pinMode( LED_BUILTIN, OUTPUT);
  pinMode( switch1, OUTPUT);
  digitalWrite( switch1, pinLow);
  
  pinMode( switch2, OUTPUT);
  digitalWrite( switch2, pinLow);

  setupNetwork();

  udp.begin( udpPort);

}

void loop()
{
	FromUdpToSerial();
	FromSerialToUdp();
}
