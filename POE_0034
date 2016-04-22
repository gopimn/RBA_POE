/* WELCOME TO GOPILABZ.
   THIS PROGRAM IS MADE TO TEST THE SINCRONIZATION TROUGH ETHERNET OF TWO
   DATA ADQUISITION UNITS.
   THIS IS THE MASTER CODE.
   HARDWARE: ARDUINO DUE + ETHERNET SHIELD 2
*/


/*LIBRARIES*/
#include <SPI.h>
#include <Scheduler.h>
#include <Ethernet2.h>

/*VARIABLES*/

//CLOCK
int clockPin = 22;
bool toogle_flag = 0; // 0 apagado 1 prendido
unsigned long local_time_stamp = 0;

//NETWORK
int master_port = 2034;
byte mac_local[] = { 0x90, 0xA2, 0xDA, 0x10, 0x39, 0x6A };
byte ip_local[] = { 192, 168, 255, 2};
byte ip_pc[] = { 192, 168, 255, 3};// modificar

EthernetServer server(master_port);
EthernetClient client;

EthernetClient client_pc;
unsigned long aux = 4261148655ul;
byte a1, a2, a3, a4;
int i = 0;


//ADC
const int DTRDY = 9;      //Data ready pin of the ADC
const int ADS_CS = 8;     //Comunication control pin
const int CS = 10;        //ethernet CS
int regarray[14];         //This array saves the actual confoiguration of the ADC
String msg = "";
String msg2 = "";
int msg_rdy = 0;
int msg2_rdy = 0;
int msgflag = 0;

//DAQ
signed long x, y, z; //Contain the values of the sensor at each sample
int dataind1 = 0;          //index of the data
int index_adq = 0;
int dataind2 = 0;          //index of the data

void toogle() {
  if (toogle_flag == 1) {
    digitalWrite(clockPin, LOW);
    toogle_flag = 0;
  }
  else if (toogle_flag == 0) {
    digitalWrite(clockPin, HIGH);
    toogle_flag = 1;
  }
}

void ads_reset() {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);
  SPI.transfer(0x07);
  delay(10);
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}

signed long ads_get_reg_value(int reg) { //falta verificar que sean 4 bites
  int response;
  int command = 0x2;
  command <<= 4;
  command |= reg;          //Se genera el comando de lectura
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
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
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);
  SPI.transfer(command);   //Se comunica el comando
  SPI.transfer(0x00);      //cantidad de bytes a escribir - 1
  SPI.transfer(value);
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}



