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
const int sensor_entrada = 6;
const int sensor_salida = 5;
const int sensor_ficha = 15;

volatile bool flag_infra_inicio = false;
volatile bool flag_infra_intermedio = false;


// _--------------Para servo --------------
#include <Servo.h>
Servo servoMotor_E;
Servo servoMotor_S;


// ------------Para interrupciones ---------
const byte pinEntrada = 2;
const byte pinSalida = 3;


//--------------Para botones ---------------
const byte pinBotonE = 16;

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

  mfrc522_1.PCD_WriteRegister(MFRC522::RFCfgReg, 0x7F);
  mfrc522_1.PCD_WriteRegister(MFRC522::TxControlReg, 0x83); // Aumentar el amplificador
  mfrc522_1.PCD_SetAntennaGain(mfrc522_1.RxGain_max);

  mfrc522_2.PCD_WriteRegister(MFRC522::RFCfgReg, 0x7F);
  mfrc522_2.PCD_WriteRegister(MFRC522::TxControlReg, 0x83);
  mfrc522_2.PCD_SetAntennaGain(mfrc522_2.RxGain_max);


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
  pinMode(sensor_ficha, INPUT);

  //Para botones
  pinMode(pinBotonE, INPUT);

  //----------Servo
  servoMotor_E.attach(4);
  servoMotor_E.write(100);

  servoMotor_S.attach(14);
  servoMotor_S.write(100);

  //Interrupciones
  pinMode(pinEntrada, INPUT);
  pinMode(pinSalida, INPUT);


  attachInterrupt(digitalPinToInterrupt(pinEntrada), ISR_entrada, RISING);  //estao entrada
  attachInterrupt(digitalPinToInterrupt(pinSalida), ISR_Salida, RISING);    //Fichas
}

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

  if (flag_rfid_activo == 1) {
    if (ParqueoDisponible != 0) {
      // Serial.println("Entro a flujo");
      //if para boton -> entrada como externo
      if (digitalRead(pinBotonE) == HIGH) {
        //Entro como externor
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bienvenido: ");
        lcd.setCursor(0, 1);
        lcd.print("Externo");
          //Abrir la talanquera
        AbrirTalanquera_E();
        flag_infra_inicio = true;
        Serial.println("Carnet:EXT");
        Serial.println("Accion:1");
        //inicia estados
        //entra a ciclo para infrarjo
        while (flag_rfid_activo == 1) {
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
        CerrarTalanquera_E();
        //Ya leo tarjeta vuelve a normalidad
        flag_rfid_activo = 3;
      } else {
        //COn tarjeta
        if (mfrc522_1.PICC_IsNewCardPresent() && mfrc522_1.PICC_ReadCardSerial()) {
          // Show some details of the PICC (that is: the tag/card)
          for (byte i = 0; i < mfrc522_1.uid.size; i++) {  // bucle recorre de a un byte por vez el UID
            LecturaUID[i] = mfrc522_1.uid.uidByte[i];      // almacena en array el byte del UID leido
          }
          //Valida si targeta leida es aceptada o no
          bool Confirmacion_entrada = validarTarjeta(LecturaUID);

          if (Confirmacion_entrada) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Bienvenido: ");
            lcd.setCursor(0, 1);
            for (byte i = 0; i < mfrc522_1.uid.size; i++) {
              lcd.print(LecturaUID[i]);
            }
            //Abre la talanquera
            AbrirTalanquera_E();
            flag_infra_inicio = true;
            Serial.println("Carnet:"+String(LecturaUID[0]));
            Serial.println("Accion:1");

            //entra a ciclo para infrarjo
            while (flag_rfid_activo == 1) {
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
            CerrarTalanquera_E();
            //Ya leo tarjeta vuelve a normalidad
            flag_rfid_activo = 3;
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
          mfrc522_1.PICC_HaltA();
          // Stop encryption on PCD
          // mfrc522_1.PCD_StopCrypto1();

        }  //if (mfrc522[reader].PICC_IsNewC..
      }    //else de externo
    }      // if capacidad parqueo
  }        //if rfid 1

  if (flag_rfid_activo == 2) {

    if (ParqueoDisponible < 5) {
      //  Serial.println("Entra a 2");
      if (digitalRead(sensor_ficha) == LOW) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Feliz Dia: ");
        lcd.setCursor(0, 1);
        lcd.print("Externo");
        //abrir talanquera
        AbrirTalanquera_S();
        flag_infra_inicio = true;
        Serial.println("Carnet:EXT");
        Serial.println("Accion:0");

        //entra a ciclo para infrarjo
        while (flag_rfid_activo == 2) {
          //Caso 1
          if (flag_infra_inicio == true && flag_infra_intermedio == false) {
            //Esta atras de la talanquera
            if (digitalRead(sensor_entrada) == HIGH) {
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
            if (digitalRead(sensor_entrada) == HIGH) {
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
        CerrarTalanquera_S();
        //Ya leo tarjeta vuelve a normalidad
        flag_rfid_activo = 3;
      } else {
        if (mfrc522_2.PICC_IsNewCardPresent() && mfrc522_2.PICC_ReadCardSerial()) {
          ///--------------------------------
          for (byte i = 0; i < mfrc522_2.uid.size; i++) {  // bucle recorre de a un byte por vez el UID            
            LecturaUID[i] = mfrc522_2.uid.uidByte[i];     // almacena en array el byte del UID leido
          }
          //Valida si targeta leida es aceptada o no
          bool Confirmacion_salida = validarTarjeta(LecturaUID);

          if (Confirmacion_salida) {

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Feliz Dia: ");
            lcd.setCursor(0, 1);
            for (byte i = 0; i < mfrc522_2.uid.size; i++) {
              lcd.print(LecturaUID[i]);
            }
            //Abre la talanquera
            AbrirTalanquera_S();
            flag_infra_inicio = true;
            Serial.println("Carnet:"+String(LecturaUID[0]));
            Serial.println("Accion:0");

            //entra a ciclo para infrarjo
            while (flag_rfid_activo == 2) {
              //Caso 1
              if (flag_infra_inicio == true && flag_infra_intermedio == false) {
                //Esta atras de la talanquera
                if (digitalRead(sensor_entrada) == HIGH) {
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
                if (digitalRead(sensor_entrada) == HIGH) {
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
            CerrarTalanquera_S();
            //Ya leo tarjeta vuelve a normalidad
            flag_rfid_activo = 3;
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
          mfrc522_2.PICC_HaltA();
          // Stop encryption on PCD
          // mfrc522_2.PCD_StopCrypto1();
        }  //if (mfrc522[reader].PICC_IsNewC..
      } //else del salida externo
    }  //validar cantidad parqueo
  }   //rfid 2 activo
  delay(1000);
}  //loop


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

void ISR_entrada() {
  flag_rfid_activo = 1;
}

void ISR_Salida() {
  flag_rfid_activo = 2;
}

void AbrirTalanquera_E() {
  servoMotor_E.write(160);
}

void CerrarTalanquera_E() {
  servoMotor_E.write(100);
}

void AbrirTalanquera_S() {
  servoMotor_S.write(160);
}

void CerrarTalanquera_S() {
  servoMotor_S.write(100);
}