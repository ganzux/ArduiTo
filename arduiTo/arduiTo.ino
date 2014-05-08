#include <dht11.h>
#include "etherShield.h"
#include "ETHER_28J60.h"
#include <SPI.h>
#include <Ethernet.h>

dht11 DHT11;

static uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
static uint8_t ip[4] = {192, 168, 1, 25};
static uint16_t port = 80;

int ledB = 5;
int ledG = 6;
int ledR = 7;

int rele = 8;

int pulsador = 4;

int luz = 3;

#define DHT11PIN 2

long randNumber;

int estadoLuz = LOW;

ETHER_28J60 ethernet;

int chk;
int hayLuz;

void setup() {
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(rele, OUTPUT);

  pinMode(pulsador, INPUT);
  pinMode(luz, INPUT);

  digitalWrite(ledR, LOW);
  digitalWrite(ledG, LOW);
  digitalWrite(ledB, LOW);
  digitalWrite(rele,LOW);

  Serial.begin(9600);
  
  digitalWrite(ledR, HIGH); delay(1000); digitalWrite(ledR, LOW);
  digitalWrite(ledG, HIGH); delay(1000); digitalWrite(ledG, LOW);
  digitalWrite(ledB, HIGH); delay(1000); digitalWrite(ledB, LOW);
  digitalWrite(rele,HIGH); delay(1000);  digitalWrite(rele, LOW);
  
  Serial.println("Trying to connect...");

  ethernet.setup(mac, ip, port);  
  randomSeed( analogRead(0) );
  
  Serial.println("Init values...");
  chk = DHT11.read(DHT11PIN);
  hayLuz  = analogRead(luz);

  digitalWrite(ledB, HIGH); delay(100); digitalWrite(ledB, LOW); delay(100);
  digitalWrite(ledB, HIGH); delay(100); digitalWrite(ledB, LOW); delay(100);
  digitalWrite(ledB, HIGH); delay(100); digitalWrite(ledB, LOW); delay(100);
  digitalWrite(ledB, HIGH); delay(100); digitalWrite(ledB, LOW); delay(100);
  digitalWrite(ledB, HIGH); delay(100); digitalWrite(ledB, LOW); delay(100);
  
  Serial.println("Configurations OK !");
}

// the loop routine runs over and over again forever:
void loop() {

  char* params;
 
  if (params =ethernet.serviceRequest()) {
     chk = DHT11.read(DHT11PIN);
     hayLuz  = analogRead(luz);
     switch (chk) {
      case 0: Serial.println("OK"); break;
      case -1: Serial.println("Checksum error"); break;
      case -2: Serial.println("Time out error"); break;
      default: Serial.println("Unknown error"); break;
    }
     web( ethernet,params );
     params = 0;
  }
  
  delay(100);

  int pulsado = digitalRead(pulsador);
  randNumber = random(3);

  temperatureColor( (float)DHT11.temperature );

  if ( pulsado == HIGH ){
   
    if ( estadoLuz == LOW ){
      turnOn();
    }
    else {
      turnOff();
    }
    
    delay(500);

  }

}

void turnOn(){
  Serial.println("Light on");
  estadoLuz = HIGH;
  digitalWrite(ledB, estadoLuz);
  digitalWrite(rele,estadoLuz);
}

void turnOff(){
  Serial.println("Light off");
  estadoLuz = LOW;
  digitalWrite(ledB, estadoLuz);
  digitalWrite(rele,estadoLuz);
}

void temperatureColor( int celsius ){
  if( celsius > 26 ){
    digitalWrite(ledR, HIGH);
    digitalWrite(ledG, LOW);
    digitalWrite(ledB, LOW);
  } else {
    digitalWrite(ledR, LOW);
    digitalWrite(ledG, HIGH);
    digitalWrite(ledB, LOW);
  }
}

void web(ETHER_28J60 ethernet,char* params){
  
  Serial.println("WEB REQUEST INIT");
  
  if (strcmp(params, "?luz=1") == 0){
    turnOn();
  } else if (strcmp(params, "?luz=0") == 0){
    turnOff();
  }
     
  ethernet.print("<TITLE>ArduiTo</TITLE>");
  ethernet.print("<link rel='stylesheet' href='http://test.lanxiuris.es/css/style.css' />");
  ethernet.print("<meta http-equiv='refresh' content='30' >");

  ethernet.print("<H1>ArduiTo</H1>");
  ethernet.print("<p><img src='http://test.lanxiuris.es/images/a1.png' style='vertical-align: middle;' alt='Temperatura' />");
  ethernet.print( (float)DHT11.temperature );
  ethernet.print("</p>");
  ethernet.print("<p><img src='http://test.lanxiuris.es/images/a2.png' style='vertical-align: middle;' alt='Humedad' />");
  ethernet.print( (float)DHT11.humidity );

  ethernet.print("<form method='GET'>");
  if ( estadoLuz == HIGH ){
    ethernet.print("<input type='submit' name='luz' value='0' />");
  } else {
    ethernet.print("<input type='submit' name='luz' value='1'/>");
  }
  ethernet.print("</form>");
  
  Serial.println("WEB REQUEST FORMED");

  ethernet.respond();
  
    Serial.println("WEB REQUEST END");
}



double Fahrenheit(float celsius) {
   return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius) {
        return celsius + 273.15;
}

double dewPoint(double celsius, double humidity) {
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity) {
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}
