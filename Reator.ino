/*  Reator: Programa feito para o controle de um reator do tipo
 *  batelada. O programa faz a supervisão de alguns sensores e
 *  envia sinais de controle para alguns motores e uma resistência.
 *
 *  Autor: Julian Jose de Brito
 *
 *  Versão 2.2 04/07/2019: Ajuste da precisão de leitura do volume
 * 
 */

//#######################################################################################################################
// CARREGAMENTO DE BIBLIOTECAS

//carrega bibliotecas para uso do sensor de temperatura
#include <OneWire.h> // ler a biblioteca
#include <DallasTemperature.h> // ler a biblioteca
#include <TimerFour.h>
#include <HX711.h>
#include <Servo.h>


//#######################################################################################################################
// DEFINIÇÕES DOS PINOS

#define PWM_BOMB_1 5
#define PWM_BOMB_2 6
#define VALVULA 8
#define RESISTENCIA 7
#define SENSOR_TEMPERATURA 3
#define STEP_AGITADOR 10
#define SENTIDO_AGITADOR 11
#define SLEEP 12
#define PRESSAO_DOUT_PIN  A0
#define PRESSAO_SCK_PIN  A1
#define NIVEL A2

//#######################################################################################################################
// CONFIGURAÇÕES INICIAIS

// Define uma instancia do oneWire para comunicacao com o sensor de temperatura
OneWire oneWire(SENSOR_TEMPERATURA);
DallasTemperature sensors(&oneWire);
DeviceAddress ENDERECO_SENSOR_TEMPERATURA;

// Define uma instância do sensor de pressão
HX711 sensor_pressao;


// Define valvula 

Servo valvula;

//#######################################################################################################################
// PAINEL DE CONTROLE

//Define as chaves de controle
boolean CH_LIGA_BOMBA1 = 1;
boolean CH_LIGA_BOMBA2 = 1;
boolean CH_LIGA_VALVULA = 0;
boolean CH_LIBERA_SENSOR_TEMPERATURA = 0;
boolean CH_RESISTENCIA = 0;
boolean CH_HABILITA_AGITADOR = 1;
boolean CH_HABILITA_SENSOR_PRESSAO = 1;
boolean CH_HABILITA_SENSOR_NIVEL = 0;

//#######################################################################################################################
// VARIÁVEIS DO SISTEMA

float TEMPERATURA, VOLUME = 0, VOLUME_OBJETIVO = 0, Vi = 0.029, Ve = 0.0, PERIODO_INTERVALO_AGITADOR = 0, 
    PERIODO_DESLIGADO = 0;
int VELOCIDADE_AGITADOR = 0, 
    VELOCIDADE_AGITADOR_ANTERIOR = 0,
    TEMPERATURA_RESISTENCIA = 0, 
    PWM_BOMBA1 = 0, PWM_BOMBA2 = 0,
    ABERTURA_VALVULA = 0,
    TE = 1; // Responsável pelo tempo de exibição dos dados.

long TEMPO;
long PERIODO_DESLIGADO_TEMPO = 0;
long TEMPO_VALVULA = 0;
    
boolean LEITURA_NIVEL = 0; 

const long PRESSAO_OFFSET = 50682624;
const long PRESSAO_DIVIDER = 5895655;

float PERIODO_AGITADOR; // Variável que vai guardar o tempo dos intervalos de ativação do agitador
boolean PERIODO = false; // Boolean dependente do perído definido.

void setup()
{
   pinMode(STEP_AGITADOR, OUTPUT);
   digitalWrite(STEP_AGITADOR, HIGH);

   pinMode(SENTIDO_AGITADOR, OUTPUT);
   digitalWrite(SENTIDO_AGITADOR, LOW);

   pinMode(SLEEP, OUTPUT);
   digitalWrite(SLEEP, LOW);
   
   // Inicializa a comunicação serial
   Serial.begin(9600);

   // Configura o sensor de temperatura
   sensors.begin();
   sensors.getAddress(ENDERECO_SENSOR_TEMPERATURA, 0);


   pinMode(PWM_BOMB_1, OUTPUT);
   digitalWrite(PWM_BOMB_1, LOW);

   pinMode(PWM_BOMB_2, OUTPUT);
   digitalWrite(PWM_BOMB_1, LOW);

   valvula.attach(8);
//   pinMode(VALVULA, OUTPUT);
//   digitalWrite(PWM_BOMB_1, LOW);

   pinMode(RESISTENCIA, OUTPUT);
   digitalWrite(PWM_BOMB_1, LOW);

   pinMode(NIVEL, INPUT);

   sensor_pressao.begin(PRESSAO_DOUT_PIN, PRESSAO_SCK_PIN);
   sensor_pressao.set_scale(PRESSAO_DIVIDER);
   sensor_pressao.set_offset(PRESSAO_OFFSET);

   sensor_pressao.tare();

   muda_valvula();

}

