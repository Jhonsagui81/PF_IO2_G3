/*
 Board I2C / TWI pins
Uno       =   A4 (SDA), A5 (SCL)
Mega2560  =   20 (SDA), 21 (SCL)
Leonardo  =   2 (SDA), 3 (SCL)
Due       =   20 (SDA), 21 (SCL), SDA1, SCL1
esta configuracion de estos pines se encuentran dentro de la librería "wire" mas info: https://www.arduino.cc/en/Reference/Wire
 */

// ------------- RFIDs ----------------
#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>  // incluye libreria especifica para mfrc5221

#define RST_PIN_1 9  // constante para referenciar pin de reset
#define SS_PIN_1 10  // constante para referenciar pin de slave select

#define RST_PIN_2 7  // constante para referenciar pin de reset
#define SS_PIN_2 8

MFRC522 mfrc522_1(SS_PIN_1, RST_PIN_1);  // crea objeto mfrc5221 enviando pines de slave select y reset
MFRC522 mfrc522_2(SS_PIN_2, RST_PIN_2);


byte LecturaUID[4];  // crea array para almacenar el UID leido
byte Usuario1[4] = { 0x93, 0xDE, 0x33, 0x96 };
byte Usuario2[4] = { 0x73, 0x9D, 0x4F, 0xA9 };
byte Usuario3[4] = { 0x0D, 0x6A, 0xB0, 0xD1 };
byte Usuario4[4] = { 0x4C, 0xF3, 0xFB, 0x31 };


byte tagarray[][4] = {
  { 0x93, 0xDE, 0x33, 0x96 },
  { 0x73, 0x9D, 0x4F, 0xA9 },
  { 0x0D, 0x6A, 0xB0, 0xD1 },  // Nota: El orden de los bytes puede variar según la implementación
  { 0x4C, 0xF3, 0xFB, 0x31 }   // Asegúrate de verificar el orden correcto
};


///------------Para LCD ------------------
#include <LiquidCrystal_I2C.h>  //DESCARGAR LIBRERÍA:https://github.com/ELECTROALL/Codigos-arduino/blob/master/LiquidCrystal_I2C.zip
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  // si no funciona se puede usar (0x3f,16,2) || (0x27,16,2)  ||(0x20,16,2)
int screenWidth = 16;


//-----------Para infrarojos --------------
const int sensor_entrada = 7;
const int sensor_salida = 6;

volatile bool flag_infra_inicio = false;
volatile bool flag_infra_intermedio = false;


// _--------------Para servo --------------
#include <Servo.h>
Servo servoMotor;


// ------------Para interrupciones ---------
const byte pinEntrada = 2;
const byte pinSalida = 3;

//Banderas entrada/salida
volatile bool flag_entrada_interno = false;
volatile bool flag_salida_interno = false;

// -----------Flags rfid activo -----------
int flag_rfid_activo = 3;


//COntador de parqueo
int ParqueoDisponible = 5;


void setup() {
  Serial.begin(9600);

  //---------RFID
  SPI.begin();  // inicializa bus SPI
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();  // inicializa modulo lector
  //----------LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Parqueo ");
  lcd.setCursor(0, 1);
  lcd.print("Disponible: " + String(ParqueoDisponible));

  //Para infrarojo
  pinMode(sensor_entrada, INPUT);
  pinMode(sensor_salida, INPUT);


  //----------Servo
  servoMotor.attach(5);
  myServo.write(70);

  //Interrupciones
  pinMode(pinEntrada, INPUT);
  pinMode(pinSalida, INPUT);


  attachInterrupt(digitalPinToInterrupt(pinEntrada), ISR_entrada, RISING);             //estao entrada
  attachInterrupt(digitalPinToInterrupt(pinSalida), ISR_Salida, RISING);               //Fichas
}

