/*LIBRARIES*/
#include <SPI.h>
#include <Ethernet.h>

/*GLOBAL DECLARATIONS*/
int timestamp;
const int DTRDY = 8;      //Data ready pin of the ADC
const int ADS_CS = 9;     //Comunication control pin
const int CS = 10;        //ethernet CS
int regarray[14];         //This array saves the actual confoiguration of the ADC
signed long x, y, z;      //Contain the values of the sensor at each sample
signed long DATA[32];     //saves the values for sendig them for ethernet
int dataind = 0;          //index of the data

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  //mac of the device
byte ip[] = { 192, 168, 1, 177 };                     //device's ip
byte server[] = { 192, 168, 1, 153};                 // server ip
EthernetClient client;                                //Creates the variable that communicates with the server

void setup() {
  Serial.begin(230400);
  pinMode(ADS_CS, OUTPUT);
  digitalWrite(ADS_CS, HIGH);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  pinMode(DTRDY, INPUT);
  Ethernet.begin(mac, ip);
  delay(1000);
  Serial.println("Conectando...");
  if (client.connect(server, 2032)) {
    Serial.println("connected");
    client.println("conectado");
  }
  else {
    Serial.println("No se ha podido conectar con el servidor.\n <VOID STATE>");
    for (;;)
      ;
  }
  delay(2000);
  SPI.begin();
}

/* BEGIN OF FUNCTIONS*/

signed long ads_get_reg_value(int reg) { //falta verificar que sean 4 bites
  int response;
  int command = 0x2;
  command <<= 4;
  command |= reg;          //Se genera el comando de lectura
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);
  SPI.transfer(command);   //Se comunica el comando
  SPI.transfer(0x00);      //cantidad de bytes a leer - 1
  response = SPI.transfer(0xFF);
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
  return response;
}

void ads_get_all_regs() {
  for (int i = 0; i <= 14; i++)
    regarray[i] = ads_get_reg_value(i); //SE DEBEN MEZCLAR ESTAS FUNCIONES!?!?!?!
}

void ads_print_regarray() {//SE DEBEN MEZCLAR ESTAS FUNCIONES!?!?!?!
  String msj;
  for (int i = 0; i <= 13; i++) {
    msj = "0x0" + String(i, HEX) + " -> " + String(regarray[i], HEX);
    Serial.println(msj);
  }
}//search for streaming library

void ads_write_reg(int reg, int value) { // falta verificar los tamaños de los imputs
  int command = 0x4;
  command <<= 4;
  command |= reg;          //Se genera el comando de escritura
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);
  SPI.transfer(command);   //Se comunica el comando
  SPI.transfer(0x00);      //cantidad de bytes a escribir - 1
  SPI.transfer(value);
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}

void ads_reset() {
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);
  SPI.transfer(0x07);
  delay(10);
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}

long ads_read_once() {
  while (1) {
    if (digitalRead(DTRDY) == LOW) {
      signed long reading = 0x0;
      SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE1));
      digitalWrite(ADS_CS, LOW);   //CS negative logic
      SPI.transfer(0x13);          //Rdata once
      reading |= SPI.transfer(0xFF);// Get the first byte
      reading <<= 8;                // add bytes to total reading
      reading |= SPI.transfer(0xFF);// Get the second byte
      reading <<= 8;
      reading |= SPI.transfer(0xFF);// and so on...
      digitalWrite(ADS_CS, HIGH);
      SPI.endTransaction();
      return reading;
    }
  }
}

signed long c_t(long value) {
  if (value > 8388607) {
    value = value - 16777216;
  }
  return value;
}
/* END OF FUNCTIONS*/
String str;
/*     MAIN     */
void loop() {
  int samplecounter = 0;
  Serial.println("RESET TO POWERUP VALUES");
  ads_reset();
  ads_get_all_regs();
  ads_print_regarray();
  delay(1000);
  Serial.println("APPLY CONFIGURATION TO ADC");
  ads_write_reg(0x0, 0x08);
  ads_write_reg(0x2, 0x00);
  ads_write_reg(0x3, 0x0F);
  ads_write_reg(0x4, 0x18);  //OFFSET
  ads_write_reg(0x5, 0x30);  //OFFSET
  ads_write_reg(0x6, 0x4D);  //OFFSET
  ads_get_all_regs();
  ads_print_regarray();
  Serial.print("2 sec programed delay");
  while (samplecounter < 2) {
    Serial.print(".");
    delay(1000);
    samplecounter++;
  }
  Serial.println();
  while (HIGH) {
    digitalWrite(CS, HIGH);

    str=String(millis());
    str+=";";
    x = ads_read_once();
     str+=String( c_t(x));
    str+=";";
    ads_write_reg(0x0, 0x08);
    y = ads_read_once();
    str+=String(c_t(y));
    str+=";";
    ads_write_reg(0x0, 0x10);
    z = ads_read_once();
    str+=String( c_t(z));
    ads_write_reg(0x0, 0x18);
    client.println(str);
    Serial.print(str);
    Serial.println();
    dataind = 0;
    DATA[dataind] = '\0';
    if (!client.connected()) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      for (;;)
        ;
    }
  }
}
