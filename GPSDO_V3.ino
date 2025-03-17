
/******************************************************************************************************
* GPSDO By LU9DA - 2025
*
* Este fuente esta disponible para uso gratuito por radioaficionados, no asi para uso comercial
* Si la idea es vender algo con todo o parte de este fuente, debe contactar primero a LU9DA
* (lu9da@lu9da.org) para obtener atuorizacion escrita.
*
* Partes de este trabajo se basa el lo programado por F2DC en su gpsdo, por ejemplo las rutinas de
* medicion de frecuencia por interrupt, a el va el agradecimiento por hacer disponible su trabajo
*
*******************************************************************************************************
* MicroLCD library
* For more information, please visit https://github.com/stanleyhuangyc/MultiLCD/tree/master/MicroLCD
* en la libreria, en microLCD.h, descomentar el #define MEMORY_SAVING
*******************************************************************************************************
* la libreria SI5351 es la de Etherkit!
*******************************************************************************************************
* encoder library https://github.com/PaulStoffregen/Encoder
*******************************************************************************************************/

//*********** INCLUDES

#include <SoftwareSerial.h>
#include <Wire.h>
#include <MicroLCD.h>
#include <si5351.h>
#include <Encoder.h>
#include <EEPROM.h>

//*********** END INCLUDES

//***************************************

//*********** DEFINES

#define pps_Pin      2  // 1pps
#define encoder0PinA 6
#define encoder0PinB 7
#define ENCODER_BTN  4  // boton

//*********** END DEFINES

//***************************************

//*********** INSTANCES

//LCD_SH1106 lcd; /* for SH1106 OLED module */
LCD_SH1106 lcd; /* for SSD1306 OLED module */

// The Si5351 instance.
Si5351 si5351;

SoftwareSerial gpsSerial(8,9); //RX, TX pins for GPS module


// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(6, 7);

//*********** END INSTANCES

//***************************************

//*********** VARIABLES

String instring="";

String lat;
String latlong;

unsigned long pps_correct;
unsigned long mult = 0;
unsigned long XtalFreq = 100000000;
unsigned long XtalFreq_old = 100000000;
unsigned long Freq1 = 12500000;
unsigned long memoria_1;
unsigned long memoria_2;

char Xtal_frec[10];

long correction = 164000;
long stab;
long newPosition;
long oldPosition = -999;

unsigned int tcount = 0;
unsigned int tcount2 = 0;

byte pps_valid = 1;
byte stab_count = 44;
byte new_freq = 1;
byte menu = 0;
byte cursor_ = 48;

bool i2c_found = false;

float stab_float = 1000;

//*********** END VARIABLES

//***************************************

//****************** SETUP

void setup()
{ 
 
  Serial.begin(115200);
  gpsSerial.begin(9600);


  lcd.begin();
  lcd.clear();

  lcd.setCursor(43, 2);
  lcd.println(" GPSDO ");
  lcd.setCursor(17, 5);
  lcd.println("(c) LU9DA - 2025");

  delay(750);

  if (digitalRead(ENCODER_BTN) == LOW) {
    eeprom_erase();
  }

  delay(750);

  newPosition = myEnc.read();

  if (newPosition != oldPosition) {
    oldPosition = newPosition;
  }



  if (EEPROM.read(254) != 170 && EEPROM.read(253) != 85) {
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.println("     eeprom error!");
    while(1){}
  }

  EEPROM.get(0, memoria_1);
  EEPROM.get(4, memoria_2);

  i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25000000UL, 0);



  if(!i2c_found){  // test del SI5351
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.println("     eeprom error!");
    lcd.println("SI5351 not found!");
    while(1){}

  }
  

  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.output_enable(SI5351_CLK1, 0);
  si5351.output_enable(SI5351_CLK2, 0);
   
  
  gpsSerial.listen();

  


  TCCR1B = 0;                                    //Disable Timer5 during setup
  TCCR1A = 0;                                    //Reset
  TCNT1  = 0;                                    //Reset counter to zero
  TIFR1  = 1;                                    //Reset overflow
  TIMSK1 = 1;                                    //Turn on overflow flag

  pinMode(pps_Pin, INPUT_PULLUP);                        // Inititalize GPS 1pps input
  pinMode(LED_BUILTIN, OUTPUT);


  si5351.set_ms_source(SI5351_CLK1, SI5351_PLLB);
  si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);
  
  si5351.set_freq(250000000ULL, SI5351_CLK0);  //2,5 mhz
  si5351.output_enable(SI5351_CLK0, 1);        //enable clk0
                 
  si5351.set_freq(memoria_1 * 100ULL, SI5351_CLK1);  //memoria 1
  si5351.output_enable(SI5351_CLK1, 1);        //enable clk1

  si5351.set_freq(memoria_2 * 100ULL, SI5351_CLK2);  //memoria 2
  si5351.output_enable(SI5351_CLK2, 1);        //enable clk2

  lcd.clear();

  lcd.setCursor(0, 0);

  attachInterrupt(0, PPSinterrupt, RISING);
  TCCR1B = 0;

}

