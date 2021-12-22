#define peak 20             // restarle 2 del retardo inicial

unsigned int cont = 0;      // crea una variable numerica para el contador 
unsigned char pulsos = 12;  // crea una variable para indicar el numero de pulsos   
//unsigned char aux = 0XFF;   // crea una variable auxiliar "aux" con valor de 255
unsigned int remanente = 300; // crea una variable numerica
double rem_aux = 0;        //crea una variable numerica flotante

unsigned int conv = 0;
unsigned int temp = 0;            // registro lectura PB.0, es donde se guarda el valor 

void setup() 
{
  cli();        // apagas las interrupciones generales stop interrupts

  // 
  DDRB = 0B00000000;    // declaro el puerto B "PORTB" como entradas "INPUTS" 
  PORTB = 0B00000001;   // declaro el pin 0 "PIN B.0" como entrada "INPUT PULL-UP ==> PIN 8" 
  
  // put your setup code here, to run once:
  
  DDRD = 0XF0;          // Declaro como salidas pines 4, 5, 6 y 7 "PD4, 5 ,6 , 7 OUTPUTS"
  PORTD = 0X0F;         // Coloco en nivel alto puerto D "OUTPUTS LOW LEVEL - INPUTS PULL UP" 
    
  TCCR0A = 0B00000010;      // Registro del control del Timer/Contador "CTC MODE", bit7, bit5:4 operación normal, bit3:2 siempre cero,   
  OCR0A = 99;               // CLCK FSM = 50us
  TCNT0 = 0;                // RST TIM
  TIMSK0 = 0B00000010;      // INT COMPA
  
  TCCR0B = 0B00000010;      // CLKTIM = CLKSYS / 8 & START TIM

  EICRA = 0B00000010;       // CAMBIAR 2 ULTIMOS BITS A 11 PARA RISING EDGE INT
  EIMSK = 0B00000001;       // HABILITAR INT0

  sei();

  pinMode(13, OUTPUT);
  pinMode(10, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() 
{
  // pulsos setea el numero de pulsos dado por A0, en modo manual 
  conv = analogRead(A0);
  pulsos = map(conv, 0, 1023, 0, 100);

//  pulsos = 10;
  
  // remanente setea la frecuencia de cada ciclo en A1
  conv = analogRead(A1);
 // remanente = map(conv, 0, 1023, 20000, 200);    // ecuacion lineal de A1
  rem_aux = exp(9.9042 - 0.0045*conv);              // ecuacion exponencial de mapeo A1
  rem_aux--; 
  
  remanente = rem_aux;

  temp = PINB;                // Pin B lectura 
  temp &= 0x01;

  digitalWrite(13, (bool) temp);    // imagen de la senal temp

  String message = 'p' + String(pulsos) + 'f' + String(remanente);
  Serial.println(message);

  if (digitalRead(10) == HIGH)
  {
    cli();
  }
  else
    sei();
  
  _delay_ms(100);
}

ISR (TIMER0_COMPA_vect)
{
  if (cont == 0)
    PORTD |= 0B00010000;      // init cycle HIGH LEVEL
    
  if (cont < 2)
    PORTD &= 0B00011111;      // A & B & C LOW LEVEL

  else if (cont < peak)  
    PORTD |= 0B10100000;      // A & C HIGH LEVEL

  else if (cont < peak + 4)
    PORTD = 0B01111111;      // A LOW & B HIGH & C HIGH and keep cycle indicator & keep pull ups

  else if (cont < (pulsos + peak + 4))
  {
    PORTD ^= 0B01000000; 
  }

  else if (cont < remanente)
  {
    PORTD &= 0B00011111;      // CLEAR ALL keep cycle
  }

  else
  {
    PORTD &= 0B00001111;      // CLEAR ALL 
    cont = 0XFFFF;
  }

  if ((temp == 0) && (cont >= pulsos + peak + 4))
      cont = 300;             // Dejar el contador fuera del rango pero que no vuelva a cero para hacerlo con al INT0      
      
  cont++;
  
}

ISR (INT0_vect)
{
  if ((temp == 0) && (cont >= pulsos + peak + 4))
    cont = 0;
}