long ads_read_once() {
  while (1) {
    if (digitalRead(DTRDY) == LOW) {
      signed long reading = 0x0;
      SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
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



unsigned long givemeThemillis (unsigned long value) {
  return (value * 20 / 840);
}


signed long c_t(long value) {
  if (value > 8388607) {
    value = value - 16777216;
  }
  return value;
}
/*SET UP*/
void setup() {
  Serial.begin(115200);
  Serial.println("BIENVENIDO! I'M THE MASTER OF DARKNESS!!!");
  Serial.println("FILENAME: 'POE_0034_TCP_syncMaster'");
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  Serial.println("mira el osciloscopio xD");
  pinMode(ADS_CS, OUTPUT);
  digitalWrite(ADS_CS, HIGH);
  pinMode(DTRDY, INPUT);
  for (int index = 0; index < 4; index++) {
    Serial.println(".");
    delay(500);
  }
  Serial.println();
  Serial.println("DHCP request sended");
  Ethernet.begin(mac_local);
  Serial.print("The ip adress is: ");
  Serial.println(Ethernet.localIP());

  server.begin();

  Serial.print("The server is at port: ");
  Serial.println(master_port, DEC);
  Serial.println("tratando de conectarse al PC en la direccion ");
  Serial.println(ip_pc[3]);
  if (client_pc.connect(ip_pc, 2032)) {
    Serial.println("connected");
  }
  else {
    Serial.println("DRAMA MY MAN, plz reboot");
    while (HIGH);
  }



  SPI.begin();
  Serial.println("RESET TO POWERUP VALUES");
  ads_reset();
  ads_get_all_regs();
  ads_print_regarray();
  delay(1000);
  Serial.println("APPLY CONFIGURATION TO ADC");
  ads_write_reg(0x0, 0x08);
  ads_write_reg(0x2, 0x00);
  ads_write_reg(0x3, 0x07);  //640 SPS
  ads_write_reg(0x4, 0x00);  //OFFSET 18
  ads_write_reg(0x5, 0x00);  //OFFSET 30
  ads_write_reg(0x6, 0x00);  //OFFSET 4D
  ads_get_all_regs();
  ads_print_regarray();
  Serial.println("_____________________________________");
  //  Serial.println("probando, while (HIGH);");
  //  while (HIGH);
  /*INICIO CONFIGURACION DEL TIMER TC7*/
  /* turn on the timer clock in the power management controller */
  pmc_set_writeprotect(false);       // disable write protection for pmc registers
  pmc_enable_periph_clk(ID_TC7);     // enable peripheral clock TC7

  /* we want wavesel 01 with RC */
  TC_Configure(/* clock */TC2,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_SetRC(TC2, 1, 840);
  TC_Start(TC2, 1);

  //enable timer interrupts on the timer
  TC2->TC_CHANNEL[1].TC_IER = TC_IER_CPCS; // IER = interrupt enable register
  TC2->TC_CHANNEL[1].TC_IDR = ~TC_IER_CPCS; // IDR = interrupt disable register

  /* Enable the interrupt in the nested vector interrupt controller */
  /* TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1 */
  NVIC_EnableIRQ(TC7_IRQn);
  //
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);   //CS negative logic
  SPI.transfer(0x40); //escribir en el registro 0, mux0
  SPI.transfer(0x00); //un registro
  SPI.transfer(0x08); //eje x
  SPI.transfer(0x15);          //Rdata cont
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
  SPI.usingInterrupt(digitalPinToInterrupt(DTRDY));
  attachInterrupt(digitalPinToInterrupt(DTRDY), adq, FALLING);
  Serial.println("_____________________________________");

}

/* TOOGLE
   THIS FUNCTION SETS A PIN HIGH OR LOW,
   DEPENDING ON THE PREVIOUS VALUE.

*/
void adq() {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE1));
  digitalWrite(ADS_CS, LOW);   //CS negative logic
  switch (index_adq)
  {
    case 0:
      x = 0;
      x |= SPI.transfer(0xFF);// Get the first byte
      x <<= 8;                // add bytes to total reading
      x |= SPI.transfer(0xFF);// Get the second byte
      x <<= 8;
      x |= SPI.transfer(0xFF);// and so on...
      SPI.transfer(0x40);
      SPI.transfer(0x00);
      SPI.transfer(0x10);
      index_adq = 1;
      break;
    case 1:
      y = 0;
      y |= SPI.transfer(0xFF);// Get the first byte
      y <<= 8;                // add bytes to total reading
      y |= SPI.transfer(0xFF);// Get the second byte
      y <<= 8;
      y |= SPI.transfer(0xFF);// and so on...
      SPI.transfer(0x40);
      SPI.transfer(0x00);
      SPI.transfer(0x18);
      index_adq = 2;
      break;
    case 2:
      z = 0;
      z |= SPI.transfer(0xFF); // Get the first byte
      z <<= 8;                // add bytes to total reading
      z |= SPI.transfer(0xFF);// Get the second byte
      z <<= 8;
      z |= SPI.transfer(0xFF);// and so on...
      SPI.transfer(0x40);
      SPI.transfer(0x00);
      SPI.transfer(0x08);
      index_adq = 0;
      break;
  }
  digitalWrite(ADS_CS, HIGH);
  SPI.endTransaction();
}




void loop() {
  if (millis() % 15 == 0) {
    if (msg_rdy == 1) {
      //  Serial.println(msg.length());
      //  Serial.println(msg);
      client_pc.print(msg);
      //  Serial.println("msg1");
      msg_rdy = 0;
    }
    else if (msg2_rdy == 1)
    {
      //    Serial.println(msg2.length());
      //  Serial.println(msg2);
      //   Serial.println("msg2");
      client_pc.print(msg2);
      msg2_rdy = 0;
    }
  }
  if (millis() % 2000 == 0) {
    client = server.available();
    if (client.connected()) {
      i++;
      Serial.println(i);
      if (i < 4) {
        Serial.print("\t\tTM1 ");
        aux = local_time_stamp;
        a1 = aux;
        a2 = aux >> 8;
        a3 = aux >> 16;
        a4 = aux >> 24;
        Serial.println(aux, DEC);
        client.write(a1);
        client.write(a2);
        client.write(a3);
        client.write(a4);
      }
      if (i == 4) {
        Serial.print("\t\tDELAY MEASUREMENT ");
        aux = local_time_stamp;
        a1 = aux;
        a2 = aux >> 8;
        a3 = aux >> 16;
        a4 = aux >> 24;
        Serial.println(aux, DEC);
        client.write(a1);
        client.write(a2);
        client.write(a3);
        client.write(a4);
        Serial.println("#####################################33");
        i = 0;
      }
    }
    else
      Serial.println("UNABLE TO CONNECT");
  }
}




void TC7_Handler() {
  // We need to get the status to clear it and allow the interrupt to fire again
  TC_GetStatus(TC2, 1);
  if (++local_time_stamp % 200 == 0) {//4 ms
    toogle();
    if (msgflag == 0)
    {
      msg += String((local_time_stamp));
      msg += ";";
      msg += String(x, HEX);
      msg += ";";
      msg += String(y, HEX);
      msg += ";";
      msg += String(z, HEX);
      msg += "\n";
      dataind1++;
      if (dataind1 == 5) {
        msg2 = "";
        msg_rdy = 1;
        dataind1 = 0;
        msgflag = 1;
      }
    }
    else if (msgflag == 1)
    {
      msg2 += String((local_time_stamp));
      msg2 += ";";
      msg2 += String(x, HEX);
      msg2 += ";";
      msg2 += String(y, HEX);
      msg2 += ";";
      msg2 += String(z, HEX);
      msg2 += "\n";
      dataind2++;
      if (dataind2 == 5) {
        msg = "";
        msg2_rdy = 1;
        dataind2 = 0;
        msgflag = 0;
      }
    }
  }
}
