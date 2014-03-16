#include <TinyGPS++.h>


//Graphics Includes
#include <glcd.h> // include the graphics LCD library
#include "fonts/allFonts.h"         // system and arial14 fonts are used

#include "bitmaps/allBitmaps.h"
//#include "bitmaps/BoatBitmaps.h"

#include<stdlib.h>

//#include <TinyGPS.h>
// Upgraded to use TinyGPS Plus
TinyGPSPlus gps;

//SoftwareSerial mySerial(13, 12, true); // RX, TX, true=invert for connection to MR350, don't invert for EM406
// EM406: RED = Tx (i.e. Rx from GPS) -> 13
// EM406: YELLOW = Rx (i.e. Tx from GPS) -> 12
//MR350: Green = Tx (i.e. Rx from GPS) -> 13
//MR350: White = Rx - currently don't connect, since this seems to cause issues

//int j = 7;

  char* Speed = "   "; //used for displaying the speed as xxx
  char* LatStr = "         "; // 9 chars used for Lat
  char* LonStr = "         "; // 9 chars used for Lon
  char* SpeedUnit[] = {"Mph", "Nm", "Km" };
  char* screen_mode = {"Mode  "}; //used for displaying the mode on the screen

volatile int old_speed = 0, new_speed = 0, gps_sat = 0, gps_sat_old = 0, rose_cardinal = 0;
long l_speed_old = 0;
int unit_width = 0; //used for the width of the speed units text
volatile int speed_unit = 0; // 0 for Mph, 1 for knots, 2 for Km/H
int old_speed_unit = 0; //used to store previous speed unit setting

int mode = 0, old_mode = 0; // Used for displaying data on the screen
// 0 = Satellites
// 1 = Speed
// 2 = Time
// 3 = Setup

// Time zone
int time_zone = -7;

// Screen Update Time
long previousMillis = 0;
long screenUpdate = 1000;

//Define Areas on the screen
gText textArea_Speed; //used for the current speed
//gText textArea_Cardinal; //used for the N/S/E/W etc direction
//gText textArea_Units; //used for the current speed units, i.e. MPH, Knots etc
//gText textArea_LL; //used for the current Lat/Lon
//gText textArea_Serial; //used for serial debug
//gText textArea_Sats; // used for displaying the number of sats being received

//int rx_packet = 0; //count the received packets
//Define fonts for Display areas
//Verdana24
//fixednums15x31
//System5x7
//fixednums8x16
//Arial_bold_14
//Arial_14
//Arial34x47
//Wendy3x5
//Gill_10x51
//GillSans_56
//Anadale


//#define FONT_Speed GillSans_56
#define FONT_Speed Courier97 // used for 2 digit speed
#define FONT_Speed2 Courier74 //used for 3 digit speed
//#define FONT_Cardinal System5x7
#define FONT_Cardinal Arial12
#define FONT_Unit Arial_14
#define FONT_LL System5x7
#define FONT_Sats Wendy3x5
#define FONT_Time Arial_14


//#define MAX_ARC_SPEED 50 //used for the maximum speed of the arc plot

// define the push button connections
#define BUTTONL 2  // INT0
#define BUTTONR 3  // INT1



//
/* Main Setup Loop */
//


