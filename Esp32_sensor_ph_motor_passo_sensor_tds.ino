//    Configurações do Sensor Ultrassônico
#include <Ultrasonic.h>
#define pino_trigger 3
#define pino_echo 2
Ultrasonic ultrasonic(pino_trigger, pino_echo);
int distanciaminima = 10;

//    Configurações do Motor de Passo
#include <Stepper.h> 
const int stepsPerRevolution = 500; 
Stepper myStepper1(stepsPerRevolution, 8,10,9,11);  //motor direito
//Stepper myStepper2(stepsPerRevolution, 3,1,22,23);//motor esquerdo

//    Configurações do Sensor de PH
float calibragem = 20.25;  //Valor de calibragem
const int analogInPin = A0;

//    Configurações do Sensor TDS
#define TdsSensorPin A1
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
float averageVoltage = 0,tdsValue = 0,temperature = 25;


void setup()
{
  Serial.begin(9600);
  myStepper1.setSpeed(60);
  //myStepper2.setSpeed(60);
  pinMode(TdsSensorPin,INPUT); 
}


void loop()
{ 
  //Le as informações em cm do sensor ultrassônico
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  //Calculo do TDS
  float valor = (analogRead(TdsSensorPin)* (float)VREF/ 1024.0)/(1.0+0.02(temperature-25.0));
  float valorc3 = valorvalorvalor;
  float valorq2 = valorvalor;
  float valorfinal = ((valorc3 * 133.42)-(255.86valorq2)+(857.39valor))0.5;

  //Calculo do PH
  float valorph = analogRead(analogInPin);
  float pHVol=(float)valorph5.0/1024;
  float phValue = -5.70 * pHVol + calibragem;

  Serial.print("TDS = ");
  Serial.println(valorfinal);
  Serial.print("PH = ");
  Serial.println(phValue);
  Serial.print("Distancia em cm: ");
  Serial.println(cmMsec);
  /*
  função de envios para a nuvem a ser desenvolvida.
  */
  if(cmMsec < distanciaminima){
    for (int k = 0; k<=2; k++){
      myStepper1.step(682); 
    }
   }
   else{
      //myStepper1.step(682);
      //myStepper2.step(682);
   }

   delay(2000);
}