//****************** END SETUP


//****************** LOOP


void loop(){

  //********* gps

  if (gpsSerial.available()){

    instring=gpsSerial.readStringUntil('\n');



    if (instring.indexOf("GPRMC") > 0){

      instring = instring.substring(instring.indexOf("$GPRMC"));

      lcd.setCursor(0, 7);

      byte x = 0;
      instring.replace(","," ,");
      char buf[instring.length()+1];
      instring.toCharArray(buf, sizeof(buf));
      char *token;
      char *pter = buf;
     
       while ((token = strtok_r(pter, ",", &pter))){

        switch(x){

          case 1:
            Serial.print("hora: ");
            Serial.println(token);
            lcd.print(String(token).substring(0,2));
            lcd.print(":");
            lcd.print(String(token).substring(2,4));
            lcd.print(":");
            lcd.print(String(token).substring(4,6));
          break;

          case 2:  
            Serial.print("validez: ");
            Serial.println(token);
          break;

          case 3:  

            Serial.print("lat: ");
            Serial.print(token);
            //Serial.print(" ");
            //Serial.print(GpsToDecimalDegrees(token, 'S'));
            latlong="lat: ";
            latlong=latlong + token; 

          break;

          case 4:  
            Serial.print(" ");
            Serial.println(token);
            latlong=latlong + token; 
          break;

          case 5:  
            Serial.print("lon: ");
            Serial.print(token);
            //Serial.print(" ");
            //Serial.print(GpsToDecimalDegrees(token, 'W'));            
            latlong=latlong+"\n\nlon: ";
            latlong=latlong + token;
          break;

          case 6:  
            Serial.print(" ");
            Serial.println(token);
            latlong=latlong + token; 
          break;

          case 9:  
            Serial.print("fecha: ");
            Serial.println(token);
            lcd.print(" | ");
            lcd.print(String(token).substring(0,2));
            lcd.print("/");
            lcd.print(String(token).substring(2,4));
            lcd.print("/20");
            lcd.print(String(token).substring(4,6));
          break;

        }


        x++;   

      }


     
    }


    instring="";
  
  }
  
  //**********fin gps

  //**********sel menu

  if(menu == 4){
    menu=0;
  }

  if (digitalRead(ENCODER_BTN) == LOW) {
    delay(100);
    if (digitalRead(ENCODER_BTN) == LOW) {
      while(digitalRead(ENCODER_BTN) == LOW){}

        menu++;

        for (int j = 1; j < 6; j++){
          lcd.setCursor(0,j);
          lcd.print("                     ");  
        }

      

    }

  }    

  //*********fin sel menu



  si5351.set_correction(correction, SI5351_PLL_INPUT_XO);


  switch(menu){

    case 0:
      
      lcd.setCursor(0, 2);
      lcd.print(F("Xtal: "));
      ultoa(XtalFreq,Xtal_frec,10);

      if(XtalFreq < 100000000){
    
      for (int j=8;j>0;--j){
        Xtal_frec[j] = Xtal_frec[j-1];
      }

      Xtal_frec[0]=32;

      }
      lcd.print(Xtal_frec[0]);
      lcd.print(Xtal_frec[1]);
      lcd.print(Xtal_frec[2]);
      lcd.print(".");
      lcd.print(Xtal_frec[3]);
      lcd.print(Xtal_frec[4]);
      lcd.print(Xtal_frec[5]);
      lcd.print(".");    
      lcd.print(Xtal_frec[6]);
      lcd.print(Xtal_frec[7]);
      lcd.print(Xtal_frec[8]);

      lcd.print(F(" mHz "));

      lcd.setCursor(0,4);
      lcd.print(F("corr: "));
      lcd.print(correction);
      lcd.print(F(" ppm  "));



    break;


    case 1:
      
      lcd.setCursor(0,2);
      lcd.println(latlong);


    break;

    case 3:

      memoria_2 = edita2(memoria_2,3,2);
      EEPROM.put(4, memoria_2);
      delay(50);
      EEPROM.get(4, memoria_2);
      delay(50);
      si5351.set_freq(memoria_2 * 100ULL, SI5351_CLK2);  //memoria 2

      
    break;

    case 2:
      
      memoria_1 = edita(memoria_1,3,1);
      EEPROM.put(0, memoria_1);
      delay(50);
      EEPROM.get(0, memoria_1);
      delay(50);
      si5351.set_freq(memoria_1 * 100ULL, SI5351_CLK1);  //memoria 1   
        

    break;




  }    


  if (tcount2 != tcount) {

    tcount2 = tcount;
    pps_correct = millis();
  
  }

  if (millis() > pps_correct + 1200) {  //controla que haya pps
  
    pps_valid = 0;
    pps_correct = millis();

  }

  if(pps_valid != 0){
    lcd.setCursor(90,0);
    lcd.print("  Sat");
  }else{
    lcd.setCursor(90,0);
    lcd.print("NOSat");
  }

  lcd.setCursor(0, 0); 
  lcd.print(tcount);
  lcd.print(" ");




}

