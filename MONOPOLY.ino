//#include <EEPROM.h>
//#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <string.h>
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

//byte buffer1[18]={0x00};
//byte len = sizeof(buffer1);
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
bool setUsers[4] = {false, false, false, false};
//const int rAdr = 0;
//const int nAdr = 4;
//const int sAdr = 8;
//const int vAdr = 12;
//const int countAdr = 16;
char firstUser = '*';
char secondUser = '*';
//uint8_t scannedCounter = 0;
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
  Serial.print(F("Player Count: "));
  Serial.println((char)playerCount);
  switch(playerCount)
  {
    case '2':
      m = getMoney(0);  
      while(filledUsersCount() < 2)
         setMoney(m);
    break;
    case '3':
      m = getMoney(0); 
      while(filledUsersCount() < 3)
         setMoney(m);
    break;
    case '4':
      m = getMoney(0); 
      while(filledUsersCount() < 4)
         setMoney(m);
    break;
    default:
      printLCD(F("ERROR..."));
      delay(1000);
      setNewPlayers(); // in case of invalid input repeat the menu
    break;
  }
}
void addMoney(char user, short money)
{
  Serial.print(F("Add Money:"));
  Serial.print(user);
  Serial.print(F(" = "));
  Serial.println(money);
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
}
void setMoney(short money)
{
  printLCD(F("Set Money  Scan Card")); 
  delay(10);
  readCard();
  Serial.print(F("Set Money: "));
  Serial.print(user());
  Serial.print(F(" = "));
  Serial.println(money);
  switch(user())
  {
    case 'N':
      //EEPROM.put(nAdr, money);
      nMoney = money;
      setUsers[0]= true;
    break;
    case 'R':
      //EEPROM.put(rAdr, money);
      rMoney = money;
      setUsers[1]= true;
    break;
    case 'S':
      //EEPROM.put(sAdr, money);
      sMoney = money;
      setUsers[2]= true;
    break;
    case 'V':
      //EEPROM.put(vAdr, money);
      vMoney = money;
      setUsers[3]= true;
    break;
  }
  refreshScreen();
  delay(1000);
}
void play()
{
  Serial.println(F("Play: wait to read card..."));
  readCard();
  short m = 0;               
  m = getMoney(0);    
  lastMoney = m;
  addMoney(user() , -1* m);
  printLCD(F("Scan        Second     Card..."));
  readCard();
  addMoney(user() , m);
  refreshScreen();
  play();   
}
void refreshScreen()
{  
  Serial.println(F("Refresh Screen..."));
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
  
  display.print(F("N"));
  s_money = formatMoney(nMoney);
  display.print(s_money);   
  
  display.display();
}
void initDisplay()
{  
  Serial.println("initDisplay()");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("SSD1306 allocation failed");
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
void initRFID()
{
  Serial.println(F("initRFID()"));
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  //for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}
void welcome()
{
  Serial.println("welcome()");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(19, 10);
  display.print("MONOPOLY");
  display.setCursor(10, 35);
  display.print("by Vaheed");
  display.display();
  delay(4000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print(" 1 New");
  display.setCursor(16, 38);
  display.println(" 2 Load");
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
  Serial.print(F("Scanned PICC's UID: "));
  readedCard = mfrc522.uid.uidByte[0];
  Serial.print(readedCard, HEX);
  Serial.println(F(""));
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
  Serial.println("newGame()");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print("  New");
  display.setCursor(16, 38);
  display.println("  Game");
  display.display();
  delay(1500);
  setNewPlayers();
  play();
}
void loadGame()
{ 
  Serial.println(F("loadGame()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print("  Load");
  display.setCursor(16, 38);
  display.println("  Game");
  display.display();
  delay(2000);
  printLCD(F("...wip..."));
  //play();
}

void printLCD(String s)
{
  Serial.println(s);
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
uint8_t filledUsersCount()
{
  uint8_t temp = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    if(setUsers[i] == true)
      temp++;
  }
  return temp;
}

String formatMoney(short m)
{
  String s_money = String(m);
  while(s_money.length() < 4){
    s_money = " " + s_money;
  }
  return s_money;
}
short getMoney(short init)
{  
  if(init>9999) 
    return;
  short result = init;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(" Enter $$$");
  display.println();
  display.print("  ");
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
