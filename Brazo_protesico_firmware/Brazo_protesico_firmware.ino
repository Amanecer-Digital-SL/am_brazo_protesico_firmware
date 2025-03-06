#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <Preferences.h>
// Zumbador
#define Zum 12
// Pulsadores
#define SW1 4
#define SW2 32
#define SW3 18
#define SW4 33
#define SWL 26
#define SWR 27
// Potenciometros
#define RX 36
#define RY 39
#define LX 35
#define LY 34
// Servomotores
#define Servo_1 13
#define Servo_2 16
#define Servo_3 17
#define Servo_4 19
#define Servo_5 25
#define Servo_6 23
#define Servo_7 14
// Direccion I2C
#define OLED_ADDR 0x3C

uint8_t menu = 1;
uint8_t submenu = 1;

struct Posiciones{
  uint8_t pos_servo_1;
  uint8_t pos_servo_2;
  uint8_t pos_servo_3;
  uint8_t pos_servo_4;
  uint8_t pos_servo_5;
  uint8_t pos_servo_6;
  uint8_t pos_servo_7;
};


uint16_t pote_value = 0;

bool submenu_bool_1 = 0;
bool submenu_bool_2 = 0;
bool submenu_bool_3 = 0;
bool submenu_bool_1_1 = 0;
bool submenu_bool_1_2 = 0;
bool submenu_bool_1_3 = 0;
bool submenu_bool_1_4 = 0;
bool submenu_bool_1_5 = 0;
bool submenu_bool_1_6 = 0;
bool submenu_bool_1_7 = 0;
bool save_pos = 0;
bool go_pos = 0;

uint64_t sw_time = 0;
uint64_t sw_time_option = 1000;

Posiciones pos_actual = { 0, 0, 0, 0, 0, 0, 0};
Posiciones posicion_1 = { 0, 0, 0, 0, 0, 0, 0};
Posiciones posicion_2 = { 0, 0, 0, 0, 0, 0, 0};
Posiciones posicion_3 = { 0, 0, 0, 0, 0, 0, 0};
Posiciones posicion_4 = { 0, 0, 0, 0, 0, 0, 0};
Posiciones posicion_5 = { 0, 0, 0, 0, 0, 0, 0};

// Opciones del menu
String menuItems[3] = {"Mover Servo", "Ubicaciones", "Leer Serial"};
String menuItems_1[7] = {"Servomotor 1","Servomotor 2","Servomotor 3","Servomotor 4","Servomotor 5","Servomotor 6","Servomotor 7"};
String menuItems_2[5] = {"Posicion 1","Posicion 2","Posicion 3","Posicion 4","Posicion 5"};

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Preferences preferences;
Servo Servo1;
Servo Servo2;
Servo Servo3;
Servo Servo4;
Servo Servo5;
Servo Servo6;
Servo Servo7;

void recibir_serial() {
  String data = Serial.readString();
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("  Mensaje Recibido:");
  display.setCursor(0, 30);
  display.print(data);
  display.display();
}