//****************** END LOOP

//****************** INTERRUPTS

//************* INTERRUPT  1PPS
void PPSinterrupt(){
  
  pps_valid = 1;  

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  tcount++;
  //stab_count--;


  //if (tcount == 4){                              // Start counting the 2.5 MHz signal from Si5351A CLK0
  if (tcount == 2){

    TCCR1B = 7;                                  //Clock on rising edge of pin 5
      
  }

  //if (tcount == 44){                             //The 40 second gate time elapsed - stop counting
  if (tcount == 42){  
    TCCR1B = 0;                                  //Turn off counter
    
    if (pps_valid == 1) {
      XtalFreq_old = XtalFreq;
      XtalFreq = mult * 0x10000 + TCNT1;           //Calculate correction factor
      new_freq = 1;
    }

    TCNT1 = 0;                                   //Reset count to zero
    mult = 0;
    tcount = 0;                                  //Reset the seconds counter

    pps_valid = 1;
  
    //stab_count = 44;

    stab = XtalFreq - 100000000;
    stab = stab * 10 ;
  
    if (stab > 100 || stab < -100) {
      correction = correction + stab;
    }else if (stab > 20 || stab < -20) {
      correction = correction + stab / 2;
    }else{
      correction = correction + stab / 4;
    }  
    
  }
  

}

//************* end INTERRUPT  1PPS

//************* Timer 1 overflow intrrupt vector.

ISR(TIMER1_OVF_vect){
  mult++;                                          //Increment multiplier
  TIFR1 = (1 << TOV1);                             //Clear overlow flag
}

//************* end Timer 1 overflow intrrupt vector.


//****************** END INTERRUPTS

//************* FUNCIONES

/**
 * Convert NMEA absolute position to decimal degrees
 * "ddmm.mmmm" or "dddmm.mmmm" really is D+M/60,
 * then negated if quadrant is 'W' or 'S'
 */
float GpsToDecimalDegrees(const char* nmeaPos, char quadrant)
{
  float v= 0;
  if(strlen(nmeaPos)>5)
  {
    char integerPart[3+1];
    int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    memcpy(integerPart, nmeaPos, digitCount);
    integerPart[digitCount]= 0;
    nmeaPos+= digitCount;
    v= atoi(integerPart) + atof(nmeaPos)/60.;
    if(quadrant=='W' || quadrant=='S')
      v= -v;
  }
  return v;
}




