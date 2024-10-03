//Para RFID
#include <SPI.h>
#include <MFRC522.h>
//Para servo
#include <Servo.h>
// PIN Numbers : RESET + SDAs
#define RST_PIN_1 9
#define SS_PIN_1 10

#define RST_PIN_2 7
#define SS_PIN_2 8


MRFC522
// List of Tags UIDs that are allowed to open the puzzle
byte LecturaUID[4];
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
// Inlocking status :
// int tagcount = 0;
// bool access = false;

#define NR_OF_READERS 2

byte ssPins[] = { SS_1_PIN, SS_2_PIN };

// Create an MFRC522 instance :
MFRC522 mfrc522[NR_OF_READERS];


//  ------------- LCD ---------------------
#include <LiquidCrystal_I2C.h>  //DESCARGAR LIBRERÍA:https://github.com/ELECTROALL/Codigos-arduino/blob/master/LiquidCrystal_I2C.zip
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // si no funciona se puede usar (0x3f,16,2) || (0x27,16,2)  ||(0x20,16,2)
int screenWidth = 16;


//--------------Para Infrarojo -------------
const int sensor_entrada = 7;
const int sensor_salida = 6;

volatile bool flag_infra_inicio = false;
volatile bool flag_infra_intermedio = false;


// -------------Para Servo ---------------
int servoPin = 5; 
Servo myServo;


// --------- Para Interrupciones ----------
//Pin para interrupciones
const byte pinEntrada = 2;
const byte pinSalida = 3;

//Banderas entrada/salida
volatile bool flag_entrada_interno = false;
volatile bool flag_salida_interno = false;


//COntador de parqueo
int ParqueoDisponible = 5;

/**
   Initialize.
*/
void setup() {

  Serial.begin(9600);  // Initialize serial communications with the PC
  while (!Serial)
    ;  // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();  // Init SPI bus

  /* Initializing Inputs and Outputs */
  // pinMode(GreenLed, OUTPUT);
  // digitalWrite(GreenLed, LOW);
  // pinMode(relayIN, OUTPUT);
  // digitalWrite(relayIN, HIGH);
  // pinMode(RedLed, OUTPUT);
  // digitalWrite(RedLed, LOW);


  /* looking for MFRC522 readers */
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
    //mfrc522[reader].PCD_SetAntennaGain(mfrc522[reader].RxGain_max);
  }

  //LCD
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

  //Para servo 
  myServo.attach(servoPin);
  // Posicion inicial del servo, palanca abajo 
  myServo.write(70);

  //Configuracion de interrupciones
  pinMode(pinEntrada, INPUT);
  pinMode(pinSalida, INPUT);

  attachInterrupt(digitalPinToInterrupt(pinEntrada), ISR_entrada, RISING);
  attachInterrupt(digitalPinToInterrupt(pinSalida), ISR_Salida, RISING);
}

/*
   Main loop.
*/

