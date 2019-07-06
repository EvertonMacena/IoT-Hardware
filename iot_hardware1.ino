
String ssid     = "Simulator Wifi";
String password = "";

//parametros de rota
String apartment = "2";
String sensor = "142";

// informacoes de conexao
String host     	= "iot-pavi.herokuapp.com";
const int httpPort	= 80;			
String url			= "";
String url1			= "/api/apartment/";
String url2			= "/sensor/";
String url3			= "/show";

// pinos usados
int pinLed = 13;
int pinBuzzer = 7;
int pinSensorPIR = 8;
int gasSensorA = A4;

// variaveis de leitura
int sensorPIRValue = 0;
int sensorGasValue = 0;

boolean flagAlarm = true;

void wifiSetup()
{
  //definir url
  url = url1+apartment+url2+sensor+url3;
  
  //Inicio da comunicacao com o ESP-01
  Serial.begin(115200);		
  Serial.println("AT");
  delay(10);
    
  //Conexão com a rede virtual wi-fi com o módulo ESP
  Serial.print("AT+CWJAP=\"");
  Serial.print(ssid);
  Serial.print("\",\"");
  Serial.print(password);
  Serial.println("\"");
  delay(10);
	
  //Abertura de conexao TCP
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(50);
  //se o módulo ESP-01 não iniciar corretamente, ligue o LED
  if (!Serial.find("OK")) digitalWrite(pinLed, HIGH);
}

void readFlagServe()
{
  //Request HTTP
  String httpPacket = "GET " + url + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
  int length = httpPacket.length();
  
  // Envia o comprimento do pacote
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10);	//Aguardando o módulo ESP responder
  
  if (!Serial.find(">")) digitalWrite(pinLed, HIGH); // Verificamos se o ESP está ok, caso contrário acendemos o LED no pino 11
  //Enviando a request http
  Serial.print(httpPacket);
  delay(10); //Aguardando o módulo ESP responder
  
  if (!Serial.find("SEND OK\r\n")) digitalWrite(pinLed, HIGH); //Verifica se o ESP está ok, caso contrário liga o LED
	
  while(!Serial.available()) delay(5);	//Aguarda receber uma resposta do servidor

  if (Serial.find("\r\n\r\n")){	//Se receber uma resposta do servidor, pular os cabeçalhos HTTP
    delay(5);
    
    unsigned int i = 0; //Contador para um tempo limite (se não existir uma resposta após um minuto, desconectamos)
    String outputString = "";
    
    while (!Serial.find("\"is_on\":")){} //Procurando a chave "is_on" no JSON
    while (i<60000) { // timeout de 1 minuto
      if(Serial.available()) {
        char c = Serial.read(); // Ler a resposta do servidor
        if(c==',') break; 		// Até chegar ao final do elemento do ícone JSON
        outputString += c; 		// Concatenar em outputString
        i=0; 					// Zerar o timeout
      }
      i++;
    }
    
    if(outputString ==  "1"){
      flagAlarm = true;
      Serial.print("O alarme esta ativado");
    } else {
      flagAlarm = false;
      Serial.print("O alarme esta desativado");
    }
 }
  
  delay(10000);	// Esperar 10 segundos antes do próximo pedido (A demora estar aqui)
}

void gasSetup()
{
  pinMode (gasSensorA, INPUT);
}


void lightSetup()
{
  pinMode(A5, INPUT);
  pinMode(pinLed, OUTPUT);
}

void motionSetup()
{
  //Definido pinos como de entrada ou de saída
  pinMode(pinBuzzer,OUTPUT);
  pinMode(pinSensorPIR,INPUT);
  pinMode(pinLed,OUTPUT);
}
void onAlarm()
{
  //Ligando o buzzer com uma frequencia de 1500 hz.
  tone(pinBuzzer,1500);
  
  for(int i = 0; i<3; i++){
    //Ligando o led
    digitalWrite(pinLed, HIGH);
    delay(100);
    //Desligando o led
  	digitalWrite(pinLed, LOW);
    delay(100);
  }
   
  delay(4000); //tempo que o led fica acesso e o buzzer toca
   
  offAlarm();
}
 
void offAlarm() {
  //Desligando o led
  digitalWrite(pinLed,LOW );
   
  //Desligando o buzzer
  noTone(pinBuzzer);
}

void readGas()
{
  sensorGasValue = analogRead(gasSensorA);
  Serial.print("Valor do Sensor de Gas: ");  
  Serial.println(sensorGasValue);
  
  if (sensorGasValue > 700 && flagAlarm){
    onAlarm();
  } else {
    offAlarm();
  } 
}

void readMotion()
{
  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  sensorPIRValue = digitalRead(pinSensorPIR);
   
  Serial.print("Valor do Sensor PIR: ");  
  Serial.println(sensorPIRValue);
   
  //Verificando se ocorreu detecção de movimentos
  if (sensorPIRValue == 1 && flagAlarm) {
    onAlarm();
  } else {
    offAlarm();
  }    
}

void readLigth()
{
  if (analogRead(A5) < 150) {
    digitalWrite(pinLed, HIGH);
  } else {
    digitalWrite(pinLed, LOW);
  }
  delay(10);
}
  

void setup()
{
  wifiSetup();
  lightSetup();
  motionSetup();
  gasSetup();
}

void loop()
{
  readFlagServe();
  readLigth();
  readMotion();
  readGas();
}