void loop()
{

  //verifica se algo foi digitado no canal serial
  le_informacao();

  if(CH_HABILITA_SENSOR_NIVEL)
  {
    LEITURA_NIVEL = (analogRead(NIVEL) < 100) ? 1 : 0;
  }

  if(CH_LIBERA_SENSOR_TEMPERATURA)
    le_sensor_temperatura();

  if(CH_RESISTENCIA)
    aciona_resistencia();

  if(CH_HABILITA_AGITADOR)
    aciona_agitador();

  if(CH_HABILITA_SENSOR_PRESSAO)
    le_sensor_pressao();

  if(VOLUME_OBJETIVO != 0)
  { 
      if((VOLUME_OBJETIVO - VOLUME) < -0.002)
      {
        ABERTURA_VALVULA = (abs(VOLUME_OBJETIVO - VOLUME)/VOLUME_OBJETIVO)*100;
        if(ABERTURA_VALVULA < 60)
          ABERTURA_VALVULA = 60;
          
        muda_valvula();

        desativa_bombas();
      }
      else if(((VOLUME_OBJETIVO - VOLUME) > 0.002) && !LEITURA_NIVEL)
      {
        ABERTURA_VALVULA = 0;
        muda_valvula();

        int delta = abs((VOLUME_OBJETIVO - VOLUME)/VOLUME_OBJETIVO)*100;
        if(delta < 20)
        {
          PWM_BOMBA1 = 2;
          PWM_BOMBA2 = 2;
        }
        else
        {
          PWM_BOMBA1 = delta;
          PWM_BOMBA2 = delta;
        }
        
        liga_bomba1();
        liga_bomba2();
      }
      else if((abs(VOLUME_OBJETIVO - VOLUME) <= 0.004))
      {
        ABERTURA_VALVULA = 0;
        muda_valvula();
        
        desativa_bombas();
        VOLUME_OBJETIVO = 0;
        
      }
    
  }

  if(CH_LIGA_VALVULA)
    muda_valvula();

  if((millis() - TEMPO)/1000 > TE)
  {
    exibe_dados();
    TEMPO = millis();
  }
  
}

void le_informacao()
{
  if(Serial.available() > 0)
  {
    // Lê primeiro byte digitado pelo usuário e atua no sistema
      switch (Serial.read())
      {
        case 'B':
                    delay(2);
                    if(Serial.read() == '1')
                    {
                      PWM_BOMBA1 = Serial.parseInt();
                    }
                    else
                    {
                      PWM_BOMBA2 = Serial.parseInt();
                    }  

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
                        PERIODO_INTERVALO_AGITADOR = Serial.parseFloat();
                    if((Serial.available() > 0) && (Serial.read() == 'D'))
                        PERIODO_DESLIGADO = Serial.parseFloat();
                    if(VELOCIDADE_AGITADOR > 160) //156
                        VELOCIDADE_AGITADOR = 160;
                    else if(VELOCIDADE_AGITADOR < 30 && VELOCIDADE_AGITADOR != 0)
                    {
                      VELOCIDADE_AGITADOR = 30;
                      Serial.println("Velocidade mínima: 30rpm");
                    }
                    break;
        case 'V':
                    ABERTURA_VALVULA = Serial.parseInt();
                    break;
        case 'L': 
                    if(CH_HABILITA_SENSOR_PRESSAO)  //if((CH_LIGA_BOMBA1 || CH_LIGA_BOMBA2) && (CH_LIGA_VALVULA))
                    {
                      VOLUME_OBJETIVO = Serial.parseFloat();
                      Serial.print("Volume objetivo: ");
                      Serial.println(VOLUME_OBJETIVO);
                    }
                    if(LEITURA_NIVEL && VOLUME_OBJETIVO < 5)
                    {
                      
                    }
                    break;
        case 'S':
                    if(Serial.available() > 0)
                    {
                      switch (Serial.read())
                      {
                          case 'i':
                                    Vi = Serial.parseFloat();
                                    Serial.println("Volume Ajustado!");
                                    break;
                          case 'v': 
                                    sensor_pressao.tare();
                                    Serial.println("Volume Zerado!");
                                    Vi = 0;
                                    break;
                          case 'E':
                                    TE = Serial.parseInt();
                                    Serial.println("Tempo de exibicao ajustado!");
                                    break;

                      }
                    }

        default:
                    //Serial.println("Opção inválida!");
                    break;
      }
   }

}

void liga_bomba1()
{
    analogWrite(PWM_BOMB_1, map(PWM_BOMBA1,0,100,0,253));
}

void liga_bomba2()
{
    analogWrite(PWM_BOMB_2, map(PWM_BOMBA2,0,100,0,253));  
}

void desativa_bombas()
{
        PWM_BOMBA1 = 0;
        PWM_BOMBA2 = 0;
        liga_bomba1();
        liga_bomba2();
}

void muda_valvula()
{
   if(valvula.read() != map(ABERTURA_VALVULA, 0, 100, 50, 180))
   {
      valvula.attach(8);
      valvula.write(map(ABERTURA_VALVULA, 0, 100, 50, 180));
      TEMPO_VALVULA = millis();
   }
   
   if((millis() - TEMPO_VALVULA)/1000 > 1)
   {
        valvula.detach();
        digitalWrite(8, LOW);
   }
   
}

