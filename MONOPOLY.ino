//#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
#define RST_PIN         9    
#define SS_PIN          10    
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define USER_BLOCK     4
// LCD
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID Card 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

byte buffer1[18]={0x00};
byte len = sizeof(buffer1);
// Keypad
const byte ROWS = 4;
const byte COLS = 3;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {8, 7, 6, 5};
byte colPins[COLS] = {4, 3, 2};
Keypad myKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Game
byte playerCount = '0';
short rMoney = 0;
short nMoney = 0;
short sMoney = 0;
short vMoney = 0;
short lastMoney = 0;
const int rAdr = 0;
const int nAdr = 4;
const int sAdr = 8;
const int vAdr = 12;
const int countAdr = 16;
char firstUser = '*';
char secondUser = '*';
uint8_t scannedCounter = 0;
bool successRead = false;
byte readedCard;   // Stores first byte of scanned ID read from RFID Module

void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)    
    initRFID();
    initDisplay();
    welcome();
}
void loop() 
{
  char myKey = myKeypad.waitForKey();
  if (myKey)
  {
    switch (myKey)
    {
      case '1':        
        newGame();
      break;
      case '2':
        loadGame();
      break;
    }    
  }
}
void initRFID()
{
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}
void initDisplay()
{  
  Serial.println(F("initDisplay()"));
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(500);
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);   
}
void welcome()
{
  Serial.println(F("welcome()"));
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(19, 10);
  display.print(F("MONOPOLY"));
  display.setCursor(10, 35);
  display.print(F("by Vaheed"));
  display.display();
  delay(5000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print(F(" 1 New"));
  display.setCursor(16, 38);
  display.println(F(" 2 Load"));
  display.display();
 }
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  Serial.println(F("Scanned PICC's UID:"));
  readedCard = mfrc522.uid.uidByte[0];
  Serial.print(readedCard, HEX);
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}
void readCard()
{
  do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
}

