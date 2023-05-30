#include <SPI.h>
#include <LoRa.h>

int num_gp = 9;

int taille_paquet_recu;

// Paramètres LORA
int preambleLength = 8;
int frequency = 862e6 + num_gp*1e6;     // frequency : depends on the trinom number in order to avoid interferences //871e6
int SF = 7;                            // Spreading factor [ 7 - 12 ]
int BW = 125E3;                         // Other values:  250E3, 125E3, 62.5E3, 41.7E3, 20.8E3, 15.6E3, 10.4E3, 7.8E3
int CR = 5;                             // [5 à 8]


void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Je suis le récepteur");

  if (!LoRa.begin(frequency)) {
    Serial.println("Problème au démarrage du module LoRa!");
  }

  // setting of the Lora parameters (power, preamble length, Spreading Factor, Bandwidth, CodingRate, etc.)
  LoRa.setPreambleLength(preambleLength) ;
  LoRa.setSpreadingFactor(SF);  // Choix de : 7 à 12
  LoRa.setSignalBandwidth(BW);
  LoRa.setCodingRate4(CR);

}

void loop() {

  taille_paquet_recu = LoRa.parsePacket();
  // Vérifier si un paquet est bien reçu
  if (taille_paquet_recu==49) {
// sauf dos

    // Lire et afficher le contenu du paquet reçu octet par octet
    while (LoRa.available()) {
    char ma_trame[49]="";
  LoRa.readBytes( (byte*)& ma_trame, sizeof(ma_trame)) ;
      uint8_t premier_octet=(uint8_t)ma_trame[0];

      if (premier_octet<100){
    Serial.print("GAUCHE ");
    for(int six=5; six>=0; six--){
      uint8_t power2 = (1<<six);
      if(premier_octet>=power2){
          Serial.print("1");
          premier_octet-=power2;
      }
      else{
          Serial.print("0");
          //premier_octet-=power2;
      }
    }
    }
      if (premier_octet>100){
    Serial.print("DROITE ");
    premier_octet-=192;
    for(int six=5; six>=0; six--){
      uint8_t power2 = (1<<six);
      if(premier_octet>=power2){
          Serial.print("1");
          premier_octet-=power2;
      }
      else{
          Serial.print("0");
          //premier_octet-=power2;
      }
    }
   }
      Serial.print(" ");
        for (int indice=1; indice<sizeof(ma_trame);indice++){
    
        Serial.print((int8_t)ma_trame[indice]);
        Serial.print("_");
        }
        //Serial.println(ma_trame);
    }
    
  Serial.println();
  }


    if (taille_paquet_recu==62) {
// dos
  
    Serial.print("DOS ");

    // Lire et afficher le contenu du paquet reçu octet par octet
    while (LoRa.available()) {
    char ma_trame[62]="";
  LoRa.readBytes( (byte*)& ma_trame, sizeof(ma_trame)) ;
        for (int indice=0; indice<sizeof(ma_trame);indice++){
          
        Serial.print((int8_t)ma_trame[indice]); // A CHANGER ET A ADAPTER A code de Herold de DOS
        Serial.print(" ");
        }
        //Serial.println(ma_trame);
    }
    
  Serial.println();

 }
    // Aficher la puissance du signal reçu
    //Serial.print("' avec une puissance du signal (RSSI) égale à: ");
    //Serial.println(LoRa.packetRssi());
    
  
}