void setup()
{
  
  //Set up pins for push buttons
  pinMode(BUTTONL, INPUT_PULLUP);     // DATA0 (INT0)
  pinMode(BUTTONR, INPUT_PULLUP);     // DATA1 (INT1)
  
 

  
//  int p_speed = 0; // used for integer percentage speed
  
  Serial.begin(9600);
  //GPS Set up
//  mySerial.begin(38400);
//  delay(500);
 //SiRF command to set NMEA rate to 38400
Serial.println("$PSRF100,1,4800,8,1,0*0E"); // to set back to 4800, don't forget the CR - i.e. println
// Serial.println("$PSRF100,1,9600,8,1,0*0D");
// Serial.println("$PSRF100,1,19200,8,1,0*38");
// Serial.println("$PSRF100,1,38400,8,1,0*3D");
//Serial.println("$PSRF100,1,57600,8,1,0*36"); // set to 57600
//delay(500);
//Serial.end();
Serial.begin(4800);
 delay(500);
 
 
GLCD.Init(NON_INVERTED);
GLCD.ClearScreen();

//GLCD.DrawBitmap(BigAndy128x64, 0,0); // draw the BigAndy Icon at 0,0
//delay(1000); //wait 1 seconds
GLCD.ClearScreen();
GLCD.SelectFont(System5x7);
GLCD.CursorTo(1,1);
GLCD.Puts("HW IO NMEA@4800bps");
GLCD.CursorTo(1,2);
GLCD.Puts("GPS_EM406_Grx_15_NoInt");
GLCD.CursorTo(1,4);
GLCD.Puts("2014-03-16");
delay(3000);

GLCD.ClearScreen();

//Test images
//GLCD.DrawBitmap(Unit_Mph_h, 0,0);
//delay(3000);
//GLCD.DrawBitmap(Unit_Knots_h, 0,0);
//delay(3000);
//GLCD.DrawBitmap(Unit_Kmh_h, 0,0);
//delay(3000);

//Define Text Areas on Graphic Display

textArea_Speed.DefineArea(0,0,95,63);
//textArea_Cardinal.DefineArea(96,6,127,15); //used for the N/S/E/W etc direction
//textArea_Sats.DefineArea(101,0,127,5); // used for Sat numbers
//textArea_Units.DefineArea(96,49,127,63); // used for speed units

//Print default Cardinal direction
//textArea_Cardinal.SelectFont(FONT_Cardinal, BLACK);
//textArea_Cardinal.CursorTo(0,0);
//textArea_Cardinal.Puts("Null");

//Print some stuff in each of the text areas
textArea_Speed.SelectFont(FONT_Speed, BLACK);

//Print the unit - MPH on bottom right
//textArea_Units.SelectFont(FONT_Unit, BLACK);
//Determine width of the units, center and print it
//unit_width = textArea_Units.StringWidth(SpeedUnit[speed_unit]);
//textArea_Units.DrawString((char *)SpeedUnit[speed_unit], (16-(unit_width/2)),0);

//textArea_Speed.ClearArea();
//print_gps_status(0); // print No Fix

//await mode change
//   attachInterrupt(0, ISR_INT0, FALLING);  

  mode = 1; //default to the Speed screen after startup
  print_speed(0);
  print_speed_units(0);
  
} //end of setup()