void loop() {

  if (ParqueoDisponible == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Parqueo Lleno");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Parqueo ");
    lcd.setCursor(0, 1);
    lcd.print("Disponible: " + String(ParqueoDisponible));
  }

  if (flag_entrada_interno) {
    //Escucha rfid 1
    Serial.println("Ebtr");
    if (ParqueoDisponible != 0) {
      Serial.print("Entro a flujo");
      while (flag_entrada_interno) {

        if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial()) {
          Serial.print(F("Reader "));
          Serial.print(0);

          // Show some details of the PICC (that is: the tag/card)
          Serial.print(F(": Card UID:"));
          dump_byte_array(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);


          for (byte i = 0; i < mfrc522[0].uid.size; i++) {  // bucle recorre de a un byte por vez el UID
            if (mfrc522[0].uid.uidByte[i] < 0x10) {         // si el byte leido es menor a 0x10
              Serial.print(" 0");                           // imprime espacio en blanco y numero cero
            } else {                                        // sino
              Serial.print(" ");                            // imprime un espacio en blanco
            }
            Serial.print(mfrc522[0].uid.uidByte[i], HEX);  // imprime el byte del UID leido en hexadecimal
            LecturaUID[i] = mfrc522[0].uid.uidByte[i];     // almacena en array el byte del UID leido
          }
          //Valida si targeta leida es aceptada o no
          bool Confirmacion_entrada = validarTarjeta(LecturaUID);

          if (Confirmacion_entrada) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bienvenido: ");
            lcd.setCursor(0, 1);
            for (byte i = 0; i < mfrc522[0].uid.size; i++) {
              lcd.print(LecturaUID[i]);
            }
            //Abre la talanquera
            AbrirTalanquera();
            flag_infra_inicio = true;

            //entra a ciclo para infrarjo
            while (flag_entrada_interno) {
              //Caso 1
              if (flag_infra_inicio == true && flag_infra_intermedio == false) {
                //Esta atras de la talanquera
                if (digitalRead(sensor_salida) == HIGH) {
                  //Aun no se detecta movimiento
                  continue;
                } else {
                  //Se detecta movimiento
                  flag_infra_intermedio = true;
                  continue;
                }
              }
              //Caso 2
              if (flag_infra_inicio == true && flag_infra_intermedio == true) {
                if (digitalRead(sensor_salida) == HIGH) {
                  //Dejo de detectar movimiento (Ya paso el carro)
                  flag_infra_inicio = false;
                  flag_infra_intermedio = false;
                  break;
                } else {
                  //sigue detectando la presencia del carro
                  continue;
                }
              }
            }
            //Descuenta contadodr parqueos
            ParqueoDisponible -= 1;
            //Baja la talanquera
            CerrarTalanquera();
            //Ya leo tarjeta vuelve a normalidad
            flag_entrada_interno = false;
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Tarjeta invalida");
            lcd.setCursor(0, 1);
            lcd.print("Intente de nuevo");
          }



          /*Serial.print(F("PICC type: "));
        MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
        Serial.println(mfrc522[reader].PICC_GetTypeName(piccTypelcd));*/
          // Halt PICC
          mfrc522[0].PICC_HaltA();
          // Stop encryption on PCD
          // mfrc522[0].PCD_StopCrypto1();
        }  //if (mfrc522[reader].PICC_IsNewC..
      }
    }

    Serial.println("Finaliza escucha 1");

  } else if (flag_salida_interno) {
    //Escucha rfid 2
    Serial.println("Entra a salidad");
    
    while (flag_salida_interno) {
      if (mfrc522[1].PICC_IsNewCardPresent() && mfrc522[1].PICC_ReadCardSerial()) {
        Serial.print(F("Reader "));
        Serial.print(1);

        // Show some details of the PICC (that is: the tag/card)
        Serial.print(F(": Card UID:"));
        dump_byte_array(mfrc522[1].uid.uidByte, mfrc522[1].uid.size);
        // lcd.clear();
        // lcd.print(" Card UID:" + String(1));

        ///--------------------------------
        for (byte i = 0; i < mfrc522[1].uid.size; i++) {  // bucle recorre de a un byte por vez el UID
            if (mfrc522[1].uid.uidByte[i] < 0x10) {         // si el byte leido es menor a 0x10
              Serial.print(" 0");                           // imprime espacio en blanco y numero cero
            } else {                                        // sino
              Serial.print(" ");                            // imprime un espacio en blanco
            }
            Serial.print(mfrc522[1].uid.uidByte[i], HEX);  // imprime el byte del UID leido en hexadecimal
            LecturaUID[i] = mfrc522[1].uid.uidByte[i];     // almacena en array el byte del UID leido
          }
          //Valida si targeta leida es aceptada o no
          bool Confirmacion_salida = validarTarjeta(LecturaUID);

          if (Confirmacion_salida) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Feliz Dia: ");
            lcd.setCursor(0, 1);
            for (byte i = 0; i < mfrc522[1].uid.size; i++) {
              lcd.print(LecturaUID[i]);
            }
            //Abre la talanquera
            AbrirTalanquera();
            flag_infra_inicio = true;

            //entra a ciclo para infrarjo
            while (flag_salida_interno) {
              //Caso 1
              if (flag_infra_inicio == true && flag_infra_intermedio == false) {
                //Esta atras de la talanquera
                if (digitalRead(sensor_salida) == HIGH) {
                  //Aun no se detecta movimiento
                  continue;
                } else {
                  //Se detecta movimiento
                  flag_infra_intermedio = true;
                  continue;
                }
              }
              //Caso 2
              if (flag_infra_inicio == true && flag_infra_intermedio == true) {
                if (digitalRead(sensor_salida) == HIGH) {
                  //Dejo de detectar movimiento (Ya paso el carro)
                  flag_infra_inicio = false;
                  flag_infra_intermedio = false;
                  break;
                } else {
                  //sigue detectando la presencia del carro
                  continue;
                }
              }
            }
            //Aumenta parqueos contadodr parqueos
            ParqueoDisponible += 1;
            //Baja la talanquera
            CerrarTalanquera();
            //Ya leo tarjeta vuelve a normalidad
            flag_salida_interno = false;
          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Tarjeta invalida");
            lcd.setCursor(0, 1);
            lcd.print("Intente de nuevo");
          }
        ///-----------------------------

        /*Serial.print(F("PICC type: "));
        MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
        Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));*/
        // Halt PICC
        mfrc522[1].PICC_HaltA();
        // Stop encryption on PCD
        // mfrc522[1].PCD_StopCrypto1();
      }  //if (mfrc522[reader].PICC_IsNewC..
    }
  }
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/

bool validarTarjeta(byte uidLeido[]) {
  // Recorre el arreglo de tarjetas autorizadas
  for (int i = 0; i < sizeof(tagarray) / sizeof(tagarray[0]); i++) {
    bool esIgual = true;
    // Compara cada byte del UID leído con el UID de la tarjeta actual
    for (int j = 0; j < 4; j++) {
      if (uidLeido[j] != tagarray[i][j]) {
        esIgual = false;
        break;
      }
    }
    // Si todos los bytes coinciden, la tarjeta es válida
    if (esIgual) {
      return true;
    }
  }
  // Si no se encontró ninguna coincidencia, la tarjeta no es válida
  return false;
}

void dump_byte_array(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void ISR_entrada() {
  flag_entrada_interno = true;
}

void ISR_Salida() {
  flag_salida_interno = true;
}

void AbrirTalanquera() {
  myServo.write(160);
}

void CerrarTalanquera(){
  myServo.write(70);
}
