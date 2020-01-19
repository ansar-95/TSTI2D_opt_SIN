
//***********************Bibliothèque*************************************
#include <SimpleDHT.h>
#include <SPI.h>
#include <Ethernet.h>

#include <Wire.h>
#include "DS1307.h"

DS1307 clock;//define a object of DS1307 class
#include <GSM.h>
#define PINNUMBER "264843"







SimpleDHT22 dht22(pinDHT22);
//**********************DHT22******************************************************
int pinDHT22 = 8; // CApteur DHT22
int ledH = 9;      //led pour le brumisateur
int ledT = 4;     //led pour le chauffage
char c;
String  Humi;
String Temp;
float temperature = 0;
float humidity = 0;
int C1consigneH = 0;
int C2consigneT = 0;
//****************************RTC DS 1307***************************************************
int ledL = 6;     //led pour la lumière
long tabNombre[7];
int annee;

int mois;

int jour;

int heurea;
int heuree;

int minutesa;
int minutese;

int seconde;

int jourdelasemaine;

String heurrrr;
String minutesss;
String heurrrr2;
String minutesss2;

//*****************CapteurUV********************************
GSM gsmAccess;
GSM_SMS sms;
char txtMsg[200] = "Le niveau de la lampe UV est inferieur a 50%, merci de changer la lampe le plus rapidement possible";
char remoteNum[20] = "0769621300";

int etateclairage = 0;

float sensorVoltage;
float sensorValue;
float UVIndex;


//*****************ServeurWeb********************************

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0F, 0x22, 0xC5  //Adresse MAC de la carte ETHERNET SHIELD arduino
};
IPAddress ip(172, 20, 199, 240); // Adresse ip de carte ethernet


EthernetServer server(80); // Utilise le port 80

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());


  pinMode(ledH, OUTPUT);
  pinMode(ledT, OUTPUT);
  pinMode(ledL, OUTPUT);

   boolean notConnected = true;
   
    while (notConnected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      notConnected = false;
    } else {
      Serial.println("Not connected");
      delay(1000);


    }
  }

}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        c = client.read();
        //******************Si l'application reçoit des information la carte arduino va le lire par caractere************

        if (c == ']') {      /* Humidite     ex: {\"humidite\":\"25\"} */

          client.read();                         //lis une caractere après les deux points ex : anti-slash
          client.read();                        //lis une caractere après l'anti-slash ex : "
          Humi = (char)client.read();          // Conversion de char en string  Humi vaut de 2 en string
          Humi = Humi + (char)client.read();  // Conversion de char en string  Rassemble les deux valeurs Humi vaut 25 en string
          Serial.println(Humi.toInt());      //on l'affiche sur le serial monitor transformation de Humi de string en int


          C1consigneH = Humi.toInt();
          Serial.println(Humi+"claer");

        }

        if (c == '[') { // Temeprature même principe


          client.read();
          client.read();
          Temp = (char)client.read();           // Conversion de char en string
          Temp = Temp + (char)client.read();   // Conversion de char en string   Rassemble les deux valeurs
          Serial.println(Temp + "clear");

          Serial.print(Temp.toInt());


          C2consigneT = Temp.toInt();        // Transformation de char en int
        }

          if (c == '|'){        //heure d'allumage
            client.read();
            client.read();
            heurrrr = (char)client.read();
            client.read();
            client.read();
                      
            client.read();
            minutesss = (char)client.read();
            minutesss = minutesss + (char)client.read();
            Serial.println(heurrrr.toInt());
            Serial.println(minutesss.toInt());
            
            heurea = heurrrr.toInt();
            minutesa = minutesss.toInt();
            
            
            
          }

          if (c == '*'){    //heure d'extinction

            client.read();
            client.read();
            heurrrr2 = (char)client.read();
            heurrrr2 = heurrrr +(char)client.read();
            client.read();
            client.read();
            client.read();
            minutesss2 = (char)client.read();
            minutesss2 = minutesss + (char) client.read();
            
            Serial.println(heurrrr2.toInt());
            Serial.println(minutesss2.toInt());
            
            heuree = heurrrr2.toInt();
            heuree = minutesss2.toInt();
            
          }

          
            
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply

        if (c == '\n' && currentLineIsBlank) {
          client.print("{\"humidite\":");     //Affichage sur le page web
          client.print(humidity);             //Affichage sur le page web
          client.print(",\"Temperature\":");  //Affichage sur le page web
          client.print(temperature);          //Affichage sur le page web
          client.print("}");                  //Affichage sur le page web
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  //***************************CapteurDHT22******************************************************************
  Serial.println("=================================");
  Serial.println("Sample DHT22...");
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err=");
    Serial.println(err);
    delay(2000);
    return;
  }
  //**********************************Humidite***************************************************************
  Serial.print("Sample OK: ");
  Serial.print((float)humidity);
  Serial.println(" RH%");

  if (humidity < C1consigneH) {

    digitalWrite(ledH, HIGH);
    delay(10000);
    digitalWrite(ledH, LOW);
  }

  //*********************************Temperature************************************************************
  Serial.print("Sample OK: ");
  Serial.print((float)temperature);
  Serial.println("°C");


  if (temperature < C2consigneT) {

    digitalWrite(ledT, HIGH);
  }




  
