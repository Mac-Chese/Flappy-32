#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUTTON_PIN 2
#define BUZZER_PIN 23 // ESP32 GPIO3 pin connected to Buzzer's pin
#define BUZZER_2_PIN 0
#define POT_PIN 15

const int birdStartingPosY = 16;
const int gravityAmount = 1;
const int jumpAmount = 18;
const int pipeYMin = 8;
const int pipeYMax = 48;
const int birdX = 16;
const int birdRad = 5;

int birdY = birdStartingPosY;
int lastBirdY = 0;
int pipeX = 128;
int pipeY = random(pipeYMin, pipeYMax);
int lastPipeX = 148;
int lastPipeY = 32; 

int pipeRadius = 2;
int pipeWidth = 10;
int pipeHeight = 64; 
int pipeSeperator = 38;

int delayTime = -1;
// notes in the melody:
int melody[] = {
  NOTE_A7,
  NOTE_B2
};

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
  
 
}

void loop() {
  int potADCReading;
  int gravityCounter = 0;
  unsigned long startTime, currentTime;
  bool buttonLevel = false, lastButtonLevel = true;
  bool gravityFlag = false;
  bool loser = true;
  bool justLost = false;
    
  moveBird(birdX, birdY, birdRad, birdX, lastBirdY, birdRad);
  movePipe(pipeX, pipeY, lastPipeX, lastPipeY);
  display.display();

  while (true) {
    buttonLevel = digitalRead(BUTTON_PIN);
    justLost = false;

    if (lastButtonLevel == false && buttonLevel == true) {
      loser = false;
      
      Serial.println("Starting game");

      // reset game
      birdY = birdStartingPosY;
      lastBirdY = 0;
      pipeX = 128;
      pipeY = random(pipeYMin, pipeYMax);
      lastPipeX = 148;
      lastPipeY = 32; 

      display.clearDisplay();

    }

    lastButtonLevel = buttonLevel;

    while (!loser)  {
      startTime = millis();
      
      potADCReading = analogRead(POT_PIN);
      delayTime = map(potADCReading, 0, 4095, 0, 500);

      currentTime = millis();

      do {

        moveBird(birdX, birdY, birdRad, birdX, lastBirdY, birdRad);
        movePipe(pipeX, pipeY, lastPipeX, lastPipeY);
        lastBirdY = birdY;
        lastPipeX = pipeX;
        lastPipeY = pipeY;
        display.display();

        buttonLevel = digitalRead(BUTTON_PIN);

        if (lastButtonLevel == false && buttonLevel == true) {
          if ((birdY - jumpAmount) <= 2) {
            birdY = 3;
          }
          else {
            birdY -= jumpAmount;
          }
          
          tone(BUZZER_PIN, melody[0], 20);

        }
        lastButtonLevel = buttonLevel;

        if (birdY < SCREEN_HEIGHT && gravityFlag) {
          birdY += gravityAmount;
          }
        else if (birdY >= SCREEN_HEIGHT && gravityFlag) {
          loser = true;
          justLost = true;
        } 

        // how frequently gravity takes effect, every two cycles
        if (!gravityFlag) {
          if (gravityCounter < 2){
            gravityCounter ++;
          }
          
          else {
            gravityFlag = true;
            gravityCounter = 0;
          }
        }

        currentTime = millis();

        if (hitDetection(birdX, birdY, pipeX, pipeY)) {
          Serial.print(millis());
          Serial.println(", hit detected");
          loser = true;
          justLost = true;
        }
        
      } while (((currentTime - startTime) < delayTime) && !loser);

      if (pipeX > 0){
        pipeX -= 4;
      }
      else  {
        pipeX = 128;
        pipeY = random(pipeYMin, pipeYMax);
      }
    } 

    // after you lose you have to wait half a second to start again
    if (justLost) {
      tone(BUZZER_2_PIN, melody[1], 500);
      delay(500);
    }
  }
}


// returns true if hit is detected, false otherwise
bool hitDetection(int birdX, int birdY, int pipeX, int pipeY){
  int hitBoxLowY, hitBoxHighY;
  int hitBoxRight;
  int hitBoxMargin = (pipeSeperator / 2);
  int pipeLeft, pipeRight;
  bool birdHit = false;
  
  pipeLeft = pipeX;
  pipeRight = pipeX + pipeWidth;
  hitBoxRight = birdX + birdRad;

  for (int i = 0; i < ((birdRad + 1) * 2); i++) {

    if (((hitBoxRight - i) >= pipeLeft) && ((hitBoxRight - i) <= pipeRight)) {
      
      if (birdY > (pipeY + hitBoxMargin)){
        birdHit = true;
        break;
      }

      else if (birdY < (pipeY - hitBoxMargin)){
        birdHit = true;
        break;
      }
    }
  }

  return birdHit;

}