void loop() {


  if (flag_rfid_activo == 1) {
    Serial.println("Entra a 1");
    while (flag_rfid_activo == 1) {
      if (!mfrc522_1.PICC_IsNewCardPresent())  // si no hay una tarjeta presente
        continue;                             // retorna al loop esperando por una tarjeta

      if (!mfrc522_1.PICC_ReadCardSerial())  // si no puede obtener datos de la tarjeta
        continue;                           // retorna al loop esperando por otra tarjeta

      Serial.print("UID:");                           // muestra texto UID:
      for (byte i = 0; i < mfrc522_1.uid.size; i++) {  // bucle recorre de a un byte por vez el UID
        if (mfrc522_1.uid.uidByte[i] < 0x10) {         // si el byte leido es menor a 0x10
          Serial.print(" 0");                         // imprime espacio en blanco y numero cero
        } else {                                      // sino
          Serial.print(" ");                          // imprime un espacio en blanco
        }
        Serial.print(mfrc522_1.uid.uidByte[i], HEX);  // imprime el byte del UID leido en hexadecimal
        LecturaUID[i] = mfrc522_1.uid.uidByte[i];     // almacena en array el byte del UID leido
      }

      Serial.print("\t");  // imprime un espacio de tabulacion

      if (comparaUID(LecturaUID, Usuario1)) {

        // llama a funcion comparaUID con Usuario1
        Serial.println("Bienvenido Usuario 1");  // si retorna verdadero muestra texto bienvenida
                                                 //  desplazarTexto("INGRESO"," USUARIO 1",400);
        servoMotor.write(7);
        lcd.clear();
        lcd.print("USUARIO 1 ");
      } else if (comparaUID(LecturaUID, Usuario2)) {  // llama a funcion comparaUID con Usuario2
        Serial.println(" Usuario 2");                 // si retorna verdadero muestra texto bienvenida
        lcd.clear();
        lcd.print("USUARIO ADMIN");
        //  desplazarTexto("INGRESO"," USUARIO 2",400);
      } else if (comparaUID(LecturaUID, Usuario3)) {  // llama a funcion comparaUID con Usuario2
        lcd.clear();
        lcd.print("USUARIO 3 ");
        Serial.println(" Usuario 3");  // si retorna verdadero muestra texto bienvenida
        // desplazarTexto("INGRESO"," USUARIO 3",400);
      } else {                            // si retorna falso
        Serial.println("No te conozco");  // muestra texto equivalente a acceso denegado
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("NO EXISTE");
      }
      flag_rfid_activo = 3;
      mfrc522_1.PICC_HaltA();
    }
  }  //if rfid 1

  if (flag_rfid_activo == 2) {
    Serial.println("Entra a 2");
    while (flag_rfid_activo == 2) {
      if (!mfrc522_2.PICC_IsNewCardPresent())  // si no hay una tarjeta presente
        continue;                             // retorna al loop esperando por una tarjeta

      if (!mfrc522_2.PICC_ReadCardSerial())  // si no puede obtener datos de la tarjeta
        continue;                           // retorna al loop esperando por otra tarjeta

      Serial.print("UID:");                           // muestra texto UID:
      for (byte i = 0; i < mfrc522_2.uid.size; i++) {  // bucle recorre de a un byte por vez el UID
        if (mfrc522_2.uid.uidByte[i] < 0x10) {         // si el byte leido es menor a 0x10
          Serial.print(" 0");                         // imprime espacio en blanco y numero cero
        } else {                                      // sino
          Serial.print(" ");                          // imprime un espacio en blanco
        }
        Serial.print(mfrc522_2.uid.uidByte[i], HEX);  // imprime el byte del UID leido en hexadecimal
        LecturaUID[i] = mfrc522_2.uid.uidByte[i];     // almacena en array el byte del UID leido
      }

      Serial.print("\t");  // imprime un espacio de tabulacion

      if (comparaUID(LecturaUID, Usuario1)) {

        // llama a funcion comparaUID con Usuario1
        Serial.println("Bienvenido Usuario 1");  // si retorna verdadero muestra texto bienvenida
                                                 //  desplazarTexto("INGRESO"," USUARIO 1",400);
        servoMotor.write(7);
        lcd.clear();
        lcd.print("USUARIO 1 ");
      } else if (comparaUID(LecturaUID, Usuario2)) {  // llama a funcion comparaUID con Usuario2
        Serial.println(" Usuario 2");                 // si retorna verdadero muestra texto bienvenida
        lcd.clear();
        lcd.print("USUARIO ADMIN");
        //  desplazarTexto("INGRESO"," USUARIO 2",400);
      } else if (comparaUID(LecturaUID, Usuario3)) {  // llama a funcion comparaUID con Usuario2
        lcd.clear();
        lcd.print("USUARIO 3 ");
        Serial.println(" Usuario 3");  // si retorna verdadero muestra texto bienvenida
        // desplazarTexto("INGRESO"," USUARIO 3",400);
      } else {                            // si retorna falso
        Serial.println("No te conozco");  // muestra texto equivalente a acceso denegado
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("NO EXISTE");
      }
      flag_rfid_activo = 3;
      mfrc522_2.PICC_HaltA();
    }
  }
}

boolean comparaUID(byte lectura[], byte usuario[])  // funcion comparaUID
{
  for (byte i = 0; i < mfrc522_1.uid.size; i++) {  // bucle recorre de a un byte por vez el UID

    if (lectura[i] != usuario[i])  // si byte de UID leido es distinto a usuario
      return (false);              // retorna falso
  }
  return (true);  // si los 4 bytes coinciden retorna verdadero
}



void ISR_entrada() {
  flag_rfid_activo = 1;
}

void ISR_Salida() {
  flag_rfid_activo = 2;
}