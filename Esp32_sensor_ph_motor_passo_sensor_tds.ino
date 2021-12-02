#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//    Configurações MQTT
#define TOPICO_PUBLISH_PH  "sensor_ph"
#define TOPICO_PUBLISH_TDS "sensor_tds"

#define ID_MQTT  "projeto_qualidade_agua_SE" 

const char* SSID     = "cleconde@";
const char* PASSWORD = "67142812";

const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883;

WiFiClient espClient;
PubSubClient MQTT(espClient);

void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);

//    Connfigurar LEDs
int pino_verde = 12;
int pino_vermelho = 27;
int pino_amarelo = 14;

//    Configurações do Sensor Ultrassônico
const int pino_trigger = 15;
const int pino_echo = 2;
#define SOUND_SPEED 0.034
long duration;
float distanciaCm;
int distanciaminima = 10;

//    Configurações do Motor de Passo
#include <Stepper.h> 
const int stepsPerRevolution = 500; 
Stepper myStepper1(stepsPerRevolution, 19,18,5,17);  //motor direito
//Stepper myStepper2(stepsPerRevolution, 3,1,22,23);//motor esquerdo

//    Configurações do Sensor de PH
float calibragem = 20.25;  //Valor de calibragem
const int analogInPin = 32;

//    Configurações do Sensor TDS
#define TdsSensorPin 26
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
float averageVoltage = 0,tdsValue = 0,temperature = 25;

//MQTT==================================================================

void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("Chegou a seguinte string via MQTT: ");
  Serial.println(msg);
}

void reconnectMQTT(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }  
}

void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void reconnectWiFi(void)
{
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}

//======================================================================

void setup()
{
  Serial.begin(9600);
  delay(1000);
  Serial.println("Disciplina Sistemas Embarcados: Sistema de Qualidade da Água");
  delay(1000);
  
  myStepper1.setSpeed(30);
  //myStepper2.setSpeed(60);
  pinMode(TdsSensorPin,INPUT); 
  pinMode(pino_verde, OUTPUT);
  pinMode(pino_vermelho, OUTPUT);
  pinMode(pino_amarelo, OUTPUT);
  
  pinMode(pino_trigger, OUTPUT);
  pinMode(pino_echo, INPUT);

  initWiFi();
  initMQTT();
}
  
void loop()
{ 
  VerificaConexoesWiFIEMQTT();
  
  //Le as informações em cm do sensor ultrassônico
  digitalWrite(pino_trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pino_trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pino_trigger, LOW);
  duration = pulseIn(pino_echo, HIGH);
  distanciaCm = duration * SOUND_SPEED/2;
  

  //Calculo do TDS
  float valor = (analogRead(TdsSensorPin)* (float)VREF/ 1024.0)/(1.0+0.02*(temperature-25.0));
  float valorc3 = valor*valor*valor;
  float valorq2 = valor*valor;
  float valorfinal = ((valorc3 * 133.42)-(255.86*valorq2)+(857.39*valor))*0.5;

  //Calculo do PH
  float valorph = analogRead(analogInPin);
  float pHVol=(float)valorph*5.0/1024;
  float phValue = -5.70 * pHVol + calibragem;
    
  Serial.print("TDS = ");
  Serial.println(valorfinal);
  Serial.print("PH = ");
  Serial.println(phValue);
  Serial.print("Distancia em cm: ");
  Serial.println(distanciaCm);
  /*
  função de envios para a nuvem a ser desenvolvida.
  */
  if(distanciaCm < distanciaminima){
    for (int k = 0; k<=2; k++){
      myStepper1.step(682); 
      digitalWrite(pino_verde, HIGH);
    }
   }
   else{
      //myStepper1.step(682);
      //myStepper2.step(682);
      digitalWrite(pino_verde, LOW);
   }
   

   if(valorfinal >1000){
    MQTT.publish(TOPICO_PUBLISH_TDS,"TDS Alto");
    digitalWrite(pino_vermelho, HIGH);
   }
   else{
    MQTT.publish(TOPICO_PUBLISH_TDS,"TDS Baixo");
    digitalWrite(pino_vermelho, LOW);
   }
   
   if(phValue > 9.5 || phValue < 6.0){
    MQTT.publish(TOPICO_PUBLISH_PH,"PH Alto");
    digitalWrite(pino_amarelo, HIGH);
   }else{
    MQTT.publish(TOPICO_PUBLISH_PH,"PH Baixo");
    digitalWrite(pino_amarelo, LOW);
   }

   MQTT.loop();
   delay(2000);
}
