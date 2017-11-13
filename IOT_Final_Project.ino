#include "SPI.h"
#include "MFRC522.h"
#include "Servo.h"
#include <GSM.h>

#define SS_PIN 10
#define RST_PIN 9
#define SP_PIN 8

// Hardware setup:
// VCC to 3.3V
// RST to D9
// GND to GND
// MISO to D12
// MOSI to D11
// SCK to D13
// NSS to D10

// Servo:
// Brown: GND
// Red: 5V
// Orange: D6


MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

Servo servo;
int pos = 0;

GSM gsmAccess;
GSM_SMS sms;
char remoteNumber[20]= "07449976201";
char txtMsg[200]="Feeder has been used";

bool needToSend = false;
int messageSent = 0;

void setup() {
  // RFID setup
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Servo Setup
  servo.attach(6);
  servo.write(10);

  // GSM Setup 
  boolean notConnected = true;
      while(notConnected){
    if(gsmAccess.begin()==GSM_READY){
     notConnected = false;
    }
  else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  Serial.println("GSM Connected");
  
}

void loop() {

  // If tag is not near reader, close lid
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()){
    servo.write(10);
    delay(200);
    

// Send SMS once lid has closed and SMS has not already been sent. 
    if (needToSend) {
      sendSMS();
      needToSend = false;
    }

    return;
  }
  
  

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
 // Assigns UID to string
  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();
  Serial.println(strID);


  
// Have to call this twice as occasionally get a false 0 returned. 
rfid.PICC_IsNewCardPresent();
bool present = rfid.PICC_IsNewCardPresent();

// While tag is present, keep lid open 
while (present==1){
  rfid.PICC_IsNewCardPresent();
  present = rfid.PICC_IsNewCardPresent();
  

// Tag 56:52:91:BB
// Card B0:9D:5F:7E
 if (strID.indexOf("56:52:91:BB") >= 0) {
 
   // Change this depending on how far you want lid to open. 
         servo.write(160);
        delay(2000);
        needToSend = true;

  } 
}


  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}


void sendSMS(){
  Serial.println("Sending SMS");
sms.beginSMS(remoteNumber);
  sms.print(txtMsg);
  sms.endSMS();
  Serial.println("Message sent");
  sms.flush();
  

}