void loop()
{

//  Serial.println("Start of main loop");
      // process new gps info here
      int speed_width =0, cardinal_width=0;
      long lat, lon, l_speed;
      float flat, flon, flat2, flon2, speed_k = 0;
      float dist2 = 0; // fix_age;
      char* cardinal = "   "; //to store compass cardinal
    unsigned long currentMillis = millis();


while (Serial.available() > 0){
//    int c = mySerial.read();
        int c = Serial.read();
        gps.encode(c);
//        textArea_Speed.ClearArea();
//        textArea_Speed.SelectFont(FONT_LL, BLACK);
//        textArea_Speed.CursorTo(1,1);
//        textArea_Speed.Puts("F Chk ");
//        textArea_Speed.PrintNumber((int)gps.failedChecksum());
//        textArea_Speed.CursorTo(1,2);
//        textArea_Speed.Puts("Chars  ");
//        textArea_Speed.PrintNumber((int)gps.charsProcessed());
//        textArea_Speed.CursorTo(1,3);
//        textArea_Speed.Puts("Fix  ");
//        textArea_Speed.PrintNumber((int)gps.sentencesWithFix());


//        rx_packet++;

   }
   
   if (digitalRead(BUTTONL) == LOW) {
  mode++;
 if (mode == 4)
  {
    mode = 0;
  } 
  GLCD.ClearScreen();
//  GLCD.SelectFont(Arial_bold_14);
//  GLCD.Puts("Mode ");
//  GLCD.PrintNumber(mode);
//  GLCD.SelectFont(System5x7);
switch (mode){
 
 case 0:
  GLCD.DrawBitmap(Mode_Status, 0,0); // draw Mode_Status at 0,0
    delay(1000);
    GLCD.ClearScreen();
  break;
 case 1:
  GLCD.DrawBitmap(Mode_Speed, 0,0); // draw Mode_Speed at 0,0
    delay(1000);
    GLCD.ClearScreen();
        print_speed(new_speed);
  break;
 case 2:
  GLCD.DrawBitmap(Mode_Time, 0,0); // draw Mode_Clock at 0,0
    delay(1000);
    GLCD.ClearScreen();
  break;
 case 3:
  GLCD.DrawBitmap(Mode_Setup, 0,0); // draw Mode_Setup at 0,0
    delay(1000);
    GLCD.ClearScreen();
  break;

}


  
}

if (mode == 1){
   if (digitalRead(BUTTONR) == LOW) {
     speed_unit++;
     if (speed_unit > 2)
     {
       speed_unit = 0;
     }
     GLCD.DrawBitmap(Mode_Speed_Units, 0,0); // draw Mode_Speed_Units at 0,0
    delay(1000);
    GLCD.ClearScreen();
    print_speed(old_speed);
   }
}

// Update the screen timer

if(currentMillis - previousMillis > screenUpdate) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    if (old_mode != mode){
    old_mode = mode;
//      GLCD.ClearScreen();
    }
    if (mode == 0)
   {
      print_satellites();
   }
    if (mode == 1)
    {
      if (old_speed != new_speed){
      old_speed = new_speed; 
      print_speed(new_speed);
      }
       draw_rose(gps.course.deg());
       print_speed_units(speed_unit);

    }
    if (mode == 2)
    {
      print_time();
    }
        if (mode == 3)
    {
      print_setup();
    }

    
} // end of screen update counter



   
   
   
//   if (old_speed_unit != speed_unit){
//     if (speed_unit == 0){
//        print_speed_units(speed_unit);
//     }
//     if (speed_unit == 1){
//         print_speed_units(speed_unit);
//     }
//     if (speed_unit == 2){
//       print_speed_units(speed_unit);
//     }
//   }
//        
    /* Speed */
    
    //Select speed units 0=Mph, 1=Nm, 2=Kmh
    switch (speed_unit) {
      case 0:
        new_speed = (int)gps.speed.mph();
        break;
      case 1:
        new_speed = (int)gps.speed.knots();
        break;
      case 2:
        new_speed = (int)gps.speed.kmph();
        break;
      
    }
    



//Print cardinal
//textArea_Cardinal.SelectFont(FONT_Cardinal, BLACK);
//textArea_Cardinal.ClearArea(); // clear the previous value


//cardinal_width = textArea_Cardinal.StringWidth(gps.cardinal(gps.course.deg()));
//textArea_Cardinal.DrawString((char*)gps.cardinal(gps.course.deg()), (16-(cardinal_width/2)),0);


//draw_rose(gps.course.deg());
    
     
    
    /* Status */
//    if (gps_sat != gps_sat_old){
//    gps_sat = (int)gps.satellites.value();
//      print_gps_status((int)gps_sat);
//      gps_sat_old = gps_sat;
//rx_packet = 0;
//    }

} // end of loop()

//////////////////////////////////////////////////////////////////////////////
//
// Print speed
//
//////////////////////////////////////////////////////////////////////////////

