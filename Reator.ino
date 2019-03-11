/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e
 *  envia sinais de controle para alguns motores e uma resistência.
 *
 *  Autor: Julian Jose de Brito
 *
 *  Versão 1.4 01/11/2018: -Exclusão das linas de codigo relacionadas ao sensor ultrassônico
 *                         -Implementação da função filtro
 *  Versão 1.5 26/11/2018: -Mudanças para deixar o código mais legível
 *                         -Conserto em bugs na função filtro
 *  Versão 1.6 22/11/2018: Inclusão do suporte ao sensor de fluxo YF-S401
 */

//#######################################################################################################################
// CARREGAMENTO DE BIBLIOTECAS

//carrega bibliotecas para uso do sensor de temperatura
#include <OneWire.h> // ler a biblioteca
#include <DallasTemperature.h> // ler a biblioteca

//carrega a biblioteca para as interrupções
#include <TimerFour.h>

//#######################################################################################################################
// DEFINIÇÕES DOS PINOS

#define velocidade_bomba1 6 // Define pinos para o pwm da bomba 1.
#define resistencia 7 // Define o pino de saida para a resistência.
#define sensor_temperatura 3 // Sinal do DS18B20 (sensor de temperatura).
#define step_agitador 10 // Define os pinos de saida para o motor de passo (agitador).
#define sentido_agitador 11 // Pino que define o sentido do agitador
#define condutivimetro A3 // Sensor de condutividade
#define sensor_nivel A0 // Sensor de nível

unsigned const sensor_fluxo = 5;

//#######################################################################################################################
// CONFIGURAÇÕES INICIAIS

// Define uma instancia do oneWire para comunicacao com o sensor de temperatura
OneWire oneWire(sensor_temperatura);
DallasTemperature sensors(&oneWire);
DeviceAddress ENDERECO_SENSOR_TEMPERATURA;

//#######################################################################################################################
// PAINEL DE CONTROLE

boolean CH_LIGA_BOMBA1 = 1;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 0;
boolean CH_SENSOR_NIVEL = 0;
boolean CH_CONDUTIVIMETRO = 0;
boolean CH_HABILITA_SENSOR_FLUXO = 1;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA, NIVEL, CONDUTIVIDADE, FLUXO;
int VELOCIDADE_AGITADOR = 100, TEMPERATURA_RESISTENCIA = 50, PWM_BOMBA1 = 0;

// Variáveis de tempo
unsigned long time;
unsigned long TEMPO_ATUAL;

// Sensor de fluxo

unsigned long duration;

int INDEX = 0;

float NIVEL_APROXIMADO, LEITURAS_NIVEL[20];

void setup()
{
   // Inicializa a comunicação serial
   Serial.begin(9600);

   // Configura o sensor de temperatura
   sensors.begin();
   sensors.getAddress(ENDERECO_SENSOR_TEMPERATURA, 0);

   // Seta os pinos como saida ou entrada
   pinMode(velocidade_bomba1, OUTPUT);
   pinMode(resistencia, OUTPUT);
   pinMode(step_agitador, OUTPUT);
   pinMode(sentido_agitador, OUTPUT);
   pinMode(sensor_nivel, INPUT);
   pinMode(sensor_fluxo, INPUT);
}

void loop()
{
  //verifica se algo foi digitado no canal serial
  le_informacao();

  if(CH_LIGA_BOMBA1)
    liga_motor();

  if(CH_LIBERA_SENSOR_TEMPERATURA)
    le_sensor_temperatura();

  if(CH_HABILITA_AGITADOR)
    aciona_agitador();

  if(CH_SENSOR_NIVEL)
    le_nivel();

  if(CH_RESISTENCIA)
    aciona_resistencia();

  if(CH_HABILITA_SENSOR_FLUXO)
    le_fluxo();
  // le_condutividade(); Ainda precisa ser implementado
  exibe_dados();
}

void le_informacao()
{
  // Lê as informações digitadas na interface serial e atualiza as variáveis do sistema
  while (Serial.available() > 0)
  {
    // Lê primeiro byte digitado pelo usuário
      switch (Serial.read())
      {
        if(CH_CONDUTIVIMETRO)
        case 'B':
                    PWM_BOMBA1 = Serial.parseInt();
                    break;
        case 'T':
                    TEMPERATURA_RESISTENCIA = Serial.parseFloat();
                    break;
        case 'A':
                    VELOCIDADE_AGITADOR = Serial.parseInt(); //faixa recomendada 100 - 700
                    break;
        case 'L':
                    break;
        case 'I':
                    // Implementar função de impressão avançada de dados. !!!
                    break;
        case 'F':
                    break;
        default:
                   //Serial.println("Opcao invalida!");
                    break;
      }
   }
}

void le_fluxo()
{
        duration = pulseInLong(sensor_fluxo, HIGH);
        FLUXO = duration;
      //  FLUXO = (1/(duration*2))/98;
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
  tone(step_agitador, VELOCIDADE_AGITADOR);
}

void le_nivel()
{
  NIVEL = analogRead(A0);

  if(INDEX == 20)
    INDEX = 0;

  LEITURAS_NIVEL[INDEX] = NIVEL;
  NIVEL_APROXIMADO = filtro(20, NIVEL_APROXIMADO, LEITURAS_NIVEL);

  INDEX++;
}

void le_condutividade()
{
  CONDUTIVIDADE = analogRead(A3);
}

float filtro(int tamanho, float media_anterior, float *leituras)
{
  // filtra os valores do sensor de nível

  int i = 0, variacao = 2;
  float soma = 0, media = 0;

  for(i = 0; i < tamanho; i++)
      soma += leituras[i];

  media = soma/tamanho;

  //filtra o valor da média com base em uma variação de valor = 2
  if(!((media >= (media_anterior + (variacao/2))) ||  (media <= (media_anterior-(variacao/2)))))
      media = media_anterior;

  return media;
}

void exibe_dados()
{

  if(CH_HABILITA_AGITADOR)
  {
    Serial.print(" Velocidade do agitador: ");
    Serial.print(VELOCIDADE_AGITADOR);
  }

  if(CH_LIGA_BOMBA1)
  {
    Serial.print(" PWM Bomba 1: ");
    Serial.print(PWM_BOMBA1);
  }

  if(CH_RESISTENCIA)
  {
    Serial.print(" Temperatura Resistencia: ");
    Serial.print(TEMPERATURA_RESISTENCIA);
  }

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  {
    Serial.print(" Temp C: ");
    Serial.print(TEMPERATURA);
  }

  if(CH_SENSOR_NIVEL)
  {
    Serial.print(" Nivel apoximado: ");
    Serial.print(NIVEL);
  }

  if(CH_CONDUTIVIMETRO)
  {
    Serial.print(" Condutividade: ");
    Serial.print(CONDUTIVIDADE);
  }

  if(CH_HABILITA_SENSOR_FLUXO)
  {
    Serial.print(" Fluxo (L/min): ");
    Serial.print(FLUXO);
  }

  Serial.print("\n");
}
