#include <SPI.h>
#include <LoRa.h>


//--------------------
#include "Wire.h"
// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
using namespace std;
MPU9250 accelgyro;
I2Cdev   I2C_M;

uint8_t buffer_m[7];
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

int8_t Axyz[3];
int8_t Gxyz[3];
int8_t component_z;
int8_t anticlockwise_z;

//-------------
int analogPin = A1;
int val=0;





int num_gp = 9;
int compteur = 0;
int nb_octets_a_envoyer = 49; //10,15,20, 25 ;
unsigned long Temps_debut_tx ;
float Temps_total_tx ;

// Paramètres LORA
int preambleLength = 6;
int frequency = 862e6 + num_gp*1e6;     // frequency : depends on the trinom number in order to avoid interferences //871e6
int SF = 7;                            // Spreading factor [ 7 - 12 ]
int BW = 125E3;                         // Other values:  250E3, 125E3, 62.5E3, 41.7E3, 20.8E3, 15.6E3, 10.4E3, 7.8E3
int CR = 5;                             // [5 à 8] do not change

int power = 10; // others : 5,10, 14, 17 max

void setup() {

  Wire.begin();
  Serial.begin(38400);
  accelgyro.initialize();
  accelgyro.testConnection();


  pinMode(A1,INPUT);


  delay(1000); //originally 5000
  Serial.println("Je suis l'émetteur");

  if (!LoRa.begin(frequency)) {
    Serial.println("Problème au démarrage du module LoRa!");
  }

}

void loop() {


char message_a_envoyer [49]= "" ;
uint8_t premier_octet=3;


for(int fois=0; fois<6;fois++){
//GAUCHE 00, >73; Droite 11, >75
  val=analogRead(analogPin);
  Serial.println(val);

  premier_octet=premier_octet << 1;

  if (val>310 ){
    premier_octet+=1;
  }



  getAccel_Data();
	getGyro_Data();
	getCompass_Data();  
  message_a_envoyer[8*fois+1]=(char)Axyz[0];  
  message_a_envoyer[8*fois+2]=(char)Axyz[1];  
  message_a_envoyer[8*fois+3]=(char)Axyz[2];  
  message_a_envoyer[8*fois+4]=(char)Gxyz[0];  
  message_a_envoyer[8*fois+5]=(char)Gxyz[1];  
  message_a_envoyer[8*fois+6]=(char)Gxyz[2];  
  message_a_envoyer[8*fois+7]=(char)component_z;  
  message_a_envoyer[8*fois+8]=(char)anticlockwise_z;
	//Serial.println((char)byteArr[2]); 

}


    message_a_envoyer[0]=premier_octet;



  Serial.print("Transmission du paquet: " + String(compteur));

  // setting ofthe Lora parameters (power, preamble length, Spreading Factor, Bandwidth, CodingRate, etc.)
  LoRa.setTxPower(power); 
  LoRa.setPreambleLength(preambleLength) ;
  LoRa.setSpreadingFactor(SF);  // Choix de : 7 à 12
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);  
  LoRa.beginPacket();
  for (int i=0; i <nb_octets_a_envoyer ; i++)
      LoRa.print(message_a_envoyer[i]);
      
  Temps_debut_tx = micros();
  bool succes = LoRa.endPacket();
  Temps_total_tx = (micros()-Temps_debut_tx) / 1000.0 ; 
  
  if (succes)
  {    
    Serial.println("... Succes ... Temps  de transmission: " + String(Temps_total_tx) + " milliseconds" );   
  }
  else
  {
    Serial.println("... Echec ... Apres un temps de :  " + String(Temps_total_tx) + "milliseconds" );   
  }

  compteur++;
}



void getAccel_Data(void){
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Axyz[0] = (int8_t) ((double)ax / 256);//16384  LSB/g
  Axyz[1] = (int8_t) ((double)ay / 256);
  Axyz[2] = (int8_t) ((double)az / 256); 
}

void getGyro_Data(void){
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Gxyz[0] = (int8_t) ((double)gx / 256); //131 LSB(��/s)
  Gxyz[1] = (int8_t) ((double)gy / 256); 
  Gxyz[2] = (int8_t) ((double)gz / 256); 
}

void getCompass_Data(void){
	I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
	delay(10);
	I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);
  mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0];
	my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2];
	mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4];	
  mx-=31; //calibration
  my-=27;
  mz+=58;
  anticlockwise_z = (int8_t) (atan2(my,mx)*40);
	component_z = (int8_t) (mz/sqrt(mx*mx+my*my+mz*mz)*127);
}
