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
String s_money;
char user;
byte moneyTemp[4]={0x20,0x20,0x20,0x20};

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
void readCard()
{
  Serial.println(F("readCard()"));
  while(1)
  {  
    if(!mfrc522.PICC_IsNewCardPresent())
      continue;
    if(!mfrc522.PICC_ReadCardSerial())
      continue;
    Serial.println(F("**Card Detected:**"));

    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, USER_BLOCK, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      continue;
    }
  
    status = mfrc522.MIFARE_Read(USER_BLOCK, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      continue;
    }
    delay(200);  
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  } 
}
void writeCard() 
{
  Serial.println(F("writeCard()"));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, USER_BLOCK, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }  
  // Write block
  status = mfrc522.MIFARE_Write(USER_BLOCK, buffer1, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  delay(200);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
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
  play();
}
void loadGame()
{
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
}
void printLCD(String s)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(s);
  display.display();
}
void setMoneyToCard(short money)
{
  readCard();
  user = (char)buffer1[0];
  String s = String(money);
  while(s.length() < 4){
    s = " " + s;
  }
  for(uint8_t i=1; i<= 4; i++){
    buffer1[i]=(byte)s[i-1];
  }
  writeCard();
  placeToScreen(user,s);    
  delay(200);
}
void addMoneyToCard(short money)
{
    
  readCard();
  user = (char)buffer1[0];
  //short userMoney = 
  s_money.getBytes(moneyTemp,4);
  s_money = String(money);  
  buffer1[1] = moneyTemp[0];
  buffer1[2] = moneyTemp[1];
  buffer1[3] = moneyTemp[2];
  buffer1[4] = moneyTemp[3];
  writeCard();
  placeToScreen(user,s_money);    
  delay(200);
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
  Serial.println(playerCount);
  m = getMoney(0);    
    
  if (playerCount == '2') // valid players count
  {  
    printLCD("Scan Cards");
    setMoneyToCard(m);
    setMoneyToCard(m);
  }
  else if (playerCount == '3') // valid players count
  {   
    printLCD("Scan Cards");
    setMoneyToCard(m);
    setMoneyToCard(m);
    setMoneyToCard(m);
  }
  else if (playerCount == '4') // valid players count
  {   
    printLCD("Scan Cards");
    setMoneyToCard(m);
    setMoneyToCard(m);
    setMoneyToCard(m);
    setMoneyToCard(m);
  }
  else
    setNewPlayers(); // in case of invalid input repeat the menu
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
void placeToScreen(char user,String s_money)
{
  // while(s_money.length() < 4)
  //   s_money = " " + s_money;
  switch(user)
  {
    case 'V':      
      display.setCursor(0, 22);
      display.print(user);
      display.print(s_money);      
    break;
    case 'S':      
      display.setCursor(68, 22);
      display.print(user);
      display.print(s_money);      
    break;
    case 'R':      
      display.setCursor(0, 45);
      display.print(user);
      display.print(s_money);      
    break;
    case 'N':      
      display.setCursor(68, 45);
      display.print(user);
      display.print(s_money);      
    break;
  }
  display.display();
}
void play()
{
  short m = 0;
  m = getMoney(0);    
  setMoneyToCard(-1* m);
  setMoneyToCard(m);
}

void testIntToCard(short money){
  String s = String(money);
  while(s.length() < 4){
    s = " " + s;
  }
  for(uint8_t i=1; i<= 4; i++){
    buffer1[i]=(byte)s[i-1];
  }
  String p = "";
  for(uint8_t i=0; i< 16; i++){
    p += (char)buffer1[i];
  }
  p.trim();
  short after = (short)(p.substring(1,5)).toInt();
  Serial.println(s);
  Serial.println(p);
  Serial.println(after);
}
