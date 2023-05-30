#define SAMPLES 1024
#define SAMPLING_FREQUENCY 20000
#define OFFSET_VOLTAGE 2.5

const int signalPin = A0;
const unsigned long samplingPeriod = 1000; // Durée de l'échantillonnage en ms
const unsigned long numOfSamples = 1000; // Nombre d'échantillons à prendre pendant la période d'échantillonnage

unsigned long startTime;
unsigned long sampleCount;
int maxValue;
int minValue;
double dB;

void setup() {
  Serial.begin(9600);
  
  pinMode(signalPin, INPUT);
  startTime = millis();
  maxValue = 0;
  minValue = 1023;
  sampleCount = 0;
}

void loop() {
  int currentValue = analogRead(signalPin);
  maxValue = max(maxValue, currentValue);
  minValue = min(minValue, currentValue);

  sampleCount++;

  // Si la période d'échantillonnage est terminée
  if (millis() - startTime >= samplingPeriod) {
    // Convertir la valeur maximale en tension (0-5V)
    float maxVoltage = (float)maxValue * 5.0 / 1023.0;
    // Convertir la valeur minimale en tension (0-5V)
    float minVoltage = (float)minValue * 5.0 / 1023.0;
    // Calculer la valeur pic à pic
    float peakToPeakVoltage = maxVoltage - minVoltage;


    Serial.println("Correspondant à une intensité sonore théorique de ");
    dB = log((peakToPeakVoltage / 3.55) * 100000) * 1 / 0.0996*1/1.263157;
    Serial.println(dB);
    Serial.println("dB");

    // Réinitialiser les variables pour la prochaine période d'échantillonnage
    maxValue = 0;
    minValue = 1023;
    sampleCount = 0;
    startTime = millis();
  }
}
