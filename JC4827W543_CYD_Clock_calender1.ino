//
// ESP32 CYD (Cheap-Yellow-Display) Weather & Time Display
// written by Larry Bank
// Copyright (c) 2024 BitBank Software, Inc.
// All credit to the above Author, much of the code from his original sketches at https://github.com/bitbank2/CYD_Projects/tree/main
// altered to run on the 4.3" JC4827W543 w/resistive or capacitive touch
#define LOG_TO_SERIAL
// Define the display type used and the rest of the code should "just work"
#define LCD DISPLAY_CYD_543
// Define your time zone offset in seconds relative to GMT. e.g. Eastern USA = -(3600 * 5)
// The program will try to get it automatically, but will fall back on this value if that fails
#define TZ_OFFSET (3600)
int iTimeOffset;  // offset in seconds

// Uncomment this line to switch to use Openweathermap.org; make sure you have an API key first
#define USE_OPENWEATHERMAP

String town = "CITY";  // Customize for your city/town
String Country = "UK";     // Customize for your country (2-letter code)
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + town + "," + Country + "&units=metric&APPID=";
const String key = ".................................."; /*Your Open Weather API Key*/


#include <NTPClient.h>  //https://github.com/taranais/NTPClient
#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <ESP32Time.h>
ESP32Time rtc(0);
HTTPClient http;
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <bb_spi_lcd.h>
#include "Roboto_Black_28.h"
#include "Roboto_Black_16.h"
#include "Roboto_25.h"
#include "Roboto_Thin66pt7b.h"
#include"Orbitron_Bold_66.h"
#include "DSEG7_Classic_Bold_66.h"
// black and white graphics
#include "humidity_4bpp.h"
#include "sunrise_4bpp.h"
#include "sunset_4bpp.h"
#include "temp_4bpp.h"

#define WIFI_SSID "your ssid"
#define WIFI_PWD "yourPassword"


BB_SPI_LCD lcd;
// Define NTP Client to get time
struct tm myTime;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int iWind, rel_humid, temp, feels_temp, mintemp, maxtemp, cc_icon;
String sSunrise, sSunset, updated;
int iTemp[16], iHumidity[16], iWeatherCode[16];  // hourly conditions
uint8_t uvIndex[8];
char szOldTime[32];
int iRainChance[16];
int iDigitPos[8];  // clock digit positions
int iCharWidth, iColonWidth;
int iStartX, iStartY;

#define FONT DSEG7_Classic_Bold_66
#define FONT_GLYPHS DSEG7_Classic_Bold_66Glyphs
//#define FONT Roboto_Thin66pt7b
//#define FONT_GLYPHS Roboto_Thin66pt7bGlyphs
//#define LARGE_FONT Roboto_Black_28

uint16_t usColor = TFT_CYAN;  // time color

