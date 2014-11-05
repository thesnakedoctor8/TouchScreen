#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>             // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>            // this is needed for FT6206
#include <Adafruit_FT6206.h>
#include <EepromUtil.h>
#include <EEPROM.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <Keypad.h>

// Color Definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

// Current state definitions
#define MAIN_SCREEN           0
#define FREQUENCY_SCREEN      1
#define OUTPUT_SCREEN         2
#define FREQUENCY_SCREEN_TEXT 5

// Global variables
int deviceState = MAIN_SCREEN;    // stateof the device

//{"#00000001.00-0002.5-0"};

long frequency = 100;
double pkpkVoltage = 2.5;
double rmsVoltage = .884;
double dBm = -39.032;
int waveform = 0;
int usbConnected = 0;
float x = 0;
//char data[2];
char message[2]; 
boolean overload = false;
double currentTime = millis();
double previousTime = millis();
int multiplier = 1;

//"#10000000.01-2499.9-0"
// Send Data
String dataStr = String(22);
char data[22];
// Receive data
String readLine;
boolean signalData = false;
char readBuffer[22];
char freq[12];
char volt[7];
char wave[2];

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = {30, 32, 34, 36};
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = {38, 40, 42}; 
// Create the Keypad
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// String for holding the frequency typed
String keypadStr;// = String(9);
int keysPressed = 0;

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup(void)
{
  //while (!Serial);     // used for leonardo debugging
  Serial.begin(115200);
  Serial1.begin(115200);  
  
  // Start the display
  tft.begin();

  // pass in 'sensitivity' coefficient
  if(!ts.begin(40))
  {
    errorText();
    // Continually loop until device is reset
    while(1);
  }
  
  keypadStr = "";
  keysPressed = 0;

  if(EepromUtil::eeprom_read_string(0, readBuffer, 22))
  {
    stringToSignal();

    if(frequency < 100 || frequency > 1000000000)
    {
    	frequency = 100;
    }

    if(pkpkVoltage < 2.5 || pkpkVoltage > 2500)
    {
    	pkpkVoltage = 2.5;
        rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
        dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
    }
  }

  // Clear the screen
  clearScreen();
  // Load the main screen
  mainScreenView();
}