void le_sensor_temperatura()
{
  sensors.requestTemperatures();
  TEMPERATURA = sensors.getTempC(ENDERECO_SENSOR_TEMPERATURA);
}

void le_sensor_pressao()
{
    //VOLUME = (sensor_pressao.get_units()*25.795 + Vi);
   
    if(sensor_pressao.is_ready())
    VOLUME = (sensor_pressao.read_average(20)*4.229885286/1000+7188.26145767)/1000 + Vi;
    //VOLUME = (sensor_pressao.read()*4229885.285946/1000000+7188.26145767);
    
}

void aciona_resistencia()
{
  // Verifica se a temperatura desejada é maior ou menor do que a temperatura de leitura do sensor

  if(TEMPERATURA > TEMPERATURA_RESISTENCIA)
  digitalWrite(RESISTENCIA, LOW);

  if(TEMPERATURA < TEMPERATURA_RESISTENCIA)
  digitalWrite(RESISTENCIA, HIGH);
}

void aciona_agitador()
{  
  int step = 15;
  // Implementa uma variação suave de velocidade
  if(abs(VELOCIDADE_AGITADOR - VELOCIDADE_AGITADOR_ANTERIOR) <= step)
  {
      VELOCIDADE_AGITADOR_ANTERIOR = VELOCIDADE_AGITADOR;
  }
  else if((VELOCIDADE_AGITADOR - VELOCIDADE_AGITADOR_ANTERIOR) > step)
      VELOCIDADE_AGITADOR_ANTERIOR+= step;
      
  else if((VELOCIDADE_AGITADOR - VELOCIDADE_AGITADOR_ANTERIOR) < step)
      VELOCIDADE_AGITADOR_ANTERIOR-= step;

  define_periodo();
  
  if((VELOCIDADE_AGITADOR == 0) || not(PERIODO))
  {
      noTone(STEP_AGITADOR);
      digitalWrite(SLEEP, LOW);
  }
  else if((VELOCIDADE_AGITADOR != 0) && PERIODO)
  {
      digitalWrite(SLEEP, HIGH);
      tone(STEP_AGITADOR, VELOCIDADE_AGITADOR_ANTERIOR*200/60); // Manda uma onda quadrada para o driver do motor com a velocidade em rpm desejada.
  }
}

void define_periodo()
{
    if(not(PERIODO))  // Permite usar o acionamento suave no motor de passo usando os períodos de intervalo.
     VELOCIDADE_AGITADOR_ANTERIOR = 0;

    if(PERIODO_INTERVALO_AGITADOR != 0)
    {
      if((millis() - PERIODO_AGITADOR)/1000 >= (PERIODO_INTERVALO_AGITADOR+PERIODO_DESLIGADO))
      {
         PERIODO = false;
         PERIODO_DESLIGADO_TEMPO = millis(); 
         PERIODO_AGITADOR = millis();
      }
      else if((millis() - PERIODO_DESLIGADO_TEMPO)/1000 >= PERIODO_DESLIGADO)
      {
            PERIODO = true; 
      }     
     }
    else
     PERIODO = true;
}
void exibe_dados()
{
  Serial.print("\n************************************************************************\n");

  if(CH_HABILITA_AGITADOR)
  {
    Serial.print("Velocidade do agitador (rpm): ");
    Serial.print(VELOCIDADE_AGITADOR_ANTERIOR);

    if(PERIODO_INTERVALO_AGITADOR != 0)
    {
      Serial.print(" Intervalo do agitador (s): ");
      Serial.print(PERIODO_INTERVALO_AGITADOR);
    }

    if(PERIODO_DESLIGADO != 0)
    {
      Serial.print(" Periodo desligado (s): ");
      Serial.print(PERIODO_DESLIGADO);
    }
    Serial.print("\n");
  }

  if(CH_LIGA_BOMBA1)
  {
    Serial.print("Bomba 1: ");
    Serial.print(PWM_BOMBA1);
    Serial.print("%");
  }

  if(CH_LIGA_BOMBA2)
  {
    Serial.print(" Bomba 2: ");
    Serial.println(PWM_BOMBA2);
  }


  if(CH_LIGA_VALVULA)
  {
    Serial.print("Valvula: ");
    Serial.print(ABERTURA_VALVULA);
    Serial.println("%");
  }
  if(CH_RESISTENCIA)
  {
    Serial.print("Temp. Resistência: ");
    Serial.print(TEMPERATURA_RESISTENCIA);
  }

  if(CH_LIBERA_SENSOR_TEMPERATURA)
  {
    Serial.print("Temp ºC: ");
    Serial.println(TEMPERATURA);
  }

  if(CH_HABILITA_SENSOR_PRESSAO)
  {
    Serial.print("Volume: ");
    Serial.print(VOLUME, 3);
  }
  
  Serial.println(" ");
}
