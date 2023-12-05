/**
Authors: Matt Barber, Aaron Morris
Date: 12/4/23
Description: This is a flappy bird game made to run on an esp32 development board
**/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUTTON_PIN 2
#define BUZZER_PIN 0
#define POT_PIN 15


const int gravityAmount = 3;
const int jumpAmount = 18;

const int birdStartingPosY = 16;
const int birdX = 16;
const int birdRad = 5;

const int pipeYMin = 15;
const int pipeYMax = 48;
const int pipeRadius = 2;
const int pipeWidth = 10;
const int pipeHeight = 64; 
const int pipeSeperator = 38;

const int pipeNoise1 = 2500;
const int pipeNoise2 = 3700;
const int birdNoise = 620;
const int deathNoise = 84;


int birdY = birdStartingPosY;
int pipeX = 120;
int pipeY = random(pipeYMin, pipeYMax);

unsigned int currentScore = 0;
unsigned int highScore = -1;

bool gravityFlag = false;
bool buttonLevel = false, lastButtonLevel = true;
bool pastPipe;

void moveBird(int xPosition, int yPosition);
void movePipe(int pipeX, int pipeY);
bool pipeHitDetection(int birdX, int birdY, int pipeX, int pipeY);
void startMenu();
void drawMenuScoreBox();
void drawGameScoreBox();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Preferences preferences;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(2000);
  display.clearDisplay();
  display.display(); 
  delay(100);

  preferences.begin("flappy", false);

  highScore = preferences.getUInt("highScore", 0);

  preferences.end();
}

void loop() {
  int delayTime;
  int potADC;
  unsigned long startTime, currentTime;
  
  bool loser = false;
  bool justLost = false;

  display.clearDisplay();
  moveBird(birdX, birdY);
  movePipe(pipeX, pipeY);

  display.display();

  while (true) {
    
    justLost = false;
    pipeY = random(pipeYMin, pipeYMax);

    startMenu();
    
    Serial.println("Starting Now");
    // reset some stuff
    loser = false;
    currentScore = 0;

    display.clearDisplay();

    lastButtonLevel = buttonLevel;

    while (!loser)  {
      startTime = millis();
      potADC = analogRead(POT_PIN);
      delayTime = map(potADC, 0, 4095, 0, 500);

      do {
        display.clearDisplay();
        drawGameScoreBox();
        moveBird(birdX, birdY);
        movePipe(pipeX, pipeY);
        display.display();

        loser = pipeHitDetection(birdX, birdY, pipeX, pipeY);
        justLost = loser;

        // read the button
        buttonLevel = digitalRead(BUTTON_PIN);

        // check if the button is going from low to high to indicate a jump
        if (lastButtonLevel == false && buttonLevel == true && !justLost) {
          // if we are at the celing then don't keep going otherwise go up more
          if ((birdY - jumpAmount) <= 2) {
            birdY = 3;
          }
          else {
            birdY -= jumpAmount;
          }
          
          tone(BUZZER_PIN, birdNoise, 20);

        }
        lastButtonLevel = buttonLevel;

        // gravity only subtracts from the height every other time
        if (gravityFlag && !justLost) {
          birdY += gravityAmount;
          gravityFlag = false;
          }
        else {
          gravityFlag = true;
        }

        currentTime = millis();

      } while (((currentTime - startTime) < delayTime) && !loser);

      if (pipeX > 0) {
        pipeX -= 4;
      }
      else  {
        pipeX = 120;
        pipeY = random(pipeYMin, pipeYMax);
      }
    } 

    // after you lose you have to wait half a second to start again
    if (justLost) {
      noTone(BUZZER_PIN);
      tone(BUZZER_PIN, deathNoise, 500);
      
      // if the high score is new save it in flash
      if (currentScore == highScore) {
      preferences.begin("flappy", false);
      preferences.putUInt("highScore", highScore);
      preferences.end();
      }

      delay(500);
    }
  }
}


void startMenu()  {
    pipeX = 120;
    birdY = 16;
    
    while (true) {

      display.clearDisplay();
      drawMenuScoreBox();
      moveBird(birdX, birdY);
      movePipe(pipeX, pipeY);
      display.display();
  
      
      if (birdY > 48) {
        birdY = 16;
     
        tone(BUZZER_PIN, birdNoise, 10);
      }

      if (gravityFlag) {
          birdY += gravityAmount;
          gravityFlag = false;
          }
      else {
          gravityFlag = true;
      }
      buttonLevel = digitalRead(BUTTON_PIN);

      // check if the button is pressed to start game
      if (lastButtonLevel == false && buttonLevel == true) {
        birdY -= jumpAmount;
        break;
      }

      lastButtonLevel = buttonLevel;

    }

}


