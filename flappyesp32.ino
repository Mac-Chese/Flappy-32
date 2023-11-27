/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define BUTTON_PIN 16 // ESP32 GPIO16 pin connected to button's pin
#define BUZZER_PIN 23 // ESP32 GPIO3 pin connected to Buzzer's pin

#define POT_PIN 15

int birdY, lastBirdY = 0;
int pipeX, pipeY, lastPipeX = 148, lastPipeY = 32; 

int delayTime = -1;
// notes in the melody:
int melody[] = {
  NOTE_A7
};

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
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
  
  for (int i = 128; i >= 0; i= i - 8){
    potADCReading = analogRead(POT_PIN);
    pipeX = i; 
    birdY = map(potADCReading, 0, 4095, 0, 64);
    pipeY = map(potADCReading, 0, 4095, 8, 56);
    delayTime = map(potADCReading, 0, 4095, 0, 500);
    
    moveBird(16, birdY, 6, 16, lastBirdY, 6);

    movePipe(pipeX, pipeY, lastPipeX, lastPipeY);

    lastBirdY = birdY;
    lastPipeX = pipeX;
    lastPipeY = pipeY;
    display.display();

    tone(BUZZER_PIN, melody[0]);
    delay(100);
    noTone(BUZZER_PIN);

    }
  
  
}

void movePipe(int pipeX, int pipeY, int lastPipeX, int lastPipeY) { 
  int pipeRadius = 4, pipeWidth = 16, pipeHeight = 64, pipeSeperator = 17;

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