void movePipe(int pipeX, int pipeY, int lastPipeX, int lastPipeY) { 
  int topX, topY, botX, botY;
  int lastTopX, lastTopY, lastBotX, lastBotY;

  lastTopX = lastPipeX - (pipeWidth / 2);
  lastTopY = lastPipeY - (pipeHeight + ((pipeSeperator - 1)) / 2);
  lastBotX = lastPipeX - (pipeWidth / 2);
  lastBotY = lastPipeY + ((pipeSeperator - 1) / 2);

  topX = pipeX - (pipeWidth / 2);
  topY = pipeY - (pipeHeight + ((pipeSeperator - 1)) / 2);
  botX = pipeX - (pipeWidth / 2);
  botY = pipeY + ((pipeSeperator - 1) / 2);

  display.fillRoundRect(lastTopX, lastTopY, pipeWidth, pipeHeight, pipeRadius, BLACK);
  display.fillRoundRect(lastBotX, lastBotY, pipeWidth, pipeHeight, pipeRadius, BLACK);

  // draw new ones
  display.fillRoundRect(topX, topY, pipeWidth, pipeHeight, pipeRadius, WHITE);
  display.fillRoundRect(botX, botY, pipeWidth, pipeHeight, pipeRadius, WHITE);

}


void moveBird(int xPosition, int yPosition, int birdRadius, int lastXPosition, int lastYPosition, int lastBirdRadius) {
  
  //draw bird here
  int circleX0, circleY0, circleRadius;
  int triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2;
  int birdEyeX0, birdEyeY0, birdEyeRadius;

  int lastCircleX0, lastCircleY0, lastCircleRadius;
  int lastTriangleX0, lastTriangleY0, lastTriangleX1, lastTriangleY1, lastTriangleX2, lastTriangleY2;

  circleY0 = yPosition;
  circleX0 = xPosition;
  circleRadius = birdRadius;

  triangleX0 = xPosition + ( 1.75  * circleRadius) ;
  triangleY0 = yPosition;
  triangleX1 = xPosition + (0.75 * circleRadius);
  triangleY1 = yPosition - (0.75 * circleRadius);
  triangleX2 = xPosition + (0.75 * circleRadius);
  triangleY2 = yPosition + (0.75 * circleRadius);

  birdEyeX0 = xPosition;
  birdEyeY0 = yPosition - (birdRadius / 2.33333333);
  birdEyeRadius = birdRadius / 6;
  
  if (birdEyeRadius == 0) {
    birdEyeRadius = 1;
  }

  lastCircleY0 = lastYPosition;
  lastCircleX0 = lastXPosition;
  lastCircleRadius = lastBirdRadius;

  lastTriangleX0 = lastXPosition + ( 1.75  * lastCircleRadius) ;
  lastTriangleY0 = lastYPosition;
  lastTriangleX1 = lastXPosition + (0.75 * lastCircleRadius);
  lastTriangleY1 = lastYPosition - (0.75 * lastCircleRadius);
  lastTriangleX2 = lastXPosition + (0.75 * lastCircleRadius);
  lastTriangleY2 = lastYPosition + (0.75 * lastCircleRadius);

  display.fillCircle(lastCircleX0, lastCircleY0, lastBirdRadius + 1, BLACK); // hitbox
  display.fillTriangle(lastTriangleX0, lastTriangleY0, lastTriangleX1, lastTriangleY1, lastTriangleX2, lastTriangleY2, BLACK); // hitbox

  display.drawCircle(circleX0, circleY0, birdRadius + 1, BLACK); // outline

  display.fillTriangle(triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2, WHITE); // beak
  display.fillCircle(circleX0, circleY0, circleRadius, WHITE); // body
  display.fillCircle(birdEyeX0, birdEyeY0, birdEyeRadius, BLACK); // eye
  display.drawTriangle(triangleX0, triangleY0, triangleX1, triangleY1, triangleX2, triangleY2, BLACK); // beak outline

  

}