const char *szMonths[] = { "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };
const int iMonthLens[] = { 31, 28, 31, 30, 31, 30, 31, 30, 30, 31, 30, 31 };
const char *szDays[] = { "S", "M", "T", "W", "T", "F", "S" };
void drawCalendar(struct tm *pTime, const GFXfont *pFont , int x, int y) {
  
  int i, j, k, dx, dy;  // calculate delta of each cell
  uint16_t u16LineClr, u16CellClr, u16TextClr;
  int tx, ty, iDay, iDeltaX;
  int iMonth, iStartDay;
  int16_t x1, y1;
  uint16_t w, w2, h, h2;

  iMonth = pTime->tm_mon;
  iStartDay = pTime->tm_wday - ((pTime->tm_mday - 1) % 7);
  if (iStartDay < 0) iStartDay += 7;

  u16LineClr = TFT_RED;
  u16CellClr = TFT_BLACK;
  u16TextClr = TFT_WHITE;

  lcd.setFreeFont(pFont);
  lcd.getTextBounds("00", 0, 0, &x1, &y1, &w, &h);
  
// Adjusting the cell width (dx) and height (dy) for larger size

  dx = 1 * (w + 4);        // Increased width of each cell
  dy = 1 * ((h * 5) / 4);   // Increased height of each cell

  // draw the month name area
  lcd.setTextColor(u16TextClr, TFT_BLACK);
  lcd.drawRect(x, y, (dx * 7) + 8, dy + 2, u16LineClr);
  lcd.fillRect(x + 1, y + 1, (dx * 7) + 6, dy - 1, u16CellClr);
  lcd.getTextBounds(szMonths[iMonth], 0, 0, &x1, &y1, &w2, &h2);
  iDeltaX = 1 + ((dx * 7 + 6) - w2) / 2;
  lcd.setCursor(x + iDeltaX, y + h2 + (dy - h2) / 2);
  lcd.print(szMonths[iMonth]);

  // draw the grid and cells
  k = (iStartDay + iMonthLens[iMonth] > 35) ? 8 : 7;
  for (j = 1; j < k; j++) {    // y
    for (i = 0; i < 7; i++) {  // x
      lcd.drawRect(x + (i * (dx + 1)), y + (j * (dy + 1)), dx + 2, dy + 2, u16LineClr);
    }
  }  // for j
  // draw the day names
  ty = y + dy + 1;
  tx = x + 1;
  for (i = 0; i < 7; i++) {
    lcd.setCursor(tx + (dx - w / 2) / 2, ty + h + (dy - h) / 2);
    lcd.print(szDays[i]);
    tx += dx + 1;
  }
  // draw the days of the month
  ty = y + 1 + (dy + 1) * 2;
  tx = x + 1 + (iStartDay * (dx + 1));
  iDay = iStartDay;
  for (i = 1; i <= iMonthLens[iMonth]; i++) {
    uint16_t u16Clr;
    iDeltaX = (i < 10) ? (dx - w / 2) / 2 : (dx - w) / 2;

    u16Clr = (i == pTime->tm_mday) ? u16TextClr : u16CellClr;
  
    lcd.fillRect(tx, ty, dx, dy, u16Clr);
    u16Clr = (i != pTime->tm_mday) ? u16TextClr : u16CellClr;
    lcd.setTextColor(u16Clr, TFT_BLACK);
    lcd.setCursor(tx + iDeltaX, ty + h + (dy - h) / 2);
    lcd.print(i, DEC);
    tx += dx + 1;
    iDay++;
    if (iDay == 7) {  // next line
      iDay = 0;
      tx = x + 1;
      ty += dy + 1;
    }
  }
} /* drawCalendar() */


void DisplayWeather(void) {
  char *s, szTemp[64];
  int i, j;

  lcd.fillScreen(TFT_BLACK);
  // show the update time+date
  lcd.setFont(FONT_8x8);
  lcd.setCursor(0, lcd.height() - 8);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.print("Last Update: ");
  lcd.print(updated.c_str());
  Serial.println(szTemp);


  lcd.drawBMP((uint8_t *)sunrise_4bpp, 0, 20, 1, -1, 2);
  lcd.drawBMP((uint8_t *)sunset_4bpp, 0, 60);
  lcd.drawBMP((uint8_t *)humidity_4bpp, 443, 68); 
  lcd.drawBMP((uint8_t *)temp_4bpp, 446, 0);

  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  strcpy(szTemp, sSunrise.c_str());
  szTemp[5] = 0;  // don't need the AM/PM part
  s = szTemp;
  if (s[0] == '0') s++;  // skip leading 0
  lcd.setCursor(0, 20);  

  lcd.setFreeFont(&Roboto_25);
  lcd.print(s);
  strcpy(szTemp, sSunset.c_str());
  szTemp[5] = 0;  // don't need the AM/PM part
  s = szTemp;
  if (s[0] == '0') s++;   // skip leading 0
  lcd.setCursor(0, 115); 
  lcd.print(s);
  lcd.setCursor(415, 100);
  lcd.print(rel_humid, DEC);

  lcd.setCursor(398, 25); 
  sprintf(szTemp, "%dC", temp);
  lcd.print(szTemp);

}
int GetWeather(void) {
  char szTemp[64];
  int i, iHour, httpCode = -1;

  lcd.println("Getting Weather Data...");
  http.setAcceptEncoding("identity");

#ifdef USE_OPENWEATHERMAP
  http.begin(endpoint + key);

#endif
  httpCode = http.GET();  //send GET request
  if (httpCode != 200) {
#ifdef LOG_TO_SERIAL
    Serial.print("Error on HTTP request: ");
    Serial.println(httpCode);
#endif
#ifdef ARDUINO_ARCH_ESP32
    http.end();
#endif
    return 0;
  } else {
#ifdef ARDUINO_ARCH_ESP32
    String payload = http.getString();
    http.end();

#endif  // ESP32
    lcd.printf("%d bytes recvd\n", payload.length());
#ifdef LOG_TO_SERIAL
    sprintf(szTemp, "Received %d bytes from server\n", payload.length());
    Serial.print(szTemp);
    if (payload.length() < 4000) {
      Serial.printf(payload.c_str());
    }
#endif
    //     StaticJsonDocument<80000> doc;
    DynamicJsonDocument doc(26000);  // hopefully this is enough to capture the data; latest request returns 48k of text
#ifdef LOG_TO_SERIAL
    sprintf(szTemp, "JsonDocument::capacity() = %d\n", (int)doc.capacity());
    Serial.print(szTemp);
#endif
    DeserializationError err = deserializeJson(doc, payload);
    doc.shrinkToFit();
    if (err) {
#ifdef LOG_TO_SERIAL
      Serial.print("deserialization error ");
      Serial.println(err.c_str());
#endif
      lcd.printf("deserialization error %s\n", err.c_str());
      return 0;
    }
#ifdef LOG_TO_SERIAL
    Serial.println("Deserialization succeeded!");
#endif

    //     JsonArray cca = doc["main"].as<JsonArray>();
    feels_temp = doc["main"]["feels_like"];
    rel_humid = doc["main"]["humidity"];
    temp = doc["main"]["temp"];
    mintemp = doc["main"]["temp_min"];
    maxtemp = doc["main"]["temp_max"];
    const time_t iSunrise = doc["sys"]["sunrise"].as<int>() + iTimeOffset;
    struct tm *stime;
    stime = gmtime(&iSunrise);
   // sprintf(szTemp, "%d:%02d", (stime->tm_hour > 12) ? stime->tm_hour - 12 : stime->tm_hour, stime->tm_min);
   sprintf(szTemp, "%d:%02d", stime->tm_hour, stime->tm_min);
    sSunrise = String(szTemp);
    const time_t iSunset = doc["sys"]["sunset"].as<int>() + iTimeOffset;
    stime = gmtime(&iSunset);
    sprintf(szTemp, "%d:%02d", (stime->tm_hour > 12) ? stime->tm_hour - 12 : stime->tm_hour, stime->tm_min);
   //sprintf(szTemp, "%d:%02d", stime->tm_hour, stime->tm_min);
    sSunset = String(szTemp);
    iWind = doc["wind"]["speed"];
    time_t iUpdated = doc["dt"].as<int>() + iTimeOffset;
    updated = String(ctime(&iUpdated));  // last update time

#ifdef LOG_TO_SERIAL
   
    sprintf(szTemp, "sunrise: %s, sunset: %s, mintemp = %d, maxtemp = %d\n", sSunrise.c_str(), sSunset.c_str(), mintemp, maxtemp);
    Serial.print(szTemp);

#endif

    return 1;
  }
} /* GetWeather() */



void GetInternetTime() {
  char szIP[32];

  iTimeOffset = TZ_OFFSET;  // start with fixed offset you can set in the program
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(iTimeOffset);  //My timezone
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  unsigned long epochTime = timeClient.getEpochTime();
  //Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);
  memcpy(&myTime, ptm, sizeof(myTime));  // get the current time struct into a local copy
  rtc.setTime(epochTime);                // set the ESP32's internal RTC to the correct time
  Serial.printf("Current time: %02d:%02d:%02d\n", myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
 timeClient.end();  // don't need it any more
} /* GetInternetTime() */

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PWD);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting Wifi");
  }

  Serial.println("WiFi Connected!");
  delay(5000);
  timeClient.begin();
  lcd.begin(LCD);
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(FONT_12x16);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);


  // Prepare positions of clock digits for format HH:MM:SS
