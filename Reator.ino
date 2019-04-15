/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e
 *  envia sinais de controle para alguns motores e uma resistência.
 *
 *  Autor: Julian Jose de Brito
 *
 *  Versão 1.92 15/04/2019: Melhorias no controle do agitador para evitar despercício de energia.
 *                          Variação suave da velocidade do agitador.
 *                          Controle da faixa de intervalo de ativação do agitador.
 * 
 */

//#######################################################################################################################
// CARREGAMENTO DE BIBLIOTECAS

//carrega bibliotecas para uso do sensor de temperatura
#include <OneWire.h> // ler a biblioteca
#include <DallasTemperature.h> // ler a biblioteca
#include <TimerFour.h>

//carrega a biblioteca para configurar as interrupções
//#include <TimerThree.h>

//#######################################################################################################################
// DEFINIÇÕES DOS PINOS

// Define pinos para o pwm da bomba 1.
#define velocidade_bomba1 6

// Define o pino de saida para a resistência.
#define resistencia 7

// Sinal do DS18B20 (sensor de temperatura).
#define sensor_temperatura 3

// Define os pinos de saida para o motor de passo (agitador).
#define step_agitador 10
#define sentido_agitador 11
#define sleep 12

//#######################################################################################################################
// CONFIGURAÇÕES INICIAIS

// Define uma instancia do oneWire para comunicacao com o sensor de temperatura
OneWire oneWire(sensor_temperatura);
DallasTemperature sensors(&oneWire);
DeviceAddress ENDERECO_SENSOR_TEMPERATURA;

//#######################################################################################################################
// PAINEL DE CONTROLE

//Define as chaves de controle
boolean CH_LIGA_BOMBA1 = 1;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 1;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA;
int VELOCIDADE_AGITADOR = 0, VELOCIDADE_AGITADOR_ANTERIOR = 0,
    TEMPERATURA_RESISTENCIA = 50, 
    PWM_BOMBA1 = 0, PERIODO_INTERVALO_AGITADOR = 0;

unsigned long PERIODO_AGITADOR; // Variável que vai guardar o tempo dos intervalor de ativação do agitador
boolean PERIODO = false; // Boolean dependente do perído definido.

void setup()
{

   // Inicializa a comunicação serial
   Serial.begin(9600);

   // Configura o sensor de temperatura
   sensors.begin();
   sensors.getAddress(ENDERECO_SENSOR_TEMPERATURA, 0);


   pinMode(velocidade_bomba1, OUTPUT);
   digitalWrite(velocidade_bomba1, LOW);

   pinMode(resistencia, OUTPUT);
   digitalWrite(velocidade_bomba1, LOW);

   pinMode(step_agitador, OUTPUT);
   digitalWrite(step_agitador, HIGH);

   pinMode(sentido_agitador, OUTPUT);
   digitalWrite(sentido_agitador, LOW);

   pinMode(sleep, OUTPUT);
   digitalWrite(sleep, LOW);

}

void loop()
{

  //verifica se algo foi digitado no canal serial
  le_informacao();

  if(CH_LIGA_BOMBA1)
  liga_motor();

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  le_sensor_temperatura();

  if(CH_RESISTENCIA)
  aciona_resistencia();

  if(CH_HABILITA_AGITADOR)
  aciona_agitador();

  exibe_dados();
}

void le_informacao()
{
  while (Serial.available() > 0)
  {
    // Lê primeiro byte digitado pelo usuário e atua no sistema
      switch (Serial.read())
      {
        case 'B':
                    PWM_BOMBA1 = Serial.parseInt();
                    break;
        case 'T':
                    TEMPERATURA_RESISTENCIA = Serial.parseFloat();
                    break;
        case 'A':
                    VELOCIDADE_AGITADOR = Serial.parseInt(); //faixa recomendada limite 156
                    if((Serial.available() > 0) && (Serial.read() == 'P'))
                        PERIODO_INTERVALO_AGITADOR = Serial.parseInt();
                    if(VELOCIDADE_AGITADOR > 156)
                        VELOCIDADE_AGITADOR = 156;
                    break;
        default:
                    break;
      }
   }

}

void liga_motor()
{
  analogWrite(velocidade_bomba1, PWM_BOMBA1);
}

void le_sensor_temperatura()
{
  sensors.requestTemperatures();
  TEMPERATURA = sensors.getTempC(ENDERECO_SENSOR_TEMPERATURA);
}

void aciona_resistencia()
{
  // Verifica se a temperatura desejada é maior ou menor do que a temperatura de leitura do sensor

  if(TEMPERATURA > TEMPERATURA_RESISTENCIA)
  digitalWrite(resistencia, LOW);

  if(TEMPERATURA < TEMPERATURA_RESISTENCIA)
  digitalWrite(resistencia, HIGH);
}

void aciona_agitador()
{  
  // Implementa uma variação suave de velocidade
  if((VELOCIDADE_AGITADOR - abs(VELOCIDADE_AGITADOR_ANTERIOR)) <= 15)
  {
      VELOCIDADE_AGITADOR_ANTERIOR = VELOCIDADE_AGITADOR;
  }
  else if((VELOCIDADE_AGITADOR - VELOCIDADE_AGITADOR_ANTERIOR) > 15)
      VELOCIDADE_AGITADOR_ANTERIOR+= 15;
      
  else if((VELOCIDADE_AGITADOR - VELOCIDADE_AGITADOR_ANTERIOR) < 15)
      VELOCIDADE_AGITADOR_ANTERIOR-= 15;

      
  Serial.println(VELOCIDADE_AGITADOR_ANTERIOR);

  if(PERIODO_INTERVALO_AGITADOR != 0)
  {
    if((millis() - PERIODO_AGITADOR)/1000 >= PERIODO_INTERVALO_AGITADOR)
      {
           PERIODO = PERIODO? false: true;
           PERIODO_AGITADOR = millis();    
      }      
  }
  else
     PERIODO = true;

  if(not(PERIODO))  // Permite usar o acionamento suave no motor de passo usando os períodos de intervalo.
     VELOCIDADE_AGITADOR_ANTERIOR = 0;
  
  if((VELOCIDADE_AGITADOR == 0) || (PERIODO == 0 && PERIODO_INTERVALO_AGITADOR != 0))
  {
      noTone(step_agitador);
      digitalWrite(sleep, LOW);
  }
  else if((VELOCIDADE_AGITADOR != 0) && PERIODO)
  {
      digitalWrite(sleep, HIGH);
      tone(step_agitador, VELOCIDADE_AGITADOR_ANTERIOR*200/60); // Manda uma onda quadrada para o driver do motor com a velocidade em rpm desejada.
  }
}

void exibe_dados()
{

  if(CH_HABILITA_AGITADOR)
  {
    Serial.print(" Velocidade do agitador (rpm): ");
    Serial.print(VELOCIDADE_AGITADOR_ANTERIOR);

    Serial.print(" intervalo do agitador (s): ");
    Serial.print(PERIODO_INTERVALO_AGITADOR);

  }

  if(CH_LIGA_BOMBA1)
  {
    Serial.print(" PWM Bomba 1: ");
    Serial.print(PWM_BOMBA1);
  }

  if(CH_RESISTENCIA)
  {
    Serial.print(" Temperatura Resistência: ");
    Serial.print(TEMPERATURA_RESISTENCIA);
  }

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  {
    Serial.print(" Temp ºC: ");
    Serial.print(TEMPERATURA);
  }
  Serial.println(" ");
}