void print_speed(int l_speed)
{
  int speed_width = 0;
  char* Speed = "   ";
  itoa(l_speed, Speed, 10);
  speed_width = textArea_Speed.StringWidth(Speed);
  
  textArea_Speed.ClearArea(); // clean out previous value
  //If the speed is 2 digits, then larger font, else smaller font
  if (l_speed >99){
    textArea_Speed.SelectFont(FONT_Speed2, BLACK);
  }
  else {
    textArea_Speed.SelectFont(FONT_Speed, BLACK);
  }

  if (l_speed < 10){
    textArea_Speed.DrawString(Speed, (64-(speed_width/2)),0);
  }
  else{

  textArea_Speed.DrawString(Speed, (48-(speed_width/2)),0);
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// print_ssetup
//
//////////////////////////////////////////////////////////////////////////////

void  print_setup()
{
  GLCD.ClearScreen();
   GLCD.SelectFont(System5x7);
  GLCD.CursorTo(1,1);
  GLCD.Puts("Setup");
  GLCD.CursorTo(1,2);
  GLCD.Puts("Timezone = ");
  GLCD.PrintNumber(time_zone);
  GLCD.CursorTo(1,3);
  GLCD.Puts("Default Speed = ");
  switch (speed_unit){
    case 0:
    GLCD.Puts("Mph");
    break;
    case 1:
    GLCD.Puts("Knots");
    break;
    case 2:
    GLCD.Puts("Km/h");
    break;
    
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// print_satellites
//
//////////////////////////////////////////////////////////////////////////////


void print_satellites()
{
  int sats;
  GLCD.ClearScreen();
  GLCD.CursorTo(1,1);
  GLCD.Puts("Mode 0 - Status");
  GLCD.CursorTo(1,2);
  GLCD.Puts("GPS sats = ");
  sats = gps.satellites.value();
  if (sats < 10){
    GLCD.Puts(" ");
}

  GLCD.PrintNumber(sats);
}

//////////////////////////////////////////////////////////////////////////////
//
// print_time
//
//////////////////////////////////////////////////////////////////////////////

void print_time()
{
  int gps_hour, gps_min, gps_sec;
  
  GLCD.ClearScreen();
  GLCD.SelectFont(FONT_Time);
//  GLCD.CursorTo(1,1);
//  GLCD.Puts("Mode 2 - Clock");

  GLCD.CursorTo(1,1); 
  GLCD.Puts("UTC = ");
  gps_hour=gps.time.hour();
  gps_min=gps.time.minute();
  gps_sec=gps.time.second();

  if (gps_hour <10){
    GLCD.Puts(" ");
  }
  GLCD.PrintNumber(gps_hour);
  GLCD.Puts(":");
  if (gps_min <10){
    GLCD.Puts("0");
  }
  GLCD.PrintNumber(gps_min);
  GLCD.Puts(":");
  if (gps_sec <10){
    GLCD.Puts("0");
  }
  GLCD.PrintNumber(gps_sec);

  GLCD.CursorTo(1,3); 
  GLCD.Puts("PDT = ");
  gps_hour=gps_hour + time_zone;
  if (gps_hour < 0){
    gps_hour = gps_hour + 24;
  }
  if (gps_hour <10){
    GLCD.Puts(" ");
  }
  GLCD.PrintNumber(gps_hour);
  GLCD.Puts(":");
  if (gps_min <10){
    GLCD.Puts("0");
  }

  GLCD.PrintNumber(gps_min);
  GLCD.Puts(":");
  if (gps_sec <10){
    GLCD.Puts("0");
  }
  GLCD.PrintNumber(gps_sec);
  GLCD.SelectFont(System5x7);

}

//////////////////////////////////////////////////////////////////////////////
//
// print_gps_status
//
//////////////////////////////////////////////////////////////////////////////

//void print_gps_status(int sats)
//{
//  textArea_Sats.ClearArea();
//  textArea_Sats.SelectFont(FONT_Sats, BLACK);
//  textArea_Sats.CursorTo(0,0);
//  switch (sats) {
//    case 0:
//    textArea_Sats.Puts("----");
//    break;
//    default:
//    textArea_Sats.PrintNumber(sats);  
//  }
//}

//////////////////////////////////////////////////////////////////////////////
//
// print_speed_units
//
//////////////////////////////////////////////////////////////////////////////

//void print_speed_units(int speed_unit)
//{
//  textArea_Units.ClearArea(); //clear the old units
//  unit_width = textArea_Units.StringWidth(SpeedUnit[speed_unit]); //determine the width of the unit text
//textArea_Units.DrawString(SpeedUnit[speed_unit], (16-(unit_width/2)),0); //print the unit Mph/Kmh/Knots
//old_speed_unit = speed_unit ;
//}

//////////////////////////////////////////////////////////////////////////////
//
// print_mode
//
//////////////////////////////////////////////////////////////////////////////

void print_speed_units(int speed_unit)
{
//  strcpy(screen_mode,"Mode ");

//  itoa(speed_unit, screen_mode[5], 0);
//  textArea_Speed.ClearArea(); //clear the speed area
//  textArea_Speed.SelectFont(FONT_Cardinal);
//  unit_width = textArea_Units.StringWidth(screen_mode); //determine the width of the unit text
//textArea_Speed.Puts("Mode "); 
switch (speed_unit) {
  case 0:
   GLCD.DrawBitmap(Unit_Mph_h, 96,48); // draw Mph unit

  break;
    case 1:
   GLCD.DrawBitmap(Unit_Knots_h, 96,48); // draw Knots unit

   break;
  case 2:
     GLCD.DrawBitmap(Unit_Kmh_h, 96,48); // draw Kmh unit

  break;


}

//  textArea_Units.ClearArea(); //clear the old units
//  unit_width = textArea_Units.StringWidth(SpeedUnit[speed_unit]); //determine the width of the unit text
//textArea_Units.DrawString(SpeedUnit[speed_unit], (16-(unit_width/2)),0); //print the unit Mph/Kmh/Knots
old_speed_unit = speed_unit ;
//delay(1000);
//textArea_Speed.ClearArea();
}

//////////////////////////////////////////////////////////////////////////////
//
// satellite_display
//
//////////////////////////////////////////////////////////////////////////////
//
//$GPGSV
//
//GPS Satellites in view
//
//eg. $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
//    $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
//    $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D
//
//
//    $GPGSV,1,1,13,02,02,213,,03,-3,000,,11,00,121,,14,13,172,05*67
//
//
//1    = Total number of messages of this type in this cycle
//2    = Message number
//3    = Total number of SVs in view
//4    = SV PRN number
//5    = Elevation in degrees, 90 maximum
//6    = Azimuth, degrees from true north, 000 to 359
//7    = SNR, 00-99 dB (null when not tracking)
//8-11 = Information about second SV, same as field 4-7
//12-15= Information about third SV, same as field 4-7
//16-19= Information about fourth SV, same as field 4-7

void satellite_display()

{
  int sat_count = 0;
  
  TinyGPSCustom magneticVariation(gps, "GPGSV", 10);
  
}

//////////////////////////////////////////////////////////////////////////////
//
// draw_rose
//
//////////////////////////////////////////////////////////////////////////////


void draw_rose(float course)
{
  int direction = (int)((course +11.25f) / 22.5f);
  switch (direction) {
 case 0:
  GLCD.DrawBitmap(Rose_N, 96,0); // draw the compass arrow
  break;
  case 1:
    GLCD.DrawBitmap(Rose_NNE, 96,0); // draw the compass arrow
  break;
  case 2:
    GLCD.DrawBitmap(Rose_NE, 96,0); // draw the compass arrow
  break;

  case 3:
    GLCD.DrawBitmap(Rose_ENE, 96,0); // draw the compass arrow
  break;
  case 4:
    GLCD.DrawBitmap(Rose_E, 96,0); // draw the compass arrow
  break;
  case 5:
    GLCD.DrawBitmap(Rose_ESE, 96,0); // draw the compass arrow
  break;
  case 6:
    GLCD.DrawBitmap(Rose_SE, 96,0); // draw the compass arrow
  break;
  case 7:
    GLCD.DrawBitmap(Rose_SSE, 96,0); // draw the compass arrow
  break;
  case 8:
    GLCD.DrawBitmap(Rose_S, 96,0); // draw the compass arrow
  break;
  case 9:
    GLCD.DrawBitmap(Rose_SSW, 96,0); // draw the compass arrow
  break;
  case 10:
    GLCD.DrawBitmap(Rose_SW, 96,0); // draw the compass arrow
  break;
  case 11:
    GLCD.DrawBitmap(Rose_WSW, 96,0); // draw the compass arrow
  break;
  case 12:
    GLCD.DrawBitmap(Rose_W, 96,0); // draw the compass arrow
  break;
  case 13:
    GLCD.DrawBitmap(Rose_WNW, 96,0); // draw the compass arrow
  break;
  case 14:
    GLCD.DrawBitmap(Rose_NW, 96,0); // draw the compass arrow
  break;
  case 15:
    GLCD.DrawBitmap(Rose_NNW, 96,0); // draw the compass arrow
  break;

  }
  
}
