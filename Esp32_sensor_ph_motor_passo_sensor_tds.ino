//    Configurações do Sensor Ultrassônico
#include <Ultrasonic.h>
#define pino_trigger 10
#define pino_echo 11
Ultrasonic ultrasonic(pino_trigger, pino_echo);

//    Configurações do Motor de Passo
#include <Stepper.h> 
const int stepsPerRevolution = 500; 
Stepper myStepper(stepsPerRevolution, 6,7,8,15); 

//    Configurações do Sensor de PH
float calibragem = 20.25;  //cValor de calibragem
const int analogInPin = 36;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10],temp;

//    Configurações do Sensor TDS
#define TdsSensorPin 39
#define VREF 5.0 // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;


void setup()
{
  Serial.begin(9600);
  myStepper.setSpeed(100);
  pinMode(TdsSensorPin,INPUT); 
}

  
void loop()
{ 
  //Le as informações em cm do sensor ultrassônico
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);


  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  
  static unsigned long printTimepoint = millis();
  
  if(millis()-printTimepoint > 800U)
  {
    printTimepoint = millis();
    for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
    analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF/ 1024.0; 
    float compensationCoefficient=1.0+0.02*(temperature-25.0);
    float compensationVolatge=averageVoltage/compensationCoefficient;
    tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5;
    
    Serial.print("TDS:");
    Serial.print(tdsValue,0);
    Serial.println("ppm");

    //Calculos do sensor de PH
    for(int i=0;i<10;i++){
      buf[i]=analogRead(analogInPin);
      delay(30);
    }
    for(int i=0;i<9;i++){
      for(int j=i+1;j<10;j++)
      {
        if(buf[i]>buf[j])
        {
          temp=buf[i];
          buf[i]=buf[j];
          buf[j]=temp;
        }
      }
    }
    avgValue=0;
    
    for(int i=2;i<8;i++)
      avgValue+=buf[i];
      
    float pHVol=(float)avgValue*5.0/1024/6;
    float phValue = -5.70 * pHVol + calibragem;
    

    Serial.print("PH = ");
    Serial.println(phValue);
    Serial.print("Distancia em cm: ");
    Serial.println(cmMsec);

    if(cmMsec < 10){
      for (int k = 0; k<=2; k++){
        myStepper.step(682); 
      }
      //myStepper.step(2046);
    }
    else{
      myStepper.step(2046);
    }

    delay(2000);
  }
}

//    Configurações do Sensor TDS
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++){
    for (i = 0; i < iFilterLen - j - 1; i++){
      if (bTab[i] > bTab[i + 1]){
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}
