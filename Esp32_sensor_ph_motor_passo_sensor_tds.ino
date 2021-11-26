//Configuração wifi e MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#define TOPICO_MONITORAMENTO   "topico_monitoramento"
#define ID_MQTT  "esp32_mqtt"     //id mqtt (para identificação de sessão)
const char* SSID = " "; // SSID / nome da rede WI-FI 
const char* PASSWORD = " "; // Senha da rede WI-FI 
const char* BROKER_MQTT = "ec2-52-14-128-203.us-east-2.compute.amazonaws.com"; //URL do broker MQTT na AWS
int BROKER_PORT = 1234; // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

//    Connfigurar LEDs
int pino_verde = 7;
int pino_vermelho = 5;
int pino_amarelo = 6;

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
  pinMode(pino_verde, OUTPUT);
  pinMode(pino_vermelho, OUTPUT);
  pinMode(pino_amarelo, OUTPUT);
}


/* Função: inicializa e conecta-se na rede WI-FI desejada
*  Parâmetros: nenhum
*  Retorno: nenhum
*/
void initWiFi(void) 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
      
    reconnectWiFi();
}
 
/* Função: inicializa parâmetros de conexão MQTT(endereço do 
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void initMQTT(void) 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
      
     reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
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
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
void initMQTT(void);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);
 

  
void loop()
{ 
  //Le as informações em cm do sensor ultrassônico
  float cmMsec;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

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
  Serial.println(cmMsec);
  /*
  função de envios para a nuvem a ser desenvolvida.
  */
  if(cmMsec < distanciaminima){
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
    digitalWrite(pino_vermelho, HIGH);
   }
   else{
    digitalWrite(pino_vermelho, LOW);
   }
   
   if(phValue > 9.5 || phValue < 6.0){
    digitalWrite(pino_amarelo, HIGH);
   }else{
    digitalWrite(pino_amarelo, LOW);
   }
  
   payload = valorfinal //em desenvolvimento, o objeto json precisa ser criado
   MQTT.publish(TOPICO_MONITORAMENTO, payload);

   delay(2000);
}