void newGame()
{
  Serial.println(F("newGame()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print(F("  New"));
  display.setCursor(16, 38);
  display.println(F("  Game"));
  display.display();
  delay(1500);
  setNewPlayers();
  display.setCursor(0, 0);
  display.println(F("           "));
  display.setCursor(0, 0);
  display.print(F("Press #"));
  display.display();
  play();
}
void loadGame()
{
  vMoney = sMoney = rMoney = nMoney = 0;
  Serial.println(F("loadGame()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print(F("  Load"));
  display.setCursor(16, 38);
  display.println(F("  Game"));
  display.display();
  delay(2000);
  printLCD("...wip...");
  //play();
}
void printLCD(String s)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(s);
  display.display();
}
char user()
{
  switch(readedCard){
    case 0x47:
      return 'B';
    break;
    case 0x07:
      return 'V';
    break;
    case 0xA7:
      return 'N';
    break;
    case 0xD7:
      return 'R';
    break;
    case 0xC7:
      return 'S';
    break;
    default:
      return 'U';
    break;
  }  
}
void setMoney(short money)
{
  readCard();
  Serial.print(F("Set Money:"));
  Serial.println(user());
  switch(user())
  {
    case 'N':
      //EEPROM.put(nAdr, money);
      nMoney = money;
      scannedCounter ++;
    break;
    case 'R':
      //EEPROM.put(rAdr, money);
      rMoney = money;
      scannedCounter++;
    break;
    case 'S':
      //EEPROM.put(sAdr, money);
      sMoney = money;
      scannedCounter++;
    break;
    case 'V':
      //EEPROM.put(vAdr, money);
      vMoney = money;
      scannedCounter++;
    break;
  }
  delay(100);
  refreshScreen();
}
void addMoney(char user, short money)
{
  Serial.print(F("Add Money:"));
  Serial.println(user);
  if(money<0){
    firstUser = user;
    secondUser = '*';
  }
  if(money>0){
    secondUser = user;
  }
  switch(user)
  {
    case 'N':
      nMoney = nMoney + money;
      //EEPROM.put(nAdr, nMoney); 
    break;
    case 'R':
      rMoney = rMoney + money;
      //EEPROM.put(rAdr, rMoney); 
    break;
    case 'S':
      sMoney = sMoney + money;
      //EEPROM.put(sAdr, sMoney); 
    break;
    case 'V':
      vMoney = vMoney + money;
      //EEPROM.put(vAdr, vMoney); 
    break;   
  }    
  delay(100);  
  refreshScreen();
}
void setNewPlayers()
{
  Serial.println(F("setNewPlayers()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F(" Players ?"));
  display.println(F("   2     "));
  display.println(F("     3   "));
  display.println(F("       4 "));
  display.display();
  short m = 0; // holds init money
  playerCount = myKeypad.waitForKey();
  //EEPROM.put(countAdr, playerCount);
  Serial.println((char)playerCount);
  m = getMoney(0);    
  switch(playerCount)
  {
    case '2':
      printLCD("Scan Cards");
      while(scannedCounter < 2)
         setMoney(m);
      delay(1000);
    break;
    case '3':
      printLCD("Scan Cards");
      while(scannedCounter < 3)
         setMoney(m);
      delay(1000);
    break;
    case '4':
      printLCD("Scan Cards");
      while(scannedCounter < 4)
         setMoney(m);
      delay(1000);
    break;
    default:
      printLCD(F("ERROR..."));
      delay(1000);
      setNewPlayers(); // in case of invalid input repeat the menu
    break;
  }
}
short getMoney(short init)
{  
  if(init>9999) 
    return;
  short result = init;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F(" Enter $$$"));
  display.println();
  display.print(F("  "));
  display.print(result);
  Serial.println(result);
  display.display();
  char tempNumber = myKeypad.waitForKey();
  switch(tempNumber)
  {
    case '0':      
      result = (result * 10);
      getMoney(result);
    break;
    case '1':
      result = (result * 10) + 1;
       getMoney(result);
    break;
    case '2':
      result = (result * 10) + 2;
       getMoney(result);
    break;
    case '3':
      result = (result * 10) + 3;
       getMoney(result);
    break;
    case '4':
      result = (result * 10) + 4;
       getMoney(result);
    break;
    case '5':
      result = (result * 10) + 5;
       getMoney(result);
    break;
    case '6':
      result = (result * 10) + 6;
       getMoney(result);
    break;
    case '7':
      result = (result * 10) + 7;
       getMoney(result);
    break;
    case '8':
      result = (result * 10) + 8;
       getMoney(result);
    break;
    case '9':
      result = (result * 10) + 9;
       getMoney(result);
    break;
    case '*':
      result = (result / 10);
       getMoney(result);
    break;
    case '#':
      if(result > 9999)
      {
        result = 0;
        getMoney(result);
      }
      else
        return result;
    break;
  }
}
void refreshScreen()
{  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(firstUser);
  display.print(F("=>"));
  display.print(lastMoney);
  display.print(F("=>"));
  display.print(secondUser);
  String s_money;  
     
  display.setCursor(0, 22);
  display.print(F("V"));
  s_money =  formatMoney(vMoney);
  display.print(s_money);   
  
  display.setCursor(68, 22);
  display.print(F("S"));
  s_money =  formatMoney(sMoney);
  display.print(s_money);   
 
  display.setCursor(0, 45);
  display.print(F("R"));
  s_money =  formatMoney(rMoney);
  display.print(s_money);        
  
  display.setCursor(68, 45);
  display.print(F("N"));
  s_money = formatMoney(nMoney);
  display.print(s_money);   
  
  display.display();
}
String formatMoney(short m)
{
  String s_money = String(m);
  while(s_money.length() < 4){
    s_money = " " + s_money;
  }
  return s_money;
}
void play()
{
  Serial.println(F("wait read card"));
  readCard();
  short m = 0;               
  m = getMoney(0);    
  lastMoney = m;
  addMoney(user() , -1* m);
  printLCD(F("Second      Card..."));
  readCard();
  addMoney(user() , m);
  play();   
}
//
//void testIntToCard(short money){
//  String s = String(money);
//  while(s.length() < 4){
//    s = " " + s;
//  }
//  for(uint8_t i=1; i<= 4; i++){
//    buffer1[i]=(byte)s[i-1];
//  }
//  String p = "";
//  for(uint8_t i=0; i< 16; i++){
//    p += (char)buffer1[i];
//  }
//  p.trim();
//  short after = (short)(p.substring(1,5)).toInt();
//  Serial.println(s);
//  Serial.println(p);
//  Serial.println(after);
//}
