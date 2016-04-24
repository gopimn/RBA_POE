#include <util.h>

/* WELCOME TO GOPILABZ.
   THIS PROGRAM IS MADE TO TEST THE SINCRONIZATION TROUGH ETHERNET OF TWO
   DATA ADQUISITION UNITS.
   THIS IS THE SLAVE CODE.
   HARDWARE: ARDUINO DUE + ETHERNET SHIELD REV3
*/


/*LIBRARIES*/
#include <SPI.h>
#include <Scheduler.h>
#include <Ethernet.h>

/*VARIABLES*/

//CLOCK
int clockPin = 22;
bool toogle_flag = 0; // 0 apagado 1 prendido
unsigned long local_time_stamp = 0;
//NETWORK
int master_port = 2034;
byte mac_local[] = { 0x90, 0xA2, 0xDA, 0x10, 0x39, 0x6B};
byte ip_local[] = { 192, 168, 255, 3};
byte ip_master[] = { 192, 168, 255, 2};
EthernetClient client;


signed long master_time_stamp, TS1, TS2, TS3, TS4, TS5, TM1 = 0, TM2 = 0, TM3 = 0, TM4 = 0, TM5 = 0;
signed long comm_delay = 0;
long offset = 0;
byte ctrl_byte = 0;
int i = 0;
byte command = B101;



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


/*SET UP*/
void setup() {
  Serial.begin(115200);
  Serial.println("IM AT YOUR ORDERS, IM A SLAVE");
  Serial.println("FILENAME: 'package_0005'");
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  Serial.println("mira el osciloscopio xD");
  pinMode(ADS_CS, OUTPUT);
  digitalWrite(ADS_CS, HIGH);
  pinMode(DTRDY, INPUT);
  for (int index = 0; index < 4; index++) {
    Serial.print(".");
    delay(500);
  }  Serial.println();

  Serial.println("DHCP request sended");
  Ethernet.begin(mac_local);
  Serial.print("The ip adress is: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Trying to connect to server at: ");

  for (int index = 0; index < 4; index++) {
    Serial.print(ip_master[index]);
    if (index < 3)
      Serial.print(".");
  }  Serial.println();

  Serial.print("Port: ");
  Serial.println(master_port, DEC);
  if (client.connect(ip_master, master_port)) {
    Serial.println("Succesfully connected.");
  }
  else {
    Serial.println("Couldn't connect to server");
    Serial.println("VOID STATE, PLZ REBOOT");
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
 
  /* turn on the timer clock in the power management controller */
  pmc_set_writeprotect(false);       // disable write protection for pmc registers
  pmc_enable_periph_clk(ID_TC7);     // enable peripheral clock TC7

  /* we want wavesel 01 with RC */
  TC_Configure(/* clock */TC2,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_SetRC(TC2, 1, 840);//20microseg
  TC_Start(TC2, 1);

  // enable timer interrupts on the timer
  TC2->TC_CHANNEL[1].TC_IER = TC_IER_CPCS; // IER = interrupt enable register
  TC2->TC_CHANNEL[1].TC_IDR = ~TC_IER_CPCS; // IDR = interrupt disable register

  /* Enable the interrupt in the nested vector interrupt controller */
  /* TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1 */
  NVIC_EnableIRQ(TC7_IRQn);
  //Scheduler.startLoop(clockmeup);

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

unsigned long givemeThemicros (unsigned long value) {
  return (value * 20 / 840);
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
      Serial.println(msg.length());
      Serial.println(msg);
      //client_pc.print(msg);
      Serial.println("msg1");
      msg_rdy = 0;
    }
    else if (msg2_rdy == 1)
    {
      Serial.println(msg2.length());
      Serial.println(msg2);
      Serial.println("msg2");
      //client_pc.print(msg2);
      msg2_rdy = 0;
    }
  }
  if (millis() % 50 == 0) {
    TS3 = local_time_stamp;
    client.write(command);
    if (client.available() > 0) {
      Serial.println(i++);
      if (i < 4) {
        TM1 |= client.read();
        TM1 <<= 8;
        TM1 |= client.read();
        TM1 <<= 8;
        TM1 |= client.read();
        TM1 <<= 8;
        TM1 |= client.read();
        TS1 = local_time_stamp;
        TM1 = htonl(TM1);
        Serial.print("\t\tTM1 ");
        Serial.println(TM1, DEC);
        Serial.print("\t\tTS1 ");
        Serial.println(TS1, DEC);
        offset = TS1 - TM1 - comm_delay;
        local_time_stamp -= offset;
        Serial.print("\t\tOFFSET ");
        Serial.println(offset);
        TM2 = TM1;
        TS2 = TS1;
      }

      else if (i == 4) {
        TM3 |= client.read();
        TM3 <<= 8;
        TM3 |= client.read();
        TM3 <<= 8;
        TM3 |= client.read();
        TM3 <<= 8;
        TM3 |= client.read();
        TM3 = htonl(TM3);
        Serial.print("\t\tTM2 ");
        Serial.println(TM2, DEC);
        Serial.print("\t\tTS2 ");
        Serial.println(TS2, DEC);
        Serial.print("\t\tTM3 ");
        Serial.println(TM3);
        Serial.print("\t\tTS3 ");
        Serial.println(TS3);
        Serial.print("\t\tDELAY ");
        comm_delay = ((TS2 - TM2) + (TM3 - TS3)) / 2;
        Serial.println(((TS2 - TM2) + (TM3 - TS3)) / 2);
        Serial.println("#####################################");
        i = 0;
      }
      TM1 = 0;
      TM3 = 0;
    }
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