// *************   eeprom erase & init
void eeprom_erase() {

  lcd.clear();

  lcd.print("     Memory INIT");

  EEPROM.put(0, 10000000UL);  // 10.000.000,00
  EEPROM.put(4, 10000000UL);

  delay(250);

  EEPROM.write(254, 170);
  EEPROM.write(253, 85);

  delay(250);

  lcd.clear();

  lcd.print("     Done");

  delay(250);

  lcd.clear();

}

//********** fin eeprom erase & init init

//********** edita

unsigned long edita(unsigned long frecuencia, byte linea, byte chan) {
  //void edita(unsigned long frecuencia, byte linea){

    
  boolean ciclo=true;
  unsigned long long_press = 0;
  char frec_char[8] = "*";

  oldPosition = newPosition = myEnc.read() / 4;

  //frecuencia*=100;
  ultoa(frecuencia,frec_char,10);


  while (frec_char[7] < 48 || frec_char[7] > 57){

    for (byte i=7; i>0; i--){
      frec_char[i] = frec_char[i-1];
    }

    frec_char[0] = 48;

  }

  
  lcd.setCursor(0, linea);
  lcd.print("M");
  lcd.print(chan);
  
  cursor_ = 24;
  lcd.setCursor(cursor_, linea);
  lcd.print(frec_char[0]);
  lcd.print(frec_char[1]);
  lcd.print(".");
  lcd.print(frec_char[2]);
  lcd.print(frec_char[3]);
  lcd.print(frec_char[4]);
  lcd.print(".");
  lcd.print(frec_char[5]);
  lcd.print(frec_char[6]);
  lcd.print(frec_char[7]);  
  lcd.setCursor(cursor_, linea + 1);
  lcd.print("-");
  lcd.setCursor(90, linea);
  lcd.print("<");

 
  while(ciclo){

  newPosition = myEnc.read() / 4;

    if (newPosition != oldPosition) {
      if (newPosition > oldPosition) {

        switch (cursor_) {
          case 24:
            frec_char[0] = frec_char[0] + 1;
            if (frec_char[0] > 57){
              frec_char[0] = 48;
            }
            break;

          case 30:
            frec_char[1] = frec_char[1] + 1;
            if (frec_char[1] > 57){
              frec_char[1] = 48;
            }
            break;

          case 42:
            frec_char[2] = frec_char[2] + 1;
            if (frec_char[2] > 57){
              frec_char[2] = 48;
            }
            break;

          case 48:
            frec_char[3] = frec_char[3] + 1;
            if (frec_char[3] > 57){
              frec_char[3] = 48;
            }
            break;

          case 54:
            frec_char[4] = frec_char[4] + 1;
            if (frec_char[4] > 57){
              frec_char[4] = 48;
            }
            break;

          case 66:
            frec_char[5] = frec_char[5] + 1;
            if (frec_char[5] > 57){
              frec_char[5] = 48;
            }
            break;  

          case 72:
            frec_char[6] = frec_char[6] + 1;
            if (frec_char[6] > 57){
              frec_char[6] = 48;
            }
            break;  

          case 78:
            frec_char[7] = frec_char[7] + 1;
            if (frec_char[7] > 57){
              frec_char[7] = 48;
            }
            break;              

        
        }



      }else{

        switch (cursor_) {
          case 24:
            frec_char[0] = frec_char[0] - 1;
            if (frec_char[0] < 48){
              frec_char[0] = 57;
            }
            break;

          case 30:
            frec_char[1] = frec_char[1] - 1;
            if (frec_char[1] < 48){
              frec_char[1] = 57;
            }
            break;

          case 42:
            frec_char[2] = frec_char[2] - 1;
            if (frec_char[2] < 48){
              frec_char[2] = 57;
            }
            break;

          case 48:
            frec_char[3] = frec_char[3] - 1;
            if (frec_char[3] < 48){
              frec_char[3] = 57;
            }
            break;

          case 54:
            frec_char[4] = frec_char[4] - 1;
            if (frec_char[4] < 48){
              frec_char[4] = 57;
            }
            break;

          case 66:
            frec_char[5] = frec_char[5] - 1;
            if (frec_char[5] < 48){
              frec_char[5] = 57;
            }
            break;     

          case 72:
            frec_char[6] = frec_char[6] - 1;
            if (frec_char[6] < 48){
              frec_char[6] = 57;
            }
            break;   

          case 78:
            frec_char[7] = frec_char[7] - 1;
            if (frec_char[7] < 48){
              frec_char[7] = 57;
            }
            break;                   
        
        }


      }

      lcd.setCursor(0, linea);
      lcd.print("M");
      lcd.print(chan);

      lcd.setCursor(24, linea);
      lcd.print(frec_char[0]);
      lcd.print(frec_char[1]);
      lcd.print(".");
      lcd.print(frec_char[2]);
      lcd.print(frec_char[3]);
      lcd.print(frec_char[4]);
      lcd.print(".");
      lcd.print(frec_char[5]);
      lcd.print(frec_char[6]);
      lcd.print(frec_char[7]);

      oldPosition = newPosition;

    }





    if (digitalRead(ENCODER_BTN) == LOW) {

      delay(100);
      
      if (digitalRead(ENCODER_BTN) == LOW) {

        while (digitalRead(ENCODER_BTN) == LOW) {
          long_press+=1;
          delay(10);
        }

        if (long_press > 100){

          lcd.setCursor(0, linea);
          lcd.println(F("                 "));
          lcd.println(F("                 "));

          menu++;

        while (frec_char[0] == 48){

          for (byte i = 0; i<8; i++){
            frec_char[i] = frec_char[i+1];
          }
          
          frec_char[7] = 0;

        }

          //String frecuencia1(frec_char);

          unsigned long frecuencia1 = strtol(frec_char,NULL,0);


          Serial.println("************");
          Serial.println(frecuencia1);
          Serial.println(menu);
          Serial.println("------------");

          
          //return(frecuencia/100);
          return(strtol(frec_char,NULL,0));


        }else{

          long_press=0;

        }
        
        lcd.setCursor(cursor_, linea + 1);
        lcd.print(" ");

        cursor_ = cursor_ + 6;

        if(cursor_ == 36){
          cursor_ += 6;
        }

        if(cursor_ == 60){
          cursor_ += 6;
        }

        if (cursor_ > 78) {
          cursor_ = 24;
        }

        lcd.setCursor(cursor_, linea + 1);
        lcd.print("-");     

      }
    
    }
    
  }


  lcd.setCursor(0, linea);
  lcd.println(F("                 "));
  lcd.println(F("                 "));

  menu++;
  
  //return(frecuencia/100);


}

