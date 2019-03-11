
#define bomba 6 // Define pinos para o pwm da bomba 1.
#define leitura_fluxo 7

float duration, FLUXO;

int PWM_BOMBA;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
 
  pinMode(bomba, OUTPUT);
}

void loop() 
{
  while (Serial.available() > 2)
  {
      
      PWM_BOMBA = Serial.parseInt();  
  }

  analogWrite(bomba, PWM_BOMBA);

  duration = pulseIn(leitura_fluxo, LOW);
  float PERIODO = duration*2;
  FLUXO = (100/PERIODO)/98;
  //Serial.print("Fluxo ml/min: ");
  Serial.println(FLUXO*100000*1.16,3);

  //Serial.print("PEÍODO: ");
  //Serial.println(PERIODO,3);
 

  delay(300);
   
}
