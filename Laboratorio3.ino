#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_Address 0x3c

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1    
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int pinLM35 = A0;             
const float Vref = 5.0;             

void setup() {
  Serial.begin(9600);

  delay(250); 
  display.begin(i2c_Address, true); 
  display.setContrast(255);

  display.clearDisplay();
  display.display();

  Serial.println("OLED + LM35 inicializado");
}

void loop() {
  int lectura = analogRead(pinLM35);                
  float voltaje = lectura * (Vref / 1023.0);        
  float tempC = voltaje * 100.0;   

  // --- Mostrar en Serial ---
  Serial.print("Temperatura: ");
  Serial.print(tempC, 2);
  Serial.println(" °C");

  display.clearDisplay();

  display.setTextSize(1);              
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 0);
  display.println("Temperatura");

  if (tempC < 20) {
    display.setTextSize(2);   
  } else if (tempC >= 20 && tempC < 30) {
    display.setTextSize(3);   
  } else {
    display.setTextSize(4);   
  }

  display.setCursor(10, 25);
  display.print(tempC, 1);             
  display.println(" C");

  display.display();

  delay(1000); 
}
