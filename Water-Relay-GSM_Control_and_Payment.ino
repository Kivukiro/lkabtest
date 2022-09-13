// Include required libraries
//#include <NewSoftSerial.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <I2CKeyPad.h>
#include <Wire.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <SPI.h> 
#include <LiquidCrystal_I2C.h>//initialize the library with the numbers of the interface pins
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//SoftwareSerial lcd(scl,sda);
SoftwareSerial /*SIM900*/GSM(4, 5); // SoftwareSerial A6(U_Rxd, Tx)
MFRC522 mfrc522(10, 9); // MFRC522 mfrc522(SS_PIN, RST_PIN)
//LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo sg90;
// Initialize Pins for led's, servo and buzzer
// Blue LED is connected to 5V
constexpr uint8_t redLed = 6;
constexpr uint8_t buzzerPin = 7;
//#define relay 8 
constexpr uint8_t relay = 8; // relay pin
//#define pulse_in 2 //pin sensor inpt or sensorInterrupt
char initial_password[4] = {'1', '2', '3', '4'};  // Variable to store initial password
String tagUID = "19 35 DA B3,1B 28 C5 14";  // String to store UID of tag. Change it with your tag's UID
char password[4];   // Variable to store users password
boolean RFIDMode = true; // boolean to change modes
boolean NormalMode = true; // boolean to change modes
char key_pressed = 0; // Variable to store incoming keys
uint8_t i = 0;  // Variable used for counter
// defining how many rows and columns our keypad have
const byte rows = 4;
const byte columns = 4;
// Keypad pin map
char hexaKeys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Initializing pins for keypad
byte row_pins[rows] = {A0, A1, A2, A3};
byte column_pins[columns] = {2,1, 0};
// Create instance for keypad
Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);

//////////////////////////////////////////////////////////////////
//water flow sensor
byte sensorInterrupt = 0;  // 0 = digital pin 2
float calibrationFactor = 7.5;
volatile byte pulseCount;  //pulse
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char inchar; // Will hold the incoming character from the GSM shield
int unt_a=0, unt_b=0, unt_c=0, unt_d=0;
long total_unt =7;
int price = 0;
long price1 =0;
int Set = 10; // Set the unit price


//change +250 with country code and xxxxxxxxx with phone number to sms
String phone_no1 = "+250"; //Enter Consumer number
String phone_no2 = "+250789910258"; //Enter Company number

int flag1=0, flag2=0, flag3=0;

void setup() { 
///////////////////////////////////RFID_KEYPAD_RELAY/////////////////////////////////
  // Arduino Pin configuration
  sg90.attach(relay);  //Declare pin 8 for servo
  sg90.write(0); // Set initial position at 0 degrees
  lcd.begin(20,4);   // LCD screen
  lcd.backlight();
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  // Arduino communicates with A6/GSM shield at a baud rate of 19200
  // Make sure that corresponds to the baud rate of your module
  /*SIM900*/GSM.begin(19200);
  // AT command to set A6 to SMS mode
  /*SIM900*/GSM.print("AT+CMGF=1\r");
  delay(100);
  // Set module to send SMS data to serial out upon receipt
  /*SIM900*/GSM.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  lcd.clear(); // Clear LCD screen
////////////////////////////////////////////////////////////////////
Serial.begin(9600);
GSM.begin(9600);
pinMode(relay,OUTPUT); //digitalWrite(relay1, HIGH);

pinMode(sensorInterrupt,   INPUT);
 //A rising pulse from encodenren activated ai0(). AttachInterrupt 0 is DigitalPin nr 2 on moust Arduino.
 attachInterrupt(0, ai0, RISING);



 lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(5,0);
  lcd.print("WELCOME!");
  lcd.setCursor(3,1);
  lcd.print("WATER METER!");
   lcd.setCursor(2,3);
  lcd.print("SYSTEM!");
Serial.println("Initializing....");
initModule("AT","OK",1000);
initModule("ATE1","OK",1000);
initModule("AT+CPIN?","READY",1000);  
initModule("AT+CMGF=1","OK",1000);     
initModule("AT+CNMI=2,2,0,0,0","OK",1000);  
Serial.println("Initialized Successfully"); 
delay(100);
sendSMS(phone_no1,"Welcome To Water Meter");
lcd.clear();

if(EEPROM.read(50)==0){}
else{Write();}

EEPROM.write(50, 0);

pulseCount = EEPROM.read(10);

Read();  
if(total_unt>0){
digitalWrite(relay, HIGH); 
}

}

void loop(){

  if (NormalMode == false) {
    // Function to receive message
    receive_message();
  }
  else if (NormalMode == true) {
    // System will first look for mode
    if (RFIDMode == true) {
      // Function to receive message
      receive_message();
      lcd.setCursor(0, 0);
      lcd.print("   valve Lock");
      lcd.setCursor(0, 1);
      lcd.print(" Scan Your Tag ");
      // Look for new cards
      if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return;
      }
      // Select one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial()) {
        return;
      }
      //Reading from the card
      String tag = "";
      for (byte j = 0; j < mfrc522.uid.size; j++)
      {
        tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
        tag.concat(String(mfrc522.uid.uidByte[j], HEX));
      }
      tag.toUpperCase();
      //Checking the card
      if (tag.substring(1) == tagUID)
      {
        // If UID of tag is matched.
        lcd.clear();
        lcd.print("Tag Matched");
        
        delay(3000);
        lcd.clear();
        lcd.print("Enter Password:");
        lcd.setCursor(0, 1);
        RFIDMode = false; // Make RFID mode false
      }
      else
      {
        // If UID of tag is not matched.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wrong Tag Shown");
        lcd.setCursor(0, 1);
        lcd.print("Access Denied");
        send_message("Someone Tried with the wrong tag \nType 'close' to halt the system.");
        delay(3000);
        lcd.clear();
      }
    }
    // If RFID mode is false, it will look for keys from keypad
    if (RFIDMode == false) {
      key_pressed = keypad_key.getKey(); // Storing keys
      if (key_pressed)
      {
        password[i++] = key_pressed; // Storing in password variable
        lcd.print("*");
      }
      if (i == 4) // If 4 keys are completed
      {
        delay(200);
        if (!(strncmp(password, initial_password, 4))) // If password is matched
        {
          lcd.clear();
          lcd.print("Pass Accepted");
         delay(3000);
//If the amount entered by the user they still an remider sg90 still open.
          lcd.clear();
          i = 0;
          RFIDMode = true; // Make RFID mode true
        }
        else    // If password is not matched
        {
          lcd.clear();
          lcd.print("Wrong Password");
          send_message("Someone Tried with the wrong Password \nType 'close' to halt the system.");
          delay(3000);
          lcd.clear();
          i = 0;
          RFIDMode = true;  // Make RFID mode true
        }
      }
    }
  }
//////////////////////////////////////////////////////////////////
//If a character comes in from the cellular module...
  if(GSM.available() >0){
  inchar=GSM.read(); 
  if(inchar=='R'){
  delay(10);
  inchar=GSM.read();
  if(inchar=='U'){
  delay(10);
  inchar=GSM.read();  
  if(inchar=='c'){
  delay(10);
  inchar=GSM.read();
  if(inchar=='o'){
  delay(10);
  inchar=GSM.read();
  if(inchar=='d'){
  delay(10);
  inchar=GSM.read();
  if (inchar=='e'){
  delay(10);
  inchar=GSM.read();
      if(inchar=='1'){price = 100 / Set;  total_unt = total_unt +price; 
      sendSMS(phone_no1,"Your Recharge is Update: 100");  
      sendSMS(phone_no2,"Your Recharge is Update: 100"); 
      load_on();
      }
 else if(inchar=='2'){price = 200 / Set;  total_unt = total_unt +price; 
      sendSMS(phone_no1,"Your Recharge is Update: 200");  
      sendSMS(phone_no2,"Your Recharge is Update: 200");
      load_on();
 }
 else if(inchar=='3'){price = 300 / Set;  total_unt = total_unt +price;
      sendSMS(phone_no1,"Your Recharge is Update: 300");  
      sendSMS(phone_no2,"Your Recharge is Update: 300");
      load_on();
 }
 else if(inchar=='4'){price = 400 / Set;  total_unt = total_unt +price;
      sendSMS(phone_no1,"Your Recharge is Update: 400");  
      sendSMS(phone_no2,"Your Recharge is Update: 400"); 
      load_on();
 }
      delay(10);
      }
     }
    }
   }
  }
 }
 
else if(inchar=='D'){
  delay(10);
  inchar=GSM.read(); 
  if(inchar=='a'){
  delay(10);
  inchar=GSM.read(); 
  if(inchar=='t'){
  delay(10);
  inchar=GSM.read();  
  if(inchar=='a'){Data();}
    }
   }
  } 
 }

lcd.setCursor(0, 0);
lcd.print("Unit:");  lcd.print(total_unt); lcd.print("   "); 

lcd.setCursor(0, 1);
lcd.print("Price:"); lcd.print(price1);   lcd.print("   ");

lcd.setCursor(11, 0);
lcd.print("Pulse"); 