// returns true if hit is detected, false otherwise
bool pipeHitDetection(int birdX, int birdY, int pipeX, int pipeY){
  int hitBoxLowY, hitBoxHighY;
  int hitBoxRight, hitBoxLeft;
  int hitBoxMargin = (pipeSeperator / 2);
  int pipeLeft, pipeRight;
  bool birdHit = false;
  
  pipeLeft = pipeX - (pipeWidth / 2);
  pipeRight = pipeX + (pipeWidth / 2);
  hitBoxRight = birdX + birdRad;
  hitBoxLeft = birdX - birdRad;
 
  // checks if we are hitting the bottom
  if (birdY >= SCREEN_HEIGHT){
    birdHit = true;
  }

  // checks if the left or right side of the birds hitbox could hit the pipe and if it could then it checks if it did
  else if (((hitBoxRight > pipeLeft) && (hitBoxRight < pipeRight)) || ((hitBoxLeft > pipeLeft) && (hitBoxLeft < pipeRight))) {
    
    if ((birdY + birdRad) > (pipeY + hitBoxMargin)){
      birdHit = true;
    }

    else if ((birdY - birdRad) < (pipeY - hitBoxMargin)){
      birdHit = true;
    }
  }

  else if (pipeRight <= 5 && !pastPipe)  {
      pastPipe = true;
      
      currentScore++;
      if (currentScore > highScore) {
        highScore = currentScore;
      }

      Serial.print("Current Score:\t");
      Serial.println(currentScore);
      Serial.print("High Score:\t");
      Serial.println(highScore);

      tone(BUZZER_PIN, pipeNoise1, 10);
      tone(BUZZER_PIN, pipeNoise2, 5);
    }
  
  else if (pipeRight > 5) {
    pastPipe = false;
  }


  return birdHit;

}

void drawGameScoreBox() {
  // score box in the upper left
  int scoreCursorX = 96;
  int scoreCursorY = 56;
  
  display.setCursor(scoreCursorX, scoreCursorY);
  display.setTextColor(WHITE);
  display.print("CS ");
  display.print(currentScore);

}


void drawMenuScoreBox() {
  // score box in the upper left
  int scoreCursorX = 24;
  int scoreCursorY = 56;
  
  display.setCursor(scoreCursorX, scoreCursorY);
  display.setTextColor(WHITE);
  display.print("LS ");
  display.print(currentScore);
  display.print("  ");
  display.print("HS ");
  display.print(highScore);

}

// draws a pipe in a new location
void movePipe(int pipeX, int pipeY) { 
  int topX, topY, botY;

  topX = pipeX - (pipeWidth / 2);
  topY = pipeY - (pipeHeight + ((pipeSeperator - 1)) / 2);
  botY = pipeY + ((pipeSeperator - 1) / 2);

  // draw new ones
  display.fillRoundRect(topX, topY, pipeWidth, pipeHeight, pipeRadius, WHITE);
  display.fillRoundRect(topX, botY, pipeWidth, pipeHeight, pipeRadius, WHITE);

}


void moveBird(int xPosition, int yPosition) {
  int circleX0, circleY0;
  int triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2;
  int birdEyeX0, birdEyeY0, birdEyeRadius;

  circleY0 = yPosition;
  circleX0 = xPosition;

  triangleX0 = xPosition + ( 1.75  * birdRad) ;
  triangleY0 = yPosition;
  triangleX1 = xPosition + (0.75 * birdRad);
  triangleY1 = yPosition - (0.75 * birdRad);
  triangleX2 = xPosition + (0.75 * birdRad);
  triangleY2 = yPosition + (0.75 * birdRad);

  birdEyeX0 = xPosition;
  birdEyeY0 = yPosition - (birdRad / 2.33333333);
  birdEyeRadius = birdRad / 6;
  
  if (birdEyeRadius == 0) {
    birdEyeRadius = 1;
  }

  display.drawCircle(circleX0, circleY0, birdRad + 1, BLACK); // outline

  display.fillTriangle(triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2, WHITE); // beak
  display.fillCircle(circleX0, circleY0, birdRad, WHITE); // body
  display.fillCircle(birdEyeX0, birdEyeY0, birdEyeRadius, BLACK); // eye
  display.drawTriangle(triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2, BLACK); // beak outline

  
}