void loop()
{
  if(Serial.available())
  {
    // #10000000.01-2499.9-0
    readData();
    
    if(signalData)
    {
      stringToSignal();      
      sendDataToArduino();
      updateDisplayScreen();
      return;
    }
  }



  char key = kpd.getKey();
  if(key)  // Check for a valid key.
  {
    if(key == '#')
    {
      // Enter keypad string if already on the FREQUENCY_SCREEN_TEXT
      if(deviceState == FREQUENCY_SCREEN_TEXT)
      {
        if(keysPressed == 0)
        {
          return;
        }

        char keyFreq[12];
        keypadStr.toCharArray(keyFreq, 12);
        long tempFrequency = atol(keyFreq);
        tempFrequency *= 100;
        if(tempFrequency >= 100 && tempFrequency <= 1000000000)
        {
          frequency = tempFrequency;
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
          keypadTextMessage(140, GREEN, "Frequency Changed");
        }
        else
        {
          keypadTextMessage(140, RED, " Invalid Input!");
        }

        keypadStr = "";
        keysPressed = 0;
        keypadTextClear(100);
        keypadText(100);
        return;
      }
      
      // Go to FREQUENCY_SCREEN_TEXT if '#' pressed
      deviceState = FREQUENCY_SCREEN_TEXT;
      keypadStr = "";
      keysPressed = 0; 
      changeDisplayScreens();
      return;
    }

    // Only handle keypresses in the FREQUENCY_SCREEN_TEXT
    if(deviceState == FREQUENCY_SCREEN_TEXT)
    {
      keypadTextMessageClear(140);

      switch (key)
      {
        case '0':
          if(keysPressed != 0)
          {
            if(keysPressedFunction('0'))
            {
              keypadTextClear(100);
              keypadText(100);
            }
          }
          break;

        case '1':
          if(keysPressedFunction('1'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '2':
          if(keysPressedFunction('2'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '3':
          if(keysPressedFunction('3'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '4':
          if(keysPressedFunction('4'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '5':
          if(keysPressedFunction('5'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '6':
          if(keysPressedFunction('6'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '7':
          if(keysPressedFunction('7'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '8':
          if(keysPressedFunction('8'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '9':
          if(keysPressedFunction('9'))
          {
            keypadTextClear(100);
            keypadText(100);
          }
          break;

        case '*':
          keypadStr = "";
          keysPressed = 0;
          sendDataToArduino();
          sendDataToPC();
          keypadTextClear(100);
          keypadText(100);
          break;
      }
    }    
  }
  
  // Wait for a touch
  if(!ts.touched())
  {
    return;
  }
  
  // Retrieve a point  
  TS_Point p = ts.getPoint();
  // flip it around to match the screen.
  p.x = map(p.x, 0, 240, 240, 0);
  p.y = map(p.y, 0, 320, 320, 0);
  // Handle the touch event for the point pressed
  checkButtonPressed(p.x, p.y);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////                    View Layouts                       /////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void mainScreenView()
{
  headerBlock("Main Screen");
  frequencyBlock();
  outputLevelBlock(130);
  changeFrequencyTextButton(230);
  loadDefaults(270);
  //usbStatus(290);
  sinSquareBlock(230);
  //tablesBlock();
}

void changeFrequencyView()
{
  headerBlock("Change Frequency");
  frequencyText(51);
  incrementPanel(80);
  largeFrequencyIncrements(150);
  presetFrequencies(130);
  multiplier = 1;
  multiplierBlock(200);
  backButton();
}

void changeFrequencyTextView()
{
  headerBlock("Enter Frequency");
  frequencyText(51);
  keypadTextBlock(100);
  button(40, 220, 1, "Change Frequency Screen");
  keypadDescriptionBlock(170);
  button(164, 275, 2, "SAVE");
  backButton();
}

void changeOutputView()
{
  headerBlock("Change Output");
  peakToPeakBlock();
  rmsBlock();
  dbmBlock();
  backButton();
}









/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////                  Block Components                     /////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// ************************************************
// *******************Main Block*******************
// ************************************************

void frequencyBlock()
{
  frequencyText(51);
  button(15, 75, 2, "Change Frequency");
}

void usbStatus(int y)
{
  tft.setCursor(15, y);
  tft.setTextSize(1);
  if(usbConnected == 1)
  {
    tft.setTextColor(GREEN);
    tft.println("USB Connected");
  }
  else
  {
    tft.setTextColor(RED);
    tft.println("USB Disconnected");
  }
}

void usbStatusClear(int y)
{
  tft.fillRect(15, y, 95, 10, BLACK);
}

void sinSquareBlock(int y)
{
  // Change Wave Button
  tft.drawRect(145, y, 90, 65, BLUE);
  // Text inside the button
  tft.setCursor(175, y+10);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.println("sin");
  tft.setCursor(155, y+40);
  tft.println("square");
  // Selection Outline
  if(waveform == 0)
  {
    tft.drawRect(150, y+5, 80, 25, WHITE);
  }
  else
  {
    tft.drawRect(150, y+35, 80, 25, WHITE);
  }
}

void sinSquareBlockClear(int y)
{
  // Change Wave Button
  tft.fillRect(145, y, 90, 65, BLACK);
}

void outputLevelBlock(int y)
{
  peakToPeakText(y);
  rmsText(y+35);
  dBmText(y+70);
  
  // Change output level Button
  tft.drawRect(165, y+3, 60, 50, BLUE);
  // Text inside the button
  tft.setCursor(177, y+13);
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Change");
  tft.setCursor(177, y+33);
  tft.println("Output");
}

// ************************************************************
// *******************Change Frequency Block*******************
// ************************************************************

void presetFrequencies(int y)
{
  button(143, y, 2, "1 MHz");
  button(143, y+60, 2, "1 KHz");
  button(143, y+120, 2, "1 Hz ");
}

void largeFrequencyIncrements(int y)
{
  button(15, y, 2, "<<<");
  button(75, y, 2, ">>>");
}

void multiplierBlock(int y)
{
  // multiplier Button
  tft.drawRect(15, y, 113, 50, BLUE);
  // Text inside the button
  tft.setCursor(25, y+15);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.println("x1 x2 x3");
  switch(multiplier)
  {
    case 1:
      tft.drawRect(20, y+5, 31, 40, WHITE);      
      break;
    
    case 2:
      tft.drawRect(56, y+5, 31, 40, WHITE);
      break;
    
    case 3:
      tft.drawRect(92, y+5, 31, 40, WHITE);
      break;
  }
}

void multiplierBlockClear(int y)
{
  // Change Wave Button
  tft.fillRect(15, y, 113, 50, BLACK);
}

// ****************************************************************
// *******************Enter Frequency Text Block*******************
// ****************************************************************


void keypadTextBlock(int y)
{
  tft.drawRect(45, y-10, 142, 35, BLUE);
  keypadText(y);

  tft.setCursor(158, y);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(2);
  tft.print(keypadStr);
  tft.print("Hz");
}

void keypadDescriptionBlock(int y)
{
  tft.setCursor(0, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.println("        Instructions:");
  tft.println("        Use the keypad to enter");
  tft.println("        the frequency. Press *");
  tft.println("        to clear the input and");
  tft.println("        press # to enter it");
}

// *********************************************************
// *******************Change Output Block*******************
// *********************************************************

void peakToPeakBlock()
{
  peakToPeakText(45);
  incrementPanel(68);
}

void rmsBlock()
{
  rmsText(115);
  incrementPanel(138);
}

void dbmBlock()
{
  dBmText(185);
  incrementPanel(208);
}

// **************************************************************
// *******************(Multiple Layout) Block*******************
// **************************************************************

void headerBlock(String str)
{
  headerText(str);
  dividerLine(30);
}






/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////                        Buttons                        /////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// **************************************************
// *******************Main Buttons*******************
// **************************************************

void changeFrequencyTextButton(int y)
{
  button(15, y, 1, "Enter Frequency");
}

void loadDefaults(int y)
{
  button(15, y, 1, " Load Defaults ");
}

// ********************************************************
// *******************(Multiple) Buttons*******************
// ********************************************************
void incrementPanel(int y)
{
  button(15, y, 2, "<<");
  button(70, y, 2, " <");
  button(125, y, 2, " >");
  button(180, y, 2, ">>");
}

void backButton()
{
  button(10, 275, 2, "BACK");
}








/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////                      Texts/Labels                     /////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// ******************************************************
// *******************(Multiple) Texts*******************
// ******************************************************

void headerText(String str)
{
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print(str);
}

void frequencyText(int y)
{
  tft.setCursor(30, y);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(2);
  tft.println(frequencyToString(frequency));
}

void frequencyTextClear(int y)
{
  tft.fillRect(20, y-5, 200, 25, BLACK);
}

void keypadText(int y)
{
  tft.setCursor(50, y);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(2);
  tft.print(keypadStr);
}

void keypadTextClear(int y)
{
  tft.fillRect(50, y-5, 107, 25, BLACK);
}

void keypadTextMessage(int y, uint16_t color, String message)
{
  tft.setCursor(65, y);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.print(message);
}

//TODO CHECK DIMENSIONS
void keypadTextMessageClear(int y)
{
  tft.fillRect(65, y, 120, 10, BLACK);
}

void peakToPeakText(int y)
{
  tft.setCursor(10, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("pkpk:");
  tft.setTextColor(MAGENTA);
  tft.print(voltageToString(pkpkVoltage));
}

void peakToPeakTextClear(int y)
{
  tft.fillRect(65, y, 90, 16, BLACK);
}

void rmsText(int y)
{
  tft.setCursor(10, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("rms:");
  tft.setTextColor(MAGENTA);
  tft.print(rmsToString(rmsVoltage));
}

void rmsTextClear(int y)
{
  tft.fillRect(55, y, 100, 16, BLACK);
}

void dBmText(int y)
{
  tft.setCursor(10, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("dBm:");
  tft.setTextColor(MAGENTA);
  tft.print(dBmToString(dBm));
}

void dBmTextClear(int y)
{
  tft.fillRect(55, y, 140, 16, BLACK);
}

void errorText()
{
  // Clear the screen
  clearScreen();

  tft.setCursor(10, 20);
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Error, Please Reset Device");
}

void dividerLine(int y)
{
  tft.fillRect(0, y, 240, 2, WHITE);
}






/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////                   Helper Functions                    /////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void clearScreen()
{
  tft.fillScreen(BLACK);
}

void changeDisplayScreens()
{
  clearScreen();
  switch(deviceState) 
  {
    case MAIN_SCREEN:
        mainScreenView();
        break;
    case FREQUENCY_SCREEN:
        changeFrequencyView();
        break;
    case OUTPUT_SCREEN:
        changeOutputView();
        break;
    case FREQUENCY_SCREEN_TEXT:
        changeFrequencyTextView();
        break;
    default :
        mainScreenView();
        break;
  }
  return;
}

void updateDisplayScreen()
{
  switch(deviceState) 
  {
    case MAIN_SCREEN:
      // Draw a black rectangle over the frequency
      frequencyTextClear(51);
      // Redraw the frequency text
      frequencyText(51);
      // Draw a black rectangle over the pkpkVoltage
      peakToPeakTextClear(130);
      // Redraw the pkpkVoltage text
      peakToPeakText(130);
      // Draw a black rectangle over the pkpkVoltage
      rmsTextClear(165);
      // Redraw the pkpkVoltage text
      rmsText(165);
      // Draw a black rectangle over the dBm text
      dBmTextClear(200);
      // Redraw the dBm text
      dBmText(200);
      // Draw a black rectangle over the Sin/Square button
      sinSquareBlockClear(230);
      // Redraw the Sin/Square button
      sinSquareBlock(230);
      break;

    case FREQUENCY_SCREEN:
      // Draw a black rectangle over the frequency
      frequencyTextClear(51);
      // Redraw the frequency text
      frequencyText(51);
      break;

    case FREQUENCY_SCREEN_TEXT:
      // Draw a black rectangle over the frequency
      frequencyTextClear(51);
      // Redraw the frequency text
      frequencyText(51);
      break;

    case OUTPUT_SCREEN:
      // Draw a black rectangle over the pkpkVoltage
      peakToPeakTextClear(45);
      // Redraw the pkpkVoltage text
      peakToPeakText(45);
      // Draw a black rectangle over the pkpkVoltage
      rmsTextClear(115);
      // Redraw the pkpkVoltage text
      rmsText(115);
      // Draw a black rectangle over the dBm text
      dBmTextClear(185);
      // Redraw the dBm text
      dBmText(185);
      break;
  }
}

void checkButtonPressed(int x, int y)
{
  switch(deviceState) 
  {
    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    case MAIN_SCREEN:
        // Change frequency button
        if(x > 15 && x < 225 && y > 75 && y < 109)
        {
          deviceState = FREQUENCY_SCREEN;
          changeDisplayScreens();
        }
        
        // Change output button
        if(x > 165 && x < 225 && y > 133 && y < 183)
        {
          deviceState = OUTPUT_SCREEN;
          changeDisplayScreens();
        }

        // Sin/Square button
        if(x > 145 && x < 235 && y > 230 && y < 295)
        {
          if(waveform == 0)
          {
            waveform = 1;
          }
          else
          {
            waveform = 0;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the Sin/Square button
          sinSquareBlockClear(230);
          // Redraw the Sin/Square button
          sinSquareBlock(230);
          // Delay 500 ms (Prevent buttom spamming)
          delay(500);
        }

        // Enter Frequency button
        if(x > 15 && x < 124 && y > 230 && y < 257)
        {
          deviceState = FREQUENCY_SCREEN_TEXT;
          changeDisplayScreens();
        }

        // Load Defaults button
        if(x > 15 && x < 124 && y > 270 && y < 297)
        {
          frequency = 100;
		  pkpkVoltage = 2.5;
		  rmsVoltage = .884;
		  dBm = -39.032;
		  waveform = 0;
		  updateDisplayScreen();
        }
        break;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    case FREQUENCY_SCREEN:
        /* Chart for the increment %
        Button %      1,000 1,000,000 10,000,000
        ---------------------------------------
        <<    .1%     1     1000      10000
        <     .001%   .01   10        100
        <<<   1%      10    10000     100000
        */
        // Frequency Increment Panel
        // <<
        if(x > 15 && x < 57 && y > 80 && y < 114)
        {
          switch(multiplier)
          {
            case 1:
              frequency -= 10;      
              break;
            
            case 2:
              frequency -= 10000;
              break;
            
            case 3:
              frequency -= 10000000;
              break;
          }

          if(frequency < 1)
          {
            frequency = 1;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // <
        if(x > 70 && x < 112 && y > 80 && y < 114)
        {
          frequency -= 1;

          if(frequency < 1)
          {
            frequency = 1;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // >
        if(x > 125 && x < 167 && y > 80 && y < 114)
        {
          frequency += 1;

          if(frequency > 1000000000)
          {
            frequency = 1000000000;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // >>
        if(x > 180 && x < 222 && y > 80 && y < 114)
        {          
          switch(multiplier)
          {
            case 1:
              frequency += 10;      
              break;
            
            case 2:
              frequency += 10000;
              break;
            
            case 3:
              frequency += 10000000;
              break;
          }

          if(frequency > 1000000000)
          {
            frequency = 1000000000;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }

        // Large Frequency Decrement
        if(x > 15 && x < 69 && y > 150 && y < 184)
        {
          switch(multiplier)
          {
            case 1:
              frequency -= 100;      
              break;
            
            case 2:
              frequency -= 100000;
              break;
            
            case 3:
              frequency -= 100000000;
              break;
          }

          if(frequency < 1)
          {
            frequency = 1;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // Large Frequency Increment
        if(x > 75 && x < 129 && y > 150 && y < 184)
        {
          switch(multiplier)
          {
            case 1:
              frequency += 100;      
              break;
            
            case 2:
              frequency += 100000;
              break;
            
            case 3:
              frequency += 100000000;
              break;
          }
          
          if(frequency > 1000000000)
          {
            frequency = 1000000000;
          }
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }

        // Preset Frequency buttons
        // 1 MHz
        if(x > 143 && x < 221 && y > 130 && y < 164)
        {
          frequency = 100000000;
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // 1 KHz
        if(x > 143 && x < 221 && y > 190 && y < 224)
        {
          frequency = 100000;
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }
        // 1 Hz
        if(x > 143 && x < 221 && y > 250 && y < 284)
        {
          frequency = 100;
          sendDataToArduino();
          sendDataToPC();
          // Draw a black rectangle over the frequency
          frequencyTextClear(51);
          // Redraw the frequency text
          frequencyText(51);
        }

        // Multiplier
        if(x > 15 && x < 128 && y > 200 && y < 250)
        {
          switch(multiplier)
          {
            case 1:
              multiplier = 2;   
              break;
            
            case 2:
              multiplier = 3;
              break;
            
            case 3:
              multiplier = 1;
              break;
          }

          // Draw a black rectangle over the button
          multiplierBlockClear(200);
          // Redraw the button
          multiplierBlock(200);
          // Delay 500 ms (Prevent buttom spamming)
          delay(500);
        }

        // Back button
        if(x > 10 && x < 76 && y > 275 && y < 309)
        {
          deviceState = MAIN_SCREEN;
          changeDisplayScreens();
        }
        break;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////    
    case OUTPUT_SCREEN:
        // Peak to Peak Increment Panel
        if(x > 10 && x < 60 && y > 68 && y < 108)
        {
          if(pkpkVoltage >= 12.5)
          {
            pkpkVoltage -= 10;
            rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
            dBm = 20.0*log10((pkpkVoltage/100.0)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 65 && x < 105 && y > 68 && y < 108)
        {
          if(pkpkVoltage > 2.5)
          {
            pkpkVoltage -= .1;
            rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 120 && x < 170 && y > 68 && y < 108)
        {
          if(pkpkVoltage < 2500)
          {
            pkpkVoltage += .1;
            rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 175 && x < 225 && y > 68 && y < 108)
        {
          if(pkpkVoltage <= 2490)
          {
            pkpkVoltage += 10;
            rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }

        // rmsVoltage Increment Panel
        if(x > 10 && x < 60 && y > 138 && y < 178)
        {
          if(rmsVoltage >= 1.884)
          {
            rmsVoltage -= 1;
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 65 && x < 105 && y > 138 && y < 178)
        {
          if(rmsVoltage > .884)
          {
            rmsVoltage -= .01;
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 120 && x < 170 && y > 138 && y < 178)
        {
          if(rmsVoltage < 883.884)
          {
            rmsVoltage += .1;
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 175 && x < 225 && y > 138 && y < 178)
        {
          if(rmsVoltage <= 882.884)
          {
            rmsVoltage += 1;
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }

        // dBm Increment Panel
        if(x > 10 && x < 60 && y > 208 && y < 248)
        {
          if(dBm >= -40.032)
          {
            dBm -= 1;
            rmsVoltage = pow(.05, .5) * pow(2.7182818, (dBm/8.6858896));
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 65 && x < 105 && y > 208 && y < 248)
        {
          if(dBm >= -39.082)
          {
            dBm -= .05;
            rmsVoltage = pow(.05, .5) * pow(2.7182818, (dBm/8.6858896));
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 120 && x < 170 && y > 208 && y < 248)
        {
          if(dBm <= 19.970)
          {
            dBm += 1;
            rmsVoltage = pow(.05, .5) * pow(2.7182818, (dBm/8.6858896));
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }
        if(x > 175 && x < 225 && y > 208 && y < 248)
        {
          if(dBm <= 20.920)
          {
            dBm += .05;
            rmsVoltage = pow(.05, .5) * pow(2.7182818, (dBm/8.6858896));
            pkpkVoltage = rmsVoltage * (2 * 1.41421356237);
            sendDataToArduino();
            sendDataToPC();
            // Draw a black rectangle over the pkpkVoltage
            rmsTextClear(115);
            // Redraw the pkpkVoltage text
            rmsText(115);
            // Draw a black rectangle over the pkpkVoltage
            peakToPeakTextClear(45);
            // Redraw the pkpkVoltage text
            peakToPeakText(45);
            // Draw a black rectangle over the dBm text
            dBmTextClear(185);
            // Redraw the dBm text
            dBmText(185);
          }
        }

        // Back button
        if(x > 10 && x < 70 && y > 260 && y < 310)
        {
          deviceState = MAIN_SCREEN;
          changeDisplayScreens();
        }
        break;
    
    //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////// 
    case FREQUENCY_SCREEN_TEXT:
        // Change frequency screen button
        if(x > 40 && x < 197 && y > 220 && y < 247)
        {
          deviceState = FREQUENCY_SCREEN;
          changeDisplayScreens();
        }

        // SAVE button
        if(x > 164 && x < 230 && y > 260 && y < 310)
        {
          keypadTextMessageClear(140);
          if(storeData())
          {
          	keypadTextMessage(140, GREEN, "  Signal Saved");
          }
          else
          {
          	keypadTextMessage(140, RED, "    Error Saving ");
          }
        }

        // Back button
        if(x > 10 && x < 70 && y > 260 && y < 310)
        {
          deviceState = MAIN_SCREEN;
          changeDisplayScreens();
        }
        break;

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////  
    default :
        // Do nothing
        break;
  }
}

void button(int x, int y, int textSize, String str)
{
  int padding = 10;
  int spaceLength = textSize;
  int letterLength = 5*textSize;
  int buttonLength = (str.length() * letterLength) + ((str.length() - 1) * spaceLength);
  int buttonHeight = 7*textSize;
  tft.drawRect(x, y, buttonLength+(padding*2), buttonHeight+(padding*2), BLUE);
  
  tft.setCursor(x+padding, y+padding);
  tft.setTextColor(RED);
  tft.setTextSize(textSize);
  tft.println(str);

  //Serial.println(str);
  //Serial.println(x);
  //Serial.println(y);
  int x2 = x + (buttonLength+(padding*2));
  //Serial.println(x2);
  int y2 = y + (buttonHeight+(padding*2));
  //Serial.println(y2);
}

// Helper method. Formats the frequency to the string frequencyString
String frequencyToString(long frequency)
{
  String frequencyString = String(15);
  char strf[12];
  ltoa(frequency, strf, 10);
  String str = String(strf);
  int length = floor(log10(abs(frequency))) + 1;
  // 0 - 1,000 (Hz)               
  // 1,000 - 1,000,000 (Khz)
  // 1,000,000 - 10,000,000 (Mhz)
  
  if (frequency < 100000)
  {
  	if (frequency < 10)
  	{
	    frequencyString = ".0";
	    frequencyString += str.substring(0, length - 2);
	    frequencyString += " Hz";
  	}
  	else
  	{
  		frequencyString = str.substring(0, length - 2);
	    frequencyString += ".";
	    frequencyString += str.substring(length - 2, length);
	    frequencyString += " Hz";
  	}    
  }
  else
  {
    if (frequency >= 100000 && frequency < 100000000)
    {
      frequencyString = str.substring(0, length - 5);
      frequencyString += ".";
      frequencyString += str.substring(length - 5, length);
      frequencyString += " KHz";
    }
    else
    {
      if (frequency > 999997800 && frequency < 1000000000)
      {
        frequencyString = str.substring(0, length - 9);
        frequencyString += ".";
        frequencyString += str.substring(length - 9, length);
        frequencyString += " MHz";
      }
      else
      {
        frequencyString = str.substring(0, length - 8);
        frequencyString += ".";
        frequencyString += str.substring(length - 8, length);
        frequencyString += " MHz";
      }
    }
  }

  return frequencyString;
}

// Helper method. Formats the pkpkVoltage to the string pkpkVoltage String
String voltageToString(double pkpkVoltage)
{
  String voltageString = String(8);
  char str[6];
  // 2.5 - 1,000 (mV - 0 shift)
  // 1,000 - 2,500 (V - 3 shift)
  if (pkpkVoltage < 1000)
  {
    dtostrf(pkpkVoltage, 4, 1, str);
    voltageString = str;
    voltageString += "mV";
  }
  else
  {
    dtostrf(pkpkVoltage/1000, 4, 3, str);
    voltageString = str;
    voltageString += "V";    
  }
  return voltageString;
}

// Helper method. Formats the rmsVoltage to the string rmsVoltage String
String rmsToString(double rmsVoltage)
{
  String rmsString = String(9);
  char str[7];

  dtostrf(rmsVoltage, 5, 2, str);
  rmsString = str;
  rmsString += "mV";
  return rmsString;
}

// Helper method. Formats the dBm to the string dBm String
String dBmToString(double dBm)
{
  String dBmString = String(12);
  char str[8];

  dtostrf(dBm, 7, 3, str);
  dBmString = str;
  dBmString += " dBm";

  return dBmString;
}

boolean keysPressedFunction(char c)
{
  if(keysPressed < 8)
  {
    keysPressed++;
    keypadStr += c;
    return true;
  }
  else
  {
    return true;
  }
}

// Reading in the format of   #12345678.91-2499.9-0
void readData()
{
  signalData = false;

  // get the new byte:
  char c = (char)Serial.read(); 
  
  // If the readBuffer is the % identifier then write back # and return
  if(c == '%')
  {
    Serial.println("#");
    if(deviceState == MAIN_SCREEN)
    {
      usbConnected = 1;
      //usbStatusClear(290);
      //usbStatus(290);
    }
    return;
  }

  if(c == '!')
  {
    if(deviceState == MAIN_SCREEN)
    {
      usbConnected = 0;
      //usbStatusClear(290);
      //usbStatus(290);
    }
    return;
  }
  
  // If the readBuffer is the $ command then send the data to the PC
  if(c == '$')
  {
    sendDataToPC();
    return;
  }
  
  if(c == '#')
  {
  	unsigned long startTime;
  	unsigned long currentTime;

    readBuffer[0] = c;
    for(int i = 1; i <= 20; i++)
    {
    	startTime = millis();
    	currentTime = millis();

    	while(!Serial.available())
      	{
      	  currentTime = millis();
      	  if((currentTime - startTime) > 250)
      	  {
      	    return;
      	  }
        }
        readBuffer[i] = (char)Serial.read();
    }
    readBuffer[21] = '\0';
    if(readBuffer[9] == '.' && readBuffer[12] == '-' && readBuffer[17] == '.' && readBuffer[19] == '-')
    {
    	signalData = true;
    }
    return;

    /*
    int i = 1;
    while(i <= 20 || (currentTime - startTime) < 500)
    {
      while(!Serial.available())
      {
      	startTime = millis();
      	currentTime = millis();
      	if((currentTime - startTime) > 250)
      	{
      		retrun;
      	}
      }
      readBuffer[i] = (char)Serial.read();
      i++;
    }
    readBuffer[i] = '\0';
    signalData = true;
    return;
    */
  }  
}

void sendDataToArduino()
{
  signalToString();
  Serial1.write(data);
  //TODO CHECK IF CHANGING TO Serial.println(data) works with fahim's arduino or if problems arise
}

void sendDataToPC()
{
  signalToString();
  Serial.println(data);
}

boolean storeData()
{
  signalToString();
  
  if(EepromUtil::eeprom_write_string(0, data))
  {
  	return true;
  }

  return false;
}

// Helper method that puts the signal variables into the character arry 'data'
void signalToString()
{
  char chr1[12];
  char chr2[7];
  char chr3[2];
  
  ltoa(frequency, chr1, 10);
  dtostrf(pkpkVoltage, 6, 1, chr2);
  dtostrf(waveform, 1, 0, chr3);
  
  String str1 = String(chr1);
  String str2 = String(chr2);
  String str3 = String(chr3);
  
  dataStr = "#";
  
  int length = 10 - str1.length();
  if(length > 8)
  {
    length = 8; 
  }
  for(int i = 0; i < length; i++)
  {
    dataStr += "0";
  }
  if(length < 8)
  {
    dataStr += str1.substring(0, str1.length()-2);
    dataStr += ".";
    dataStr += str1.substring(str1.length()-2, str1.length());
  }
  else
  {
    if(str1.length() <= 1)
    {
      dataStr += ".0";
      dataStr += str1;
    }
    else
    {
      dataStr += ".";
      dataStr += str1;
    }
  }

  dataStr += "-";
  str2.replace(" ", "0");
  dataStr += str2;
  dataStr += "-";
  dataStr += str3;
  dataStr.toCharArray(data, 22);
}

// Helper method takes the data from the 'readBuffer' character arry and puts it into the signal variables
void stringToSignal()
{
	String str = String(readBuffer);
	str.substring(1, 12).toCharArray(freq, 12);
	str.substring(13, 19).toCharArray(volt, 7);
	str.substring(20, 21).toCharArray(wave, 2);

	// Remove the decimal
	freq[sizeof(freq)-4] = freq[sizeof(freq)-3];
	freq[sizeof(freq)-3] = freq[sizeof(freq)-2];
	freq[sizeof(freq)-2] = freq[sizeof(freq)-1];
	freq[sizeof(freq)-1] = '\0';

	frequency = atol(freq);
	pkpkVoltage = strtod(volt, NULL);
	rmsVoltage = pkpkVoltage / (2 * 1.41421356237);
	dBm = 20*log10((pkpkVoltage/100)/pow(.05, .5));
	waveform = strtod(wave, NULL);
}