lcd.setCursor(13, 1);
lcd.print(pulseCount); lcd.print("   ");

if(total_unt==5){
if(flag1==0){ flag1 = 1;   
sendSMS(phone_no1,"Your Balance is Low Please Recharge");  
 }  
}

if(total_unt==0){
digitalWrite(relay, LOW);  
if(flag2==0){ flag2 = 1;     
sendSMS(phone_no1,"Your Balance is Finish Please Recharge");   
 }  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
  //lcd.begin();
  lcd.print(" Flow Rate");
  lcd.setCursor(1, 1);
  lcd.print(flowRate);
  lcd.setCursor(6, 1);
  lcd.print("L/Min");
    
    

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void load_on(){
Write();
Read();    
digitalWrite(relay, HIGH);
flag1=0, flag2=0;
}

// Send SMS 
void sendSMS(String number, String msg){
GSM.print("AT+CMGS=\"");GSM.print(number);GSM.println("\"\r\n"); //AT+CMGS=”Mobile Number” <ENTER> - Assigning recipient’s mobile number
delay(500);
GSM.println(msg); // Message contents
delay(500);
GSM.write(byte(26)); //Ctrl+Z  send message command (26 in decimal).
delay(5000);  
}

void Data(){
 GSM.print("AT+CMGS=\"");GSM.print(phone_no1);GSM.println("\"\r\n"); 
 delay(1000);
 GSM.print("Unit:");GSM.println(total_unt);
 GSM.print("Price:");GSM.println(price1);
 delay(500);
 GSM.write(byte(26)); // (signals end of message)
 delay(5000);   
}

void Read(){
unt_a = EEPROM.read(1);
unt_b = EEPROM.read(2);
unt_c = EEPROM.read(3);
unt_d = EEPROM.read(4);  
total_unt = unt_d*1000+unt_c*100+unt_b*10+unt_a;  
price1 = total_unt*Set;
}

void Write(){  
unt_d = total_unt / 1000;
total_unt = total_unt - (unt_d * 1000);
unt_c = total_unt / 100;
total_unt = total_unt - (unt_c * 100);
unt_b = total_unt / 10;
unt_a = total_unt - (unt_b *10);  

EEPROM.write(1, unt_a);
EEPROM.write(2, unt_b);
EEPROM.write(3, unt_c);
EEPROM.write(4, unt_d);
}


void initModule(String cmd, char *res, int t){
while(1){
    Serial.println(cmd);
    GSM.println(cmd);
    delay(100);
    while(GSM.available()>0){
       if(GSM.find(res)){
        Serial.println(res);
        delay(t);
        return;
       }else{Serial.println("Error");}}
    delay(t);
  }
}
//////////////////////////////////////////water//////////////////////////////////

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

////////////////////////////////////////energie///////////////////////////////////////////////////////////////////

void ai0() {// ai0 is activated if DigitalPin nr 2 is going from LOW to HIGH
if(digitalRead(/*pulse_in*/sensorInterrupt)==1) {  
pulseCount =  pulseCount++; ////pulse counter increment
if( pulseCount++>9){ pulseCount=0;
if(total_unt>0){total_unt = total_unt-1;}   
Write();
Read();
  }
EEPROM.write(10, pulseCount);  
 }
}
////////////////////////////////////////////////////////////////////
// Receiving the message
void receive_message()
{
  char incoming_char = 0; //Variable to save incoming SMS characters
  String incomingData;   // for storing incoming serial data
  
  if (/*SIM900*/GSM.available() > 0)
  {
    incomingData = /*SIM900*/GSM.readString(); // Get the incoming data.
    delay(10);
  }
  // if received command is to open the door
  if (incomingData.indexOf("open") >= 0)
  {
    sg90.write(90);
    NormalMode = true;
    send_message("Opened");
    delay(10000);
    sg90.write(0);
  }
  // if received command is to halt the system
  if (incomingData.indexOf("close") >= 0)
  {
    NormalMode = false;
    send_message("Closed");
  }
  incomingData = "";
}
// Function to send the message
void send_message(String message)
{
  /*SIM900*/GSM.println("AT+CMGF=1");    //Set the GSM Module in Text Mode
  delay(100);
  /*SIM900*/GSM.println("AT+CMGS=\"+XXXXXXXXXXXX\""); // Replace it with your mobile number
  delay(100);
  /*SIM900*/GSM.println(message);   // The SMS text you want to send
  delay(100);
  /*SIM900*/GSM.println((char)26);  // ASCII code of CTRL+Z
  delay(100);
  /*SIM900*/GSM.println();
  delay(1000);
}