void print_ok(){
  display.setCursor(100,28);
  display.fillRoundRect(95,25, 20, 14,7, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.print("ok");
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void save_positions(const struct Posiciones &actual, struct Posiciones &guardada, const String &servo) {
  const int NUM_SERVOS = 7;  // Número de servos
  String memory_values[NUM_SERVOS];
  char memory_values_CHAR[NUM_SERVOS][12];

  // Generar las claves de memoria para cada servo
  for (int i = 0; i < NUM_SERVOS; ++i) {
    memory_values[i] = "pos" + servo + "_servo" + String(i + 1);  // posX_servoY
    memory_values[i].toCharArray(memory_values_CHAR[i], 12);      // Convertir a char array
  }

  // Guardar las posiciones en la estructura
  guardada = actual;

  // Guardar las posiciones en las preferencias
  preferences.begin("posiciones", false);
  preferences.putUShort(memory_values_CHAR[0], actual.pos_servo_1);
  preferences.putUShort(memory_values_CHAR[1], actual.pos_servo_2);
  preferences.putUShort(memory_values_CHAR[2], actual.pos_servo_3);
  preferences.putUShort(memory_values_CHAR[3], actual.pos_servo_4);
  preferences.putUShort(memory_values_CHAR[4], actual.pos_servo_5);
  preferences.putUShort(memory_values_CHAR[5], actual.pos_servo_6);
  preferences.putUShort(memory_values_CHAR[6], actual.pos_servo_7);
  preferences.end();
}

void mover_servos(struct Posiciones actual) {

  Servo1.setPeriodHertz(50);
  Servo2.setPeriodHertz(50);
  Servo3.setPeriodHertz(50);
  Servo4.setPeriodHertz(50);
  Servo5.setPeriodHertz(50);
  Servo6.setPeriodHertz(50);
  Servo7.setPeriodHertz(50);

  Servo1.write(actual.pos_servo_1);
  Servo2.write(actual.pos_servo_2);
  Servo3.write(actual.pos_servo_3);
  Servo4.write(actual.pos_servo_4);
  Servo5.write(actual.pos_servo_5);
  Servo6.write(actual.pos_servo_6);
  Servo7.write(actual.pos_servo_7);
}

void detener_servos(){
  analogWrite(Servo_1, 0);
  analogWrite(Servo_2, 0);
  analogWrite(Servo_3, 0);
  analogWrite(Servo_4, 0);
  analogWrite(Servo_5, 0);
  analogWrite(Servo_6, 0);
  analogWrite(Servo_7, 0);
}

uint8_t cambio_menu(uint8_t posicion, uint8_t lim_inferior, uint8_t lim_superior){
  uint8_t posicion_value = posicion;
  if(analogRead(LY)>3500){
    posicion_value--;
    while(analogRead(LY)>3000){}
  }
  else if(analogRead(LY)<500){
    posicion_value++;
    while(analogRead(LY)<1000){}
  }
  if(posicion_value < lim_inferior){
    posicion_value = lim_inferior;
  }
  else if(posicion_value > lim_superior){
    posicion_value = lim_superior;
  }
  return posicion_value;
}

void drawProgressBar(int x, int y, int width, int height, int value) {
  display.clearDisplay();
  if (value < 0) value = 0;   // Limitar el valor mínimo a 0
  if (value > 180) value = 180; // Limitar el valor máximo a 100

  int fillWidth = (value * width) / 180; // Calcular el ancho relleno según el valor

  // Dibujar el contorno de la barra
  display.drawRect(x, y, width, height, SSD1306_WHITE);

  // Dibujar el relleno de la barra
  display.fillRect(x + 1, y + 1, fillWidth - 2, height - 2, SSD1306_WHITE);

  // Actualizar la pantalla
  display.display();
}

void menu_principal(uint8_t menu_size, uint8_t selector, String menuItems[]){
  display.clearDisplay();
  uint8_t lim_inferior = 0;
  uint8_t lim_superior = menu_size - 1;
  uint8_t menu_show = (selector - 1);
  if(menu_show == lim_inferior){
    menu_show++;
  }
  else if(menu_show == lim_superior){
    menu_show--;
  }
  uint8_t menu_pos = 0;
  for(uint8_t i = menu_show; i <= (menu_show + 2); i++){
    display.setCursor(0,(menu_pos)*20+10);  
    menu_pos++;   
    if(i == selector){
      display.print(" >  ");
    }
    else{
      display.print("    ");
    }
    display.print((String)menuItems[(i-1)]);
  }
  display.display();
}

void move_pos(){
  if(digitalRead(SW1)){
    Serial.println("SW1");
    while(digitalRead(SW1));
    mover_servos(posicion_1);
    delay(1000);
    detener_servos();
  }
  else if(digitalRead(SW2)){
    Serial.println("SW2");
    while(digitalRead(SW2));
    mover_servos(posicion_2);
    delay(1000);
    detener_servos();
  }
  else if(digitalRead(SW3)){
    Serial.println("SW3");
    while(digitalRead(SW3));
    mover_servos(posicion_3);
    delay(1000);
    detener_servos();
  }
  else if(digitalRead(SW4)){
    Serial.println("SW4");
    while(digitalRead(SW4));
    mover_servos(posicion_4);
    delay(1000);
    detener_servos();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setRxTimeout(50);
  pinMode(SWR,INPUT);
  pinMode(SWL,INPUT);
  pinMode(Zum,OUTPUT);
  pinMode(Servo_1,OUTPUT);
  pinMode(Servo_2,OUTPUT);
  pinMode(Servo_3,OUTPUT);
  pinMode(Servo_4,OUTPUT);
  pinMode(Servo_5,OUTPUT);
  pinMode(Servo_6,OUTPUT);
  pinMode(Servo_7,OUTPUT);
  pinMode(SW1,INPUT);
  pinMode(SW2,INPUT);
  pinMode(SW3,INPUT);
  pinMode(SW4,INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

	Servo1.setPeriodHertz(50);
  Servo2.setPeriodHertz(50);
  Servo3.setPeriodHertz(50);
  Servo4.setPeriodHertz(50);
  Servo5.setPeriodHertz(50);
  Servo6.setPeriodHertz(50);
  Servo7.setPeriodHertz(50);

	Servo1.attach(Servo_1, 500, 4000);
  Servo2.attach(Servo_2, 500, 4000);
  Servo3.attach(Servo_3, 500, 4000);
  Servo4.attach(Servo_4, 500, 4000);
  Servo5.attach(Servo_5, 500, 4000);
  Servo6.attach(Servo_6, 500, 4000);
  Servo7.attach(Servo_7, 500, 4000);

  preferences.begin("posiciones", false);
  posicion_1.pos_servo_1 = preferences.getUShort("pos1_servo1",0);
  posicion_1.pos_servo_2 = preferences.getUShort("pos1_servo2",0);
  posicion_1.pos_servo_3 = preferences.getUShort("pos1_servo3",0);
  posicion_1.pos_servo_4 = preferences.getUShort("pos1_servo4",0);
  posicion_1.pos_servo_5 = preferences.getUShort("pos1_servo5",0);
  posicion_1.pos_servo_6 = preferences.getUShort("pos1_servo6",0);
  posicion_1.pos_servo_7 = preferences.getUShort("pos1_servo7",0);

  posicion_2.pos_servo_1 = preferences.getUShort("pos2_servo1",0);
  posicion_2.pos_servo_2 = preferences.getUShort("pos2_servo2",0);
  posicion_2.pos_servo_3 = preferences.getUShort("pos2_servo3",0);
  posicion_2.pos_servo_4 = preferences.getUShort("pos2_servo4",0);
  posicion_2.pos_servo_5 = preferences.getUShort("pos2_servo5",0);
  posicion_2.pos_servo_6 = preferences.getUShort("pos2_servo6",0);
  posicion_2.pos_servo_7 = preferences.getUShort("pos2_servo7",0);

  posicion_3.pos_servo_1 = preferences.getUShort("pos3_servo1",0);
  posicion_3.pos_servo_2 = preferences.getUShort("pos3_servo2",0);
  posicion_3.pos_servo_3 = preferences.getUShort("pos3_servo3",0);
  posicion_3.pos_servo_4 = preferences.getUShort("pos3_servo4",0);
  posicion_3.pos_servo_5 = preferences.getUShort("pos3_servo5",0);
  posicion_3.pos_servo_6 = preferences.getUShort("pos3_servo6",0);
  posicion_3.pos_servo_7 = preferences.getUShort("pos3_servo7",0);

  posicion_4.pos_servo_1 = preferences.getUShort("pos4_servo1",0);
  posicion_4.pos_servo_2 = preferences.getUShort("pos4_servo2",0);
  posicion_4.pos_servo_3 = preferences.getUShort("pos4_servo3",0);
  posicion_4.pos_servo_4 = preferences.getUShort("pos4_servo4",0);
  posicion_4.pos_servo_5 = preferences.getUShort("pos4_servo5",0);
  posicion_4.pos_servo_6 = preferences.getUShort("pos4_servo6",0);
  posicion_4.pos_servo_7 = preferences.getUShort("pos4_servo7",0);

  posicion_5.pos_servo_1 = preferences.getUShort("pos5_servo1",0);
  posicion_5.pos_servo_2 = preferences.getUShort("pos5_servo2",0);
  posicion_5.pos_servo_3 = preferences.getUShort("pos5_servo3",0);
  posicion_5.pos_servo_4 = preferences.getUShort("pos5_servo4",0);
  posicion_5.pos_servo_5 = preferences.getUShort("pos5_servo5",0);
  posicion_5.pos_servo_6 = preferences.getUShort("pos5_servo6",0);
  posicion_5.pos_servo_7 = preferences.getUShort("pos5_servo7",0);
  preferences.end();
  
}

void loop() {
  menu = cambio_menu(menu, 1, 3);
  menu_principal(3, menu, menuItems);
  move_pos();
  if(digitalRead(SWR)==HIGH){
    tone(Zum,4000,100);
    delay(100);
    while(digitalRead(SWR)==HIGH){}
    switch(menu){
      case 1:
        submenu_bool_1 = 1;
        submenu = 1;
        break;
      case 2:
        submenu_bool_2 = 1;
        submenu = 1;
        break;
      case 3: 
        submenu_bool_3 = 1;
        submenu = 1;
        recibir_serial();
        break;
    }
  }
  while(submenu_bool_1){
    submenu = cambio_menu(submenu, 1, 7);
    menu_principal(7, submenu, menuItems_1);
    move_pos();
    if(digitalRead(SWL)==HIGH){
      tone(Zum,3000,100);
      delay(100);
      while(digitalRead(SWL)==HIGH){}
      submenu_bool_1 = 0;
    }
    if(digitalRead(SWR)==HIGH){
      tone(Zum,4000,100);
      delay(100);
      while(digitalRead(SWR)==HIGH){}
      switch(submenu){
        case 1:
          submenu_bool_1_1 = 1;
          break;
        case 2:
          submenu_bool_1_2 = 1;
          break;
        case 3:
          submenu_bool_1_3 = 1;
          break;
        case 4:
          submenu_bool_1_4 = 1;
          break;
        case 5:
          submenu_bool_1_5 = 1;
          break;
        case 6:
          submenu_bool_1_6 = 1;
          break;
        case 7:
          submenu_bool_1_7 = 1;
          break;
      }
    }
    while(submenu_bool_1_1){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      //Serial.println(pote_value);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo1.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_1 = pote_value;
        Servo1.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_1, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_1 = 0;
      }
      delay(25);
    }
    while(submenu_bool_1_2){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo2.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_2 = pote_value;
        Servo2.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(120);
        analogWrite(Servo_2, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_2 = 0;
      }
      delay(25); 
    }
    while(submenu_bool_1_3){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo3.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_3 = pote_value;
        Servo3.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_3, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_3 = 0;
      }
      delay(25); 
    }
    while(submenu_bool_1_4){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo4.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_4 = pote_value;
        Servo4.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_4, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_4 = 0;
      }
      delay(25); 
    }
    while(submenu_bool_1_5){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo5.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_5 = pote_value;
        Servo5.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_5, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_5 = 0;
      }
      delay(25); 
    }
    while(submenu_bool_1_6){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        delay(100);
        Servo6.setPeriodHertz(50);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_6 = pote_value;
        Servo6.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_6, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_6 = 0;
      }
      delay(25); 
    }
    while(submenu_bool_1_7){
      move_pos();
      pote_value = analogRead(LX);
      pote_value = map(pote_value, 0, 4095, 180, 0);
      drawProgressBar(10, 20, 100, 10, pote_value);
      if(digitalRead(SWR)==HIGH){
        tone(Zum,4000,100);
        Servo7.setPeriodHertz(50);
        delay(100);
        display.setCursor(50, 40);
        display.setTextSize(2);
        display.print("OK");
        display.setTextSize(1);
        display.display();
        pos_actual.pos_servo_7 = pote_value;
        Servo7.write(pote_value);
        delay(250);
        while(digitalRead(SWR)==HIGH){}
      }
      if(digitalRead(SWL)==HIGH){
        tone(Zum,3000,100);
        delay(100);
        analogWrite(Servo_7, 0);
        while(digitalRead(SWL)==HIGH){}
        submenu_bool_1_7 = 0;
      }
      delay(25); 
    }
  }
  while(submenu_bool_2){
    move_pos();
    submenu = cambio_menu(submenu, 1, 5);
    menu_principal(5, submenu, menuItems_2);
    if(digitalRead(SWL)==HIGH){
      tone(Zum,3000,100);
      delay(100);
      while(digitalRead(SWL)==HIGH){}
      submenu_bool_2 = 0;
    }
    if(digitalRead(SWR)==HIGH){
      tone(Zum,4000,100);
      delay(100);
      sw_time = millis();
      while(digitalRead(SWR)==HIGH){
        if((millis()-sw_time)>sw_time_option){
          save_pos = 1;
          print_ok();
        }
      }
      switch(submenu){
        case 1:
          if(save_pos==1){
            save_positions(pos_actual, posicion_1,"1");
            save_pos = 0;
          }
          else{
            mover_servos(posicion_1);
            delay(1000);
            detener_servos();
          }
          break;
        case 2: 
          if(save_pos==1){
            save_positions(pos_actual, posicion_2,"2");
            save_pos = 0;
          }
          else{
            mover_servos(posicion_2);
            delay(1000);
            detener_servos();
          }
          break;
        case 3: 
          if(save_pos==1){
            save_positions(pos_actual, posicion_3,"3");
            save_pos = 0;
          }
          else{
            mover_servos(posicion_3);
            delay(1000);
            detener_servos();
          }
          break;
        case 4:
          if(save_pos==1){
            save_positions(pos_actual, posicion_4,"4");
            save_pos = 0;
          }
          else{
            mover_servos(posicion_4);
            delay(1000);
            detener_servos();
          }
          break;
        case 5:
          if(save_pos==1){
            save_positions(pos_actual, posicion_5,"5");
            save_pos = 0;
          }
          else{
            mover_servos(posicion_5);
            delay(1000);
            detener_servos();
          }
          break;
      }
    }
  }
  while(submenu_bool_3){
    move_pos();
    if(Serial.available()){
      recibir_serial();
    }
    if(digitalRead(SWL)==HIGH){
      tone(Zum,3000,100);
      delay(100);
      while(digitalRead(SWL)==HIGH){}
      submenu_bool_3 = 0;
    }
  }
}
