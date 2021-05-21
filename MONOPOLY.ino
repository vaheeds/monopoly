#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
//#include <string.h>
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
char firstUser = '*';
char secondUser = '*';
bool successRead = false;
byte readedCard;   // Stores first byte of scanned ID read from RFID Module
char *str;
uint8_t i = 0;
short m;
char s_money[5];
char tempNumber = '0';
short result = 0;
void setup() {
    Serial.begin(9600); // Initialize //Serial communications with the PC
    while (!Serial);    // Do nothing if no //Serial port is opened (added for Arduinos based on ATMEGA32U4)    
    pinMode(A0,OUTPUT);
    initHardWare();
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
void welcome()
{
  //Serial.println(F("welcome()"));
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(19, 10);
  display.print(F("MONOPOLY"));
  display.setCursor(10, 35);
  display.print(F("by Vaheed"));
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print(F(" 1 New"));
  display.setCursor(16, 38);
  display.println(F(" 2 Load"));
  display.display();
 }
void setNewPlayers()
{
  //Serial.println("setNewPlayers()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(" Players ?");
  display.println("   2     ");
  display.println("     3   ");
  display.println("       4 ");
  display.display();
  m = 0; // holds init money
  playerCount = myKeypad.waitForKey();
  //EEPROM.put(countAdr, playerCount);
  //Serial.print("Player Count: "));
  //Serial.println((char)playerCount);
  switch(playerCount)
  {
    case '2':
      m = getMoney(0);  
      while(filledUsersCount() < 2)
         setInitMoney(m);
    break;
    case '3':
      m = getMoney(0); 
      while(filledUsersCount() < 3)
         setInitMoney(m);
    break;
    case '4':
      m = getMoney(0); 
      while(filledUsersCount() < 4)
         setInitMoney(m);
    break;
    default:
      printLCD("ERROR...");
      delay(1000);
      setNewPlayers(); // in case of invalid input repeat the menu
    break;
  }
}
void flashLed()
{
  digitalWrite(A0,HIGH);
  delay(50);
  digitalWrite(A0,LOW);
}
void beepUp()
{
  tone(A2, 800);
  delay(100);
  tone(A2, 1000);
  delay(100);
  noTone(A2);
}
void beepDown()
{
  tone(A2, 600);
  delay(100);
  tone(A2, 800);
  delay(100);
  noTone(A2);
}
void addMoney(char user, short money)
{
  //Serial.print("Add Money:"));
  //Serial.print(user);
  //Serial.print(" = "));
  //Serial.println(money);
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
      saveNaveed();
    break;
    case 'R':
      rMoney = rMoney + money;
      saveRoya();
    break;
    case 'S':
      sMoney = sMoney + money;
      saveSadaf();
    break;
    case 'V':
      vMoney = vMoney + money;
      saveVaheed();
    break;   
  }
}
void setInitMoney(short money)
{
  printLCD("Set Money  Scan Card"); 
  delay(10);
  readCard();
  beepDown();
  //Serial.print("Set Money: "));
  //Serial.print(user());
  //Serial.print(" = "));
  //Serial.println(money);
  switch(user())
  {
    case 'N':
      nMoney = money;
      saveNaveed();
      setUsers[0]= true;
    break;
    case 'R':
      rMoney = money;
      saveRoya();
      setUsers[1]= true;
    break;
    case 'S':
      sMoney = money;
      saveSadaf();
      setUsers[2]= true;
    break;
    case 'V':
      vMoney = money;
      saveVaheed();
      setUsers[3]= true;
    break;
  }
  refreshScreen();
  delay(1000);
}
void play()
{
  //Serial.println("Play: wait to read card..."));
  readCard();
  beepUp();
  m = getMoney(0);    
  lastMoney = m;
  addMoney(user() , -1* m);
  printLCD("Scan Card             +");
  printMoney(m);
  display.display();
  digitalWrite(A0,HIGH);
  readCard();
  beepDown();
  digitalWrite(A0,LOW);
  addMoney(user() , m);
  saveLastTransaction();
  refreshScreen();
  play();   
}
void refreshScreen()
{  
  //Serial.println("Refresh Screen..."));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.write(firstUser);
  display.print(F("=>"));
  printMoney(lastMoney);
  display.print(F("=>"));
  display.write(secondUser);
     
  display.setCursor(0, 22);
  display.write('V');
  printMoney(vMoney);
  //display.print(cstr);   
  
  display.setCursor(68, 22);
  display.write('S');
  printMoney(sMoney);
  //display.print(cstr);   
 
  display.setCursor(0, 45);
  display.write('R');
  printMoney(rMoney);
  //display.print(cstr);
  
  display.setCursor(68, 45);
  display.write('N');
  printMoney(nMoney);
  //display.print(cstr);   
  
  display.display();
}
void initHardWare()
{  
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //Serial.println("init()"));
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    //Serial.println("SSD1306 allocation failed"));
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
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get //Serial and continue
    return 0;
  }
  //Serial.print("Scanned PICC's UID: "));
  readedCard = mfrc522.uid.uidByte[0];
  //Serial.print(readedCard, HEX);
  //Serial.println(""));
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
  //Serial.println("newGame()"));
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
  //Serial.println("loadGame()"));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawRoundRect(0, 0, 128, 64,16, SSD1306_WHITE);
  display.setCursor(20, 16);
  display.print("  Load");
  display.setCursor(16, 38);
  display.println("  Game");
  display.display();
  delay(1500);
  loadPlayers();
  play();
}
void saveVaheed()
{
  EEPROM.put(0, vMoney);
}
void saveSadaf()
{
  EEPROM.put(2, sMoney);
}
void saveRoya()
{
  EEPROM.put(4, rMoney);  
}
void saveNaveed()
{
  EEPROM.put(6, nMoney);
}
void saveLastTransaction()
{
  EEPROM.put(8, lastMoney);
  EEPROM.put(10, firstUser);
  EEPROM.put(11, secondUser);
}
void loadPlayers()
{
  EEPROM.get(0,vMoney);
  //Serial.println(vMoney);
  EEPROM.get(2,sMoney);
  //Serial.println(sMoney);
  EEPROM.get(4,rMoney);
  //Serial.println(rMoney);
  EEPROM.get(6,nMoney);
  //Serial.println(nMoney);
  EEPROM.get(8,lastMoney);
  //Serial.println(lastMoney);
  firstUser = EEPROM.read(10);
  //Serial.println(firstUser);
  secondUser = EEPROM.read(11);
  //Serial.println(secondUser);
  refreshScreen();
}
void printLCD(char *s)
{
  //Serial.println(s);
  display.clearDisplay();
  display.setCursor(0, 0);
  i = 0;
  while(s[i] != '\0'){
    display.write(s[i]);
    i++;
  }  
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
void printMoney(short m)
{
  s_money[0] = '\0';
  itoa(m,s_money,10);
  i = 0;
  while(s_money[i] != '\0'){
    display.write(s_money[i]);
    i++;
  }  
}
short getMoney(short init)
{   
  result = init > 9999 || init < 0 ? 0 : init;
  display.clearDisplay();
  display.setCursor(0, 0);
  printLCD(" Enter $$$              ");
  printMoney(result);
  //Serial.println(result);
  display.display();
  tempNumber = myKeypad.waitForKey();
  flashLed();
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
        return result;
    break;
  }
}