//********************************lumière*********************************************************************
  if (clock.hour == heurea && clock.minute == minutesa ) {
    
    digitalWrite(ledL, HIGH);
   etateclairage=1;
  
   if (etateclairage == 1) {
    sensorValue = analogRead(A0);
    sensorVoltage = sensorValue / 1024 * 5 * 1000;

    Serial.print("sensor reading = ");
    Serial.print(sensorValue);
    Serial.println("");

    Serial.print("sensor voltage = ");
    Serial.print(sensorVoltage);
    Serial.println(" mV");

    Serial.print("UV Index = ");
    Serial.println(UVIndex);

    delay(1000);

     if (sensorValue <= 50) {
      UVIndex = 1;

      // send the message
      sms.beginSMS(remoteNum);
      sms.print(txtMsg);
      sms.endSMS();
      Serial.println("\MESSAGE ENVOYE AVEC SUCCES ! \n");

      delay(10000);
    }
     else {
      if (sensorValue <= 65) {
        UVIndex = 2.00;
      }

      else {
        if (sensorValue <= 83) {
          UVIndex = 3.00;
        }

        else {
          if (sensorValue <= 103) {
            UVIndex = 4.00;
          }

          else {
            if (sensorValue <= 124) {
              UVIndex = 5.00;
            }

            else {
              if (sensorValue <= 142) {
                UVIndex = 6.00;
              }

              else {
                if (sensorValue <= 162) {
                  UVIndex = 7.00;
                }

                else {
                  if (sensorValue <= 180) {
                    UVIndex = 8.00;
                  }

                  else {
                    if (sensorValue <= 200) {
                      UVIndex = 9.00;
                    }

                    else {
                      if (sensorValue <= 221) {
                        UVIndex = 10.00  ;
                      }

                      else {
                        if (sensorValue <= 240) {
                          UVIndex = 11.00;
                        }

                        else {
                          if (sensorValue > 240) {
                            UVIndex = 12.00;
                          }

                        }


                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

  }
  }    
  
  if  (clock.hour == heuree && minutese ) {
    digitalWrite(ledL, LOW);
  }
  

}
/*Function: Display time on the serial monitor*/
void printTime()
{
  clock.getTime();
  Serial.print(clock.hour, DEC);
  Serial.print(":");
  Serial.print(clock.minute, DEC);
  Serial.print(":");
  Serial.print(clock.second, DEC);
  Serial.print("  ");
  Serial.print(clock.month, DEC);
  Serial.print("/");
  Serial.print(clock.dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(clock.year + 2000, DEC);
  Serial.print(" ");
  Serial.print(clock.dayOfMonth);
  Serial.print("*");
  switch (clock.dayOfWeek)// Friendly printout the weekday
  {
    case MON:
      Serial.print("MON");
      break;
    case TUE:
      Serial.print("TUE");
      break;
    case WED:
      Serial.print("WED");
      break;
    case THU:
      Serial.print("THU");
      break;
    case FRI:
      Serial.print("FRI");
      break;
    case SAT:
      Serial.print("SAT");
      break;
    case SUN:
      Serial.print("SUN");
      break;
  }


  Serial.println(" ");

  
  delay(2000);
}
