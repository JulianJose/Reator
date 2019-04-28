/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e
 *  envia sinais de controle para alguns motores e uma resistência.
 *
 *  Autor: Julian Jose de Brito
 *
 *  Versão 1.93 28/04/2019: Implementado o controle da segunda bomba.
 *                          Controle da valvula de saída adicionado.
 *                          Melhorias na eficiência do código.
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

// Define pinos para o pwm das bombas.
#define velocidade_bomba1 5
#define velocidade_bomba2 6

// Define o pino para a valvula de saída.
#define valvula 8

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
boolean CH_LIGA_BOMBA2 = 1;
boolean CH_LIGA_VALVULA = 1;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 0;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA;
int VELOCIDADE_AGITADOR = 100, 
    VELOCIDADE_AGITADOR_ANTERIOR = 0,
    TEMPERATURA_RESISTENCIA = 50, 
    PWM_BOMBA1 = 0, PWM_BOMBA2 = 0, 
    PERIODO_INTERVALO_AGITADOR = 0, 
    PERIODO_DESLIGADO = 0,
    PWM_VALVULA = 100;

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

   pinMode(velocidade_bomba2, OUTPUT);
   digitalWrite(velocidade_bomba1, LOW);

   pinMode(valvula, OUTPUT);
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
                    if((Serial.available() > 0) && (Serial.read() == '1'))
                      PWM_BOMBA1 = Serial.parseInt();
                    else//if((Serial.available() > 0) && (Serial.read() == '2'))
                      PWM_BOMBA2 = Serial.parseInt();

                    if(CH_LIGA_BOMBA1) // Aplica as alterações 
                      liga_bomba1();

                    if(CH_LIGA_BOMBA2)
                      liga_bomba2();
        
                    break;
        case 'T':
                    TEMPERATURA_RESISTENCIA = Serial.parseFloat();
                    break;
        case 'A':
                    VELOCIDADE_AGITADOR = Serial.parseInt(); //faixa recomendada limite 156
                    if((Serial.available() > 0) && (Serial.read() == 'P'))
                        PERIODO_INTERVALO_AGITADOR = Serial.parseInt();
                    if((Serial.available() > 0) && (Serial.read() == 'D'))
                        PERIODO_DESLIGADO = Serial.parseInt();
                    if(VELOCIDADE_AGITADOR > 156)
                        VELOCIDADE_AGITADOR = 156;
                    break;
        case 'V':
                    PWM_VALVULA = Serial.parseInt();

                    if(CH_LIGA_VALVULA)
                      muda_valvula();

                    break;
        default:
                    Serial.println("Opção inválida!")
                    break;
      }
   }

}

void liga_bomba1()
{
  analogWrite(velocidade_bomba1, PWM_BOMBA1);
}

void liga_bomba2()
{
    analogWrite(velocidade_bomba2, PWM_BOMBA2);  
}

void muda_valvula()
{
    // Regular a valvula!
    analogWrite(valvula, PWM_VALVULA);  
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


  if(PERIODO_INTERVALO_AGITADOR != 0)
  {
    if(((millis() - PERIODO_AGITADOR)/1000 >= PERIODO_INTERVALO_AGITADOR) && (PERIODO_DESLIGADO == 0))
      {
           PERIODO = PERIODO? false: true;
           PERIODO_AGITADOR = millis();    
      }  
    if(PERIODO_DESLIGADO != 0)
    {
      if((millis() - PERIODO_AGITADOR)/1000 >= PERIODO_DESLIGADO)
      {
           PERIODO = PERIODO? false: true;
           PERIODO_AGITADOR = millis();    
      }  
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

  if(CH_LIGA_BOMBA2)
  {
    Serial.print(" PWM Bomba 2: ");
    Serial.print(PWM_BOMBA2);
  }

  if(CH_LIGA_VALVULA)
  {
    Serial.print(" PWM Valvula: ");
    Serial.print(PWM_VALVULA);
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
