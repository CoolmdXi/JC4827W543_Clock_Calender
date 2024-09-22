# JC4827W543_Clock_Calender
A CYD Board showing Clock and calendar information with sunset sunrise temperature and Humidity

ESP32 CYD Weather & Time Display
This project is an ESP32-based weather and time display using the CYD (Cheap Yellow Display) screen. It pulls real-time weather data from OpenWeatherMap and displays it alongside the current date and time.

Features
Weather Information: Current temperature, humidity, and sunrise, sunset are fetched from OpenWeatherMap.
Time Display: The current time is synchronized using NTP (Network Time Protocol).
Calendar: A simple calendar display that highlights the current day.
Weather Graphics: Icons representing sunrise, sunset, humidity, and temperature.
Customizable City: Easily customize your city/town and country for localized weather.

Hardware Requirements

ESP32 Board: The core hardware running the code.

A devboard known as JC4827W543 known as CYD 543 Display: A Cheap-Yellow-Display (543 model) for showing weather and time data.
Wi-Fi: Requires a Wi-Fi connection for fetching weather data and NTP synchronization.
Libraries Used
NTPClient: For synchronizing the time via NTP.
WiFiManager: For managing Wi-Fi connections.
HTTPClient: For making HTTP requests to fetch weather data.
ArduinoJson: For parsing the weather data from OpenWeatherMap.
bb_spi_lcd: For managing the CYD display.
ESP32Time: For handling real-time clock functions on the ESP32.

Configure Wi-Fi Settings: Update your Wi-Fi SSID and password in the WIFI_SSID and WIFI_PWD definitions:

#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PWD "YourWiFiPassword"

OpenWeatherMap API Key: Replace the key string with your own OpenWeatherMap API key

const String key = "YOUR_API_KEY";
You can obtain an API key by creating a free account on OpenWeatherMap.

Timezone Offset: Define your timezone offset relative to GMT in seconds. For example, Eastern USA is UTC -5 hours:

#define TZ_OFFSET -(3600 * 5) // Eastern USA
Customize Location: Update the town and Country variables with your preferred location:


Display Features
Clock: A 7-segment styled digital clock with hours, minutes, and seconds.
Weather Information: Displays temperature, humidity, and Sunrise and sunset.
Icons: Graphical icons for sunrise, sunset, and humidity.

 ESP32 CYD (Cheap-Yellow-Display) Weather & Time Display
 Original code by Larry Bank
 Copyright (c) 2024 BitBank Software, Inc.
 
 Adapted from Larry Bank's original work with the 543 board and customized to fit my own needs.
  
 Most of the core code is Larry Bank's great work, and his efforts inspired me to 
 modify and extend it for personal use.
 For more information about Larry's work, visit https://github.com/LarryBank


