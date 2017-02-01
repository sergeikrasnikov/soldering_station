// include the library code:
#include <LiquidCrystal.h>
/*
 The circuit:
 * LCD RS pin to digital pin D12
 * LCD Enable pin to digital pin D11
 * LCD D4 pin to digital pin A5
 * LCD D5 pin to digital pin A4
 * LCD D6 pin to digital pin A3
 * LCD D7 pin to digital pin A2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K potentiometer:  ends to +5V and ground and wiper to LCD VO pin (pin 3)
 */
 // initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, A5, A4, A3, A2);

//Encoder's pins
int pinA = 3;  // Connected to D3  (Rotation)
int pinB = 4;  // Connected to D4 (Rotation)
int SW = 5; // Connected to D5 (Pressing)

const int sIThermoPin = A0;  // the analog input pin for reading an amplified voltage of the soldering iron's thermocouple 
const int hGThermoPin = A1;  // the analog input pin for reading an amplified voltage of the heat gun's thermocouple
const int sIHeatingElementPin = 8; // the output pin for controlling the soldering iron's heating element
const int hGFanPWMPin = 9;  // the output pin for controlling the heat gun's fan
const int hGHeatingElementPin = 7;  // the output pin for controlling the heat gun's heating element 

int sIEncoderPosCount = 0; 
int hGEncoderPosCount = 0;

int fanEncoderPosCount = 255; 
int fan_pwm_value = 255 ;

int buttonLast=HIGH;
int buttonMenuPos=0;
 
int pinALast;  
int aVal;
boolean bCW;

int sIThermo = 0;        
int outputValue = 0;        


int hGThermo = 0;        


const int P_coef = 4;
const int displayRedrawCount = 100;
int cyclesCount = 0;


 void setup() { 
   pinMode (pinA,INPUT_PULLUP); 
   pinMode (pinB,INPUT_PULLUP); 
   pinMode (SW,INPUT_PULLUP);
   pinMode (hGHeatingElementPin,OUTPUT);
   pinMode (sIHeatingElementPin,OUTPUT);
   /* Read Pin A
   Whatever state it's in will reflect the last position   
   */
   pinALast = digitalRead(pinA);   
   Serial.begin (115200);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

 } 

 void loop() { 
   
   // 2 step calib:
   // 1) {Xi set temperature, Yi temperature measured by an external device} :   linear fit  {80,48},{100,62},{160,92},{200,123},{300,197}
   // this gives us  0.672409 * x-8.56477,   [1] 
   // 2) {Xi set temperature, Yi temperature measured by an external device} :  linear fit  {100,136},{150,205},{200,268},{222,300}
   // this gives us 1.33191 * x + 3.48942,  [2]
   // Let's substitute [1] into [2]:  3.48942 + 1.33191 * (0.672409 * x-8.56477)  and then let's simplify => finally we have (0.895588 * x -7.91808) , [3]
   sIThermo = analogRead(sIThermoPin); // Reads the value from the specified analog pin, ADC will map input voltages between 0V and 5V  into integer values between 0 and 1023
   sIThermo = (int) (0.895588 *sIThermo -7.91808); // use [3]

   // {Xi set temperature, Yi temperature measured by an external device}:  linear fit {40,52},{80,95},{100,125},{160,195}
   //1.202 * x+2.56
   hGThermo = analogRead(hGThermoPin); // Reads the value from the specified analog pin, ADC will map input voltages between 0V and 5V  into integer values between 0 and 1023
   hGThermo = (int)(1.202*hGThermo+2.56); 

   int i;

   //thermostats are implemented by using an on-off controller

   //thermostat 1
   i = sIEncoderPosCount - sIThermo;
   i = i > 0 ? i : 0;
   
   int sI_out;
   
   if (i) 
     sI_out = HIGH;
   else
     sI_out = LOW;
   
   digitalWrite(sIHeatingElementPin,sI_out);

   //thermostat 2
   i = hGEncoderPosCount - hGThermo;
   i = i > 0 ? i : 0;
   int hG_out;
   if (i) 
     hG_out = HIGH;
   else
     hG_out = LOW;

   digitalWrite(hGHeatingElementPin,hG_out);

   //setting fan speed
   i = fanEncoderPosCount;
   i = i > 0 ? i : 0;
   i = i < 255 ? i : 255;

   fan_pwm_value = i;
   analogWrite(hGFanPWMPin,fan_pwm_value);


 // processing encoder signals
   aVal = digitalRead(pinA);
   if (aVal != pinALast){ // Means the knob is rotating
     // if the knob is rotating, we need to determine direction
     // We do that by reading pin B.
     if (digitalRead(pinB) != aVal) {  // Means pin A Changed first - We're Rotating Clockwise
       switch(buttonMenuPos)
       {
        case 0:
         sIEncoderPosCount += 10;
         break;
        case 1:
         hGEncoderPosCount += 10;
         break;
        case 2:
         fanEncoderPosCount += 10;
         break;
       }
       bCW = true;
     } else {// Otherwise B changed first and we're moving CCW
       bCW = false;
       switch(buttonMenuPos)
       {
        case 0:
         sIEncoderPosCount -= 10;
         break;
        case 1:
         hGEncoderPosCount -= 10;
         break;
        case 2:
         fanEncoderPosCount -= 10;
         break;
       }
     }
   } 

   int buttonStatus = digitalRead(SW);
   if ( buttonStatus == LOW && buttonStatus != buttonLast )
   {
    Serial.println ("Pushbutton");
    buttonMenuPos = buttonMenuPos == 2 ? 0 : buttonMenuPos+1 ;
    
   }
       

   
buttonLast = buttonStatus ;





char str[16];

char sISeparator = '-';
char hGSeparator = '-';
char fanSeparator = '=';

//refresh display and serial output 
if (cyclesCount++ % displayRedrawCount == 0) {  // skips some cycles, otherwise output processing will block input processing

      Serial.print("sIenc = ");
      Serial.print(sIEncoderPosCount);
      Serial.print(" sIThermo = ");
      Serial.print(sIThermo);
      Serial.print(" sI_out = ");
      Serial.print(sI_out);
      Serial.print(" hG_out = ");
      Serial.print(hG_out);
      Serial.print(" hGenc = ");
      Serial.print(hGEncoderPosCount);
      Serial.print(" hGThermo = ");
      Serial.print(hGThermo);
      Serial.print(" fanEnc = ");
      Serial.print(fanEncoderPosCount);
      Serial.print(" hgFan = ");
      Serial.print(fan_pwm_value);
      Serial.print(" buttonMenuPos = ");
      Serial.println(buttonMenuPos);
      
      switch (buttonMenuPos)
      {
        case 0:
         sISeparator = '*'; break;
        case 1:
         hGSeparator = '*'; break;
        case 2:
         fanSeparator = '*'; break;
      }
            
      sprintf(str, "S=%3d%c%3d       ",sIThermo,sISeparator,sIEncoderPosCount);
      lcd.setCursor(0, 0);
      lcd.print(str);
      sprintf(str, "H=%3d%c%3d F%c%3d%%",hGThermo,hGSeparator,hGEncoderPosCount,fanSeparator,fan_pwm_value*100/255);
      lcd.setCursor(0, 1);
      lcd.print(str);
}


pinALast = aVal;


 } 