iCharWidth = FONT_GLYPHS['0' - ' '].xAdvance;
iColonWidth = FONT_GLYPHS[':' - ' '].xAdvance;

// Update start X position for the new format 00:00:00
iStartX = ((lcd.width() - (8 * iCharWidth + 2 * iColonWidth)) / 2)+35; // Adjust for HH:MM:SS

// Adjust start Y position as needed
iStartY = lcd.height() - 30;  // Change if needed based on display height

// Update digit positions for HH:MM:SS
iDigitPos[0] = iStartX;                         // First hour digit
iDigitPos[1] = iStartX + iCharWidth;            // Second hour digit
iDigitPos[2] = iStartX + iCharWidth * 2;        // First colon
iDigitPos[3] = iStartX + iColonWidth + iCharWidth * 2;   // First minute digit
iDigitPos[4] = iDigitPos[3] + iCharWidth;       // Second minute digit
iDigitPos[5] = iDigitPos[4] + iCharWidth;       // Second colon
iDigitPos[6] = iDigitPos[5] + iColonWidth;      // First second digit
iDigitPos[7] = iDigitPos[6] + iCharWidth;       // Second second digit

} /* setup() */

void DisplayTime(void) {
  char szTemp[2], szTime[32];
  int i, iHour, iMin, iSec;

  // Get current time from RTC
  iHour = rtc.getHour(true);
  iMin = rtc.getMinute();
  iSec = rtc.getSecond();

  // Format time to include seconds
  sprintf(szTime, "%02d:%02d:%02d", iHour, iMin, iSec);

  // Flash the colons every second (you can modify this behavior if desired)
 // if (iSec & 0x1) {  
  //  szTime[2] = ' ';
  //  szTime[5] = ' ';
 // }
 // szTime[2] = (iSec % 1 == 0) ? ':' : ' ';  // dont flash colon
 // szTime[5] = (iSec % 1 == 0) ? ':' : ' ';  // dontflash colon....change 1 to 2 to flash



  // Compare with previous time to only update changed digits
  if (strcmp(szTime, szOldTime)) {  
    szTemp[1] = 0;
    lcd.setFreeFont(&FONT);
    for (i = 0; i < 8; i++) {
      if (szTime[i] != szOldTime[i]) {
        szTemp[0] = szOldTime[i];
        lcd.setTextColor(TFT_BLACK, TFT_BLACK + 1);  // Erase old character
        lcd.drawString(szTemp, iDigitPos[i], iStartY);  // Draw new character

        // Skip leading zero for hours if desired
        if (i == 0 && szTime[0] == '0') continue;

        lcd.setTextColor(usColor, TFT_BLACK);
        szTemp[0] = szTime[i];
        lcd.drawString(szTemp, iDigitPos[i], iStartY);
      }  // if character needs redraw
    }    // for each character
    strcpy(szOldTime, szTime);
  }
} /* DisplayTime() */

void loop() {
  int i;

  //if (ConnectToInternet()) {
  GetInternetTime();  // update the internal RTC with accurate time
  GetWeather();
  DisplayWeather();

  drawCalendar(&myTime, &Roboto_25, 105, 3);// calender position
  strcpy(szOldTime, "          ");  // force complete repaint of time after Wi-Fi update

  for (i = 0; i < 3600; i++) {  // Update every second
    DisplayTime();
    delay(1000);
  }

  delay(10000);  // Add additional delays as needed
} /* loop() */



 