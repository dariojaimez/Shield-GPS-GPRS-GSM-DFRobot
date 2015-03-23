/************************************************************************/
/*     Programme perso pour utiliser le shield GPS_GSM de DFRobot       */
/*     tout en un.                                                      */
/************************************************************************/

#include <SoftwareSerial.h>  // Uniquement pour le débogage.

#define gps_enable()    digitalWrite (4, LOW)
#define gps_disable()   digitalWrite (4, HIGH)

#define gsm_enable()    digitalWrite (3, LOW)
#define gsm_disable()   digitalWrite (3, HIGH)

#define COLLECTING 1
#define WAITING 0
#define DONE 2
#define LF 0x0a
#define CR 0x0d

SoftwareSerial mySerial(10, 11); // RX, TX (Uniquement pour le débogage).

int state;
int analogPin = 0;
String trame;
String str;
char inchar;
int ledpin = 13;

void start_gsm ()  // Fonction pour passer en mode GSM.
{
    gsm_enable ();
	delay(10);
	gps_disable ();
		
	delay (5000);
    delay (5000);
	delay (5000);
	Serial.println("AT+CMGD=1,4"); //Effacer tout les SMS en memoire.
	mySerial.println("startAT+CMGD=1,4");
}

void start_gps ()  // Fonction pour passer en mode GPS.
{
    gsm_enable ();
	delay(10);
    gps_disable ();
	
	delay (2000);
	delay (5000);
    delay (5000);
   
    mySerial.println ("waiting for GPS! ");  
	   
    Serial.println ("AT");
    mySerial.println ("Send AT");
    delay (2000);
	
	    Serial.println ("AT");
	    mySerial.println ("Send AT");
	    delay (2000);
		
    Serial.println ("AT+CGPSPWR=1");
    mySerial.println ("Send AT+CGPSPWR=1");
   
    delay (2000);
    Serial.println ("AT+CGPSRST=1");
    mySerial.println ("Send AT+CGPSRST=1");
   
    delay (2000); 
    gsm_disable ();
    gps_enable ();
    delay (2000);   
    mySerial.println ("$GPGGA information: ");
	
	/************************************************************************/
	/* Détection trame GPGGA                                                                     */
	/************************************************************************/
	trame = "";
	state = 0;
	while (true)
	{   
		if (Serial.available() > 0)
		{
			char ch = Serial.read();
			if (ch=='$')
			{	
				state = COLLECTING;
				trame = "";
			}
			else if (ch==CR || ch==LF)
			{
				state = DONE;
			}
			if (state==COLLECTING)
			{
				trame += ch;
				mySerial.print(ch);
			}
		}
		if (state==DONE)
		{
			if (trame.startsWith("$GPGGA")==true)
			{
				/************************************************************************/
				/* Décodage Trame GPGGA (Latitude et Longitude.                         */
				/************************************************************************/
				int Virg1 = trame.indexOf(',');  // On cherche les virgules de la trame.
				int Virg2 = trame.indexOf(',', Virg1 + 1 );
				int Virg3 = trame.indexOf(',', Virg2 + 1 );
				int Virg4 = trame.indexOf(',', Virg3 + 1 );
				int Virg5 = trame.indexOf(',', Virg4 + 1 );
				int Virg6 = trame.indexOf(',', Virg5 + 1 );
				String latitude = (trame.substring(Virg2+1,Virg4+1));
				String longitude = (trame.substring(Virg4+1,Virg6));
				latitude += longitude;
					
				if (Virg2 != Virg3 - 1)  //Si on a recu la latitude et longitude.
					{ 
						mySerial.println(latitude);
						send_string("XXXXXXXXXX",latitude); //Envoi SMS coordonnées GPS.
						break;								// Remplacer XXXX par numero de telephone.
				    }		
			}
			state = WAITING;
		}			
	} // Fin while detection trame.	
}

// Envoi d'un SMS (n° de tel + texte du SMS):
// Utilisé pour l'envoi coordonnées GPS
void send_string (char* numble,String string) {
        char num_buf[25];
        sprintf (num_buf, "AT+CMGS=\"%s\"", numble);
        gsm_enable ();
        gps_disable ();
        delay (2000);
        Serial.println ("AT");
        delay (200);
        Serial.println ("AT");
        delay (200);
        Serial.println ("AT+CMGF=1");
        delay (200);
        Serial.println (num_buf);
        delay (200);
        Serial.println (string);
        Serial.write (26);  // Envoi du SMS (26).
}
// Envoi d'un SMS (n° de tel + texte du SMS):
// Utilisé pour l'envoi coordonnées valeur analogique.
 
void send_string2 (char* numble, char* volt) {
	char num_buf1[25];
	sprintf (num_buf1, "AT+CMGS=\"%s\"", numble);
	delay (2000);
	Serial.println ("AT");
	delay (200);
	Serial.println ("AT");
	delay (200);
	Serial.println ("AT+CMGF=1");
	delay (200);
	Serial.println (num_buf1);
	delay (1000);
	Serial.println(volt);
	Serial.write (26);
}
 

void setup() 
{
	pinMode(3,OUTPUT);
	pinMode(4,OUTPUT);
	pinMode(5,OUTPUT);
	
	Serial.begin(9600);
	mySerial.begin(115200);
	state = WAITING;
	
	digitalWrite (5, HIGH);
	delay (1500);
	digitalWrite (5, LOW);
	delay (1500);
		
    start_gsm();
}

void loop()  // Boucle principale.
{
	/************************************************************************/
	/* Réception SMS                                                        */
	/************************************************************************/
	debut:
	 if(Serial.available()>0)
	 
	 {
		 inchar=Serial.read();
		 //mySerial.println(inchar);
		 if(inchar=='T')
		 {
			 delay(10);
			 inchar=Serial.read();
			 if (inchar=='I')  //GSM module recoit message,il envoie'+CMTI "SM", 1' sur le port série.
			 {
				 delay(10);
				 Serial.println("AT+CMGR=1");    // On demande l'envoi du SMS sur le port série.
												 //  Maintenant on va le lire 
				                                 // sur les lignes de progamme qui suivent.
				 //mySerial.println("AT+CMGR=1");          
				 delay(10);
			 }	 
	     }		 	 
		 else if (inchar=='L')                   // On test la première lettre du message.
		 {
			 delay(10);
			 inchar=Serial.read();
			 if (inchar=='H')                    //On test la deuxiéme lettre du message.
			 {
				 delay(10);
				 digitalWrite(ledpin,HIGH); 
				 mySerial.println("led on");                        //Turn on led
				 delay(50);
				 Serial.println("AT+CMGD=1,4");                    //Delete all message
				 delay(500);				
			 }
			 if (inchar=='L')     //The SMS("LH") was display in the Serial port, and Arduino has recognize it.
			 {
				 delay(10);
				 digitalWrite(ledpin,LOW);  
				 mySerial.println("led off");                       //Turn off led
				 delay(50);
				 Serial.println("AT+CMGD=1,4");                   //Delete all message
				 delay(500);				 
			 }
			 if (inchar=='G')     //The SMS("LG") was display in the Serial port, and Arduino has recognize it.
			 {	//mySerial.println("sms lg reconnu !!");
				start_gps();
				//mySerial.println("retour gps");
				start_gsm();
				delay(50);
	    		Serial.println("AT+CMGD=1,4");                   //Delete all message
				delay(500);
			 }
			 if (inchar=='A')       //The SMS("LA") was display in the Serial port, and Arduino has recognize it.
			 {	
				 delay(10);
				 int val =  analogRead(analogPin);
				 delay(50);
				 Serial.println("AT+CMGD=1,4");                   //Delete all message
				 delay(500);
				 mySerial.println(val);
				 float tension = val*5;
				 tension = tension / 1023;                      
				 
				 char nu1[25];
				 char tension_sortie[25];				 
				 dtostrf(tension,4,3,nu1);
				 sprintf (tension_sortie, "Tension A0 arduino = %s V", nu1);
				 send_string2("XXXXXXXXXX",tension_sortie);	// Remplacer XXXXXXXX par votre N° de telephone.	  			 
			 }
				 delay(50);
				 Serial.println("AT+CMGD=1,4");                   //Delete all message
				 delay(500);	 
		 }
	 }
} // Fin void Loop.