//********** fin edita


//********** edita2

unsigned long edita2(unsigned long frecuencia, byte linea, byte chan) {
  //void edita(unsigned long frecuencia, byte linea){

    
  boolean ciclo=true;
  unsigned long long_press = 0;
  char frec_char[8] = "*";

  oldPosition = newPosition = myEnc.read() / 4;

  //frecuencia*=100;
  ultoa(frecuencia,frec_char,10);


  while (frec_char[7] < 48 || frec_char[7] > 57){

    for (byte i=7; i>0; i--){
      frec_char[i] = frec_char[i-1];
    }

    frec_char[0] = 48;

  }

  
  lcd.setCursor(0, linea);
  lcd.print("M");
  lcd.print(chan);
  
  cursor_ = 24;
  lcd.setCursor(cursor_, linea);
  lcd.print(frec_char[0]);
  lcd.print(frec_char[1]);
  lcd.print(".");
  lcd.print(frec_char[2]);
  lcd.print(frec_char[3]);
  lcd.print(frec_char[4]);
  lcd.print(".");
  lcd.print(frec_char[5]);
  lcd.print(frec_char[6]);
  lcd.print(frec_char[7]);  
  lcd.setCursor(cursor_, linea + 1);
  lcd.print("-");
  lcd.setCursor(90, linea);
  lcd.print("<");

 
  while(ciclo){

  newPosition = myEnc.read() / 4;

    if (newPosition != oldPosition) {
      if (newPosition > oldPosition) {

        switch (cursor_) {
          case 24:
            frec_char[0] = frec_char[0] + 1;
            if (frec_char[0] > 57){
              frec_char[0] = 48;
            }
            break;

          case 30:
            frec_char[1] = frec_char[1] + 1;
            if (frec_char[1] > 57){
              frec_char[1] = 48;
            }
            break;

          case 42:
            frec_char[2] = frec_char[2] + 1;
            if (frec_char[2] > 57){
              frec_char[2] = 48;
            }
            break;

          case 48:
            frec_char[3] = frec_char[3] + 1;
            if (frec_char[3] > 57){
              frec_char[3] = 48;
            }
            break;

          case 54:
            frec_char[4] = frec_char[4] + 1;
            if (frec_char[4] > 57){
              frec_char[4] = 48;
            }
            break;

          case 66:
            frec_char[5] = frec_char[5] + 1;
            if (frec_char[5] > 57){
              frec_char[5] = 48;
            }
            break;  

          case 72:
            frec_char[6] = frec_char[6] + 1;
            if (frec_char[6] > 57){
              frec_char[6] = 48;
            }
            break;  

          case 78:
            frec_char[7] = frec_char[7] + 1;
            if (frec_char[7] > 57){
              frec_char[7] = 48;
            }
            break;              

        
        }



      }else{

        switch (cursor_) {
          case 24:
            frec_char[0] = frec_char[0] - 1;
            if (frec_char[0] < 48){
              frec_char[0] = 57;
            }
            break;

          case 30:
            frec_char[1] = frec_char[1] - 1;
            if (frec_char[1] < 48){
              frec_char[1] = 57;
            }
            break;

          case 42:
            frec_char[2] = frec_char[2] - 1;
            if (frec_char[2] < 48){
              frec_char[2] = 57;
            }
            break;

          case 48:
            frec_char[3] = frec_char[3] - 1;
            if (frec_char[3] < 48){
              frec_char[3] = 57;
            }
            break;

          case 54:
            frec_char[4] = frec_char[4] - 1;
            if (frec_char[4] < 48){
              frec_char[4] = 57;
            }
            break;

          case 66:
            frec_char[5] = frec_char[5] - 1;
            if (frec_char[5] < 48){
              frec_char[5] = 57;
            }
            break;     

          case 72:
            frec_char[6] = frec_char[6] - 1;
            if (frec_char[6] < 48){
              frec_char[6] = 57;
            }
            break;   

          case 78:
            frec_char[7] = frec_char[7] - 1;
            if (frec_char[7] < 48){
              frec_char[7] = 57;
            }
            break;                   
        
        }


      }

      lcd.setCursor(0, linea);
      lcd.print("M");
      lcd.print(chan);

      lcd.setCursor(24, linea);
      lcd.print(frec_char[0]);
      lcd.print(frec_char[1]);
      lcd.print(".");
      lcd.print(frec_char[2]);
      lcd.print(frec_char[3]);
      lcd.print(frec_char[4]);
      lcd.print(".");
      lcd.print(frec_char[5]);
      lcd.print(frec_char[6]);
      lcd.print(frec_char[7]);

      oldPosition = newPosition;

    }





    if (digitalRead(ENCODER_BTN) == LOW) {

      delay(100);
      
      if (digitalRead(ENCODER_BTN) == LOW) {

        while (digitalRead(ENCODER_BTN) == LOW) {
          long_press+=1;
          delay(10);
        }

        if (long_press > 100){

          lcd.setCursor(0, linea);
          lcd.println(F("                 "));
          lcd.println(F("                 "));

          menu++;

        while (frec_char[0] == 48){

          for (byte i = 0; i<8; i++){
            frec_char[i] = frec_char[i+1];
          }
          
          frec_char[7] = 0;

        }

          //String frecuencia1(frec_char);

          unsigned long frecuencia1 = strtol(frec_char,NULL,0);


          Serial.println("************");
          Serial.println(frecuencia1);
          Serial.println(menu);
          Serial.println("------------");

          
          //return(frecuencia/100);
          return(strtol(frec_char,NULL,0));


        }else{

          long_press=0;

        }
        
        lcd.setCursor(cursor_, linea + 1);
        lcd.print(" ");

        cursor_ = cursor_ + 6;

        if(cursor_ == 36){
          cursor_ += 6;
        }

        if(cursor_ == 60){
          cursor_ += 6;
        }

        if (cursor_ > 78) {
          cursor_ = 24;
        }

        lcd.setCursor(cursor_, linea + 1);
        lcd.print("-");     

      }
    
    }
    
  }


  lcd.setCursor(0, linea);
  lcd.println(F("                 "));
  lcd.println(F("                 "));

  menu++;
  


}

//********** fin edita2


//************* END FUNCIONES


