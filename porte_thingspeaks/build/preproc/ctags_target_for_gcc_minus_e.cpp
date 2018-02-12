# 1 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino"
# 1 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino"
# 2 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino" 2
# 3 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino" 2
# 4 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino" 2
# 5 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino" 2
//#include <Codes_CGU.h>// contient toutes les variables avec les loggins et les codes
# 7 "C:\\Users\\cgueble\\Documents\\Arduino\\ESP8266_Door_Status\\porte_thingspeaks\\porte_thingspeaks.ino" 2
//****************Configuration pour Debug****************
const boolean SERIAL_PORT_LOG_ENABLE = true; //true pour avoir la console active et false pour la desactiver ; il faut la desactiver pour l'application car meme pour que "verrou"
const boolean AvecMail = true; //true pour activer les mails

//Declaration des compteurs
int Cpt_Verrou = 0;
int CptTrig = 0;
int CptTrigMax = 20;

//Connexion a Internet
/*const char* ssid0 = "YourSSID_0";//type your ssid
  const char* password0 = "YourPWD_0";//type your password
  const char* ssid1 = "YourSSID_1";//type your ssid
  const char* password1 = "YourPWD_1";//type your password
  const char* ssid2 = "YourSSID_2";//type your ssid
  const char* password2 = "YourPWD_2";//type your password
*/
String CurentSSID;
String CurentSSIDTry;
//Definition des chaines de characteres pour les mails
String MailContent = "no mail content defined\r\n";
String StringDate = "no date";
String StringTime = "no time";
String Release_Date = "12-02-2018";
//Definition des Inputs
const int IlsPorte = 2; //l'acquisition du statut de la porte se fera sur GPIO2 c'est a dire D2 c'est a dire la pin 3 de la carte ESP8266-E01 ; c'est une INPUT
const int Verrou = 3; //l'acquisition du statut du verrou se fera sur GPIO3 c'est a dire D8 c'est a dire la pin 7 de la carte ESP8266-E01 ; c'est une INPUT

//Definition des Outputs

//Variables de gestion de l'etat de la porte et des mails
char server_email[] = "smtp.orange.fr";
const unsigned int localPort = 2390;

boolean Status_IlsPorte = false;
boolean Status_Verrou = false;

boolean Old_Status_IlsPorte = false;
boolean Old_Status_Verrou = true;
boolean Status_Verrou_Change = false;
boolean TraitementVerrouEnCours = false; // true pour iniber la fonction verrou

int MaxDebounse = 8;

boolean statusCheck = false;
String MailToSend = "";
boolean DailyMailSent = true;
boolean DoorClosedBack = true; //true : porte refermee
boolean Update_needed = false;
long rssi; //pour mesure RSSI

//variables pour le gestion de la date et du temps
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long secsSince1900 = 0;
const unsigned long seventyYears = 2208988800UL;
int Hour;
int Minute;
int Second;
int Day;
int DayofWeek; // Sunday is day 0
int Month; // Jan is month 0
int Year; // the Year minus 1900
int WeekDay;

//Variable pour la gestion de thingspeak
const char* server = "mqtt.thingspeak.com";// Define the ThingSpeak MQTT broker
unsigned long lastConnectionTime = 0; // track the last connection time
const unsigned long postingInterval = 1L * 1000L;// post data every 1 seconds
const unsigned long RegularpostingInterval = 10 * 60 * 1L * 1000L;// post data every 10 min
const unsigned long whatchDogValue = 60 * 60 * 1000L;// WhatchDog Value 60min
time_t epoch = 0; // contient
IPAddress ip;
IPAddress timeServerIP;

WiFiClient client; // Initialize the Wifi client library.
WiFiUDP udp; //A UDP instance to let us send and receive packets over UDP
PubSubClient mqttClient(client); // Initialize the PuBSubClient library

//************************Debut de setup*************************
void setup() {
  delay(5000);
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.begin(115200);
    Serial.print("debut de setup. SERIAL_PORT_LOG_ENABLE= ");
    Serial.println(SERIAL_PORT_LOG_ENABLE);
  }
  //Set up PIN in INPUT
  pinMode(IlsPorte, 0x00);//External PullDown
  pinMode(Verrou, 0x02);

  //Set up PIN in OUTPUT


  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((section(".irom.text"))) = ("IlsPorte= 2 (GPIO2), INPUT ExtPulldown"); &__c[0];}))))));
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((section(".irom.text"))) = ("Verrou= 3 (GPIO3), INPUT_PULLUP"); &__c[0];}))))));
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>((__extension__({static const char __c[] __attribute__((section(".irom.text"))) = ("Begining of Setup"); &__c[0];}))))));
  }

  while ((WiFi.status() != WL_CONNECTED)) {
    WifiConnexionManager();//Recherche reseau wifi + connexion
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print(".");
      delay(1000);
      Serial.print(".");
      delay(1000);
      Serial.print(".\r");
      delay(1000);
      Serial.println("   \r");
    }
  }//Loop connexion until Wifi Connetion is OK

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  }

  udp.begin(localPort);// Start UDP port for connexion to NTP server

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Starting UDP");
    Serial.print("Local port: ");
    Serial.println(udp.localPort());
  }

  GetTimeByUDP();
  UpdateTime();
  mqttClient.setServer(server, 1883);// Set the MQTT broker details

  //wifi_status_led_uninstall();

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("End of Setup");
  }
}// fin set up
//******************end of setup*************************************
//*******************************************************************
//******************start of loop************************************
void loop() {

  //LoopLog
  if(SERIAL_PORT_LOG_ENABLE){
  Serial.print(".\r");
  delay(100);
  Serial.print("0\r\n");
  }

  WifiConnectOwner((char*)ssid1,(char*)password1);
  UpdateTime();
  rssi = WiFi.RSSI();
  if (Year < 2016) { // si l'heure n'est pas configuree C est a dire si l'annee n'est pas bonne, alors on recupere l'heure sur le serveur NTP
    GetTimeByUDP();
    UpdateTime();
  }

  if (WiFi.status() != WL_CONNECTED) {// si la connexion WIFI est perdue, alors on relance le setup
    WiFi.disconnect();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Connexion lost ");
    }
    setup();
  }

  Read_Button(IlsPorte, Verrou); // lecture du statut des contacts

  // si un changement de status est detecte, on essaye de mettre a jour le statut thingspeak
  if (Update_needed == true) {
    mqttpublishtry();
  }

  if (Status_Verrou != Old_Status_Verrou )
  {
    Status_Verrou_Change = true;
    Old_Status_Verrou = Status_Verrou;
    Cpt_Verrou = 0;
    while ((Cpt_Verrou <= 120) && (Status_IlsPorte == true)) { //??? Status_IlsPorte == true ou False ?????
      delay(1000);
      Read_Button(IlsPorte, Verrou);
      Cpt_Verrou = Cpt_Verrou + 1;
    }
  }
  else {
    Status_Verrou_Change = false;
  }


  if (Status_IlsPorte == false && DoorClosedBack == false) { //Detecte la refermeture de la porte
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Door closed back");
      Serial.println("1 minute d'attente avant de continuer le traitement");
    }
    DoorClosedBack = true;
    MailContent = String("Fermeture porte \r\n");
    UpdateTime();
    delay(60000);// wait for 1 minute before sending mail
    Read_Button(IlsPorte, Verrou);
    if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) {
      Old_Status_Verrou = Status_Verrou;
      Status_Verrou_Change = false;
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email sent");
      }
    }
    else
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email failed");
      }
    }
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Ready for new acquisition");
    }
  }

  if ((Status_IlsPorte == true && DoorClosedBack == true )) //Detecte l'ouverture de la porte
  {
    DoorClosedBack = false;
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Door seen open");
      Serial.println("Debut traitement IlsPorte");
      Serial.println("1 minute d'attente avant de continuer le traitement");
    }
    MailContent = String("Ouverture porte \r\n");
    UpdateTime();
    delay(60000);// wait for 1 minute before sending mail
    Read_Button(IlsPorte, Verrou);
    if (Status_IlsPorte == false) {
      DoorClosedBack = true;
      MailContent = String("Ouverture et refermeture porte \r\n");
    }
    if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) {
      Old_Status_Verrou = Status_Verrou;
      Status_Verrou_Change = false;
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email sent");
      }
    }
    else
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email failed");
      }
    }
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Ready for new acquisition");
    }
  }// fin Status_IlsPorte==true


  if (Status_Verrou_Change == true) { // traitement du changement d'Ã©tat du verrou
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Debut traitement Verrou");
      Serial.println();
      Serial.println("Verrou status changed");
      Serial.println("Debut traitement ILS verrou");
    }
    MailContent = String("Le statut du verrou change \r\n");
    UpdateTime();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("mail content value=  ");
      Serial.println(MailContent);
    }
    delay(60000);// wait for 1 minute before sending mail
    Read_Button(IlsPorte, Verrou);
    if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) {
      Status_Verrou_Change = false;
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email sent");
      }
    }
    else
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email failed");
      }
    }
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Ready for new acquisition");
    }
  } //fin status_Verrou == true

  //debut daily mail

  if (Hour == 8) // tant qu'il est 8h locale, on re autorise le daily mail
  {
    DailyMailSent = false;
  }// fin rearm daily mail

  if ((Hour > 8) && (DailyMailSent == false)) // a partir de 9h00 locale, si le Daily mail n'a pas ete envoye, alors on l'envoie
  {
    sendDailyMail();
  }

  if (SERIAL_PORT_LOG_ENABLE) {
  Serial.print("millis() - lastConnectionTime = ");
  Serial.println(millis() - lastConnectionTime );
  }

  if (millis() - lastConnectionTime > RegularpostingInterval) {
    mqttpublishtry();
  }


if (millis() - lastConnectionTime > whatchDogValue) //whatch dog 
  {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Loop Whatch Dog activation -> relaunch Setup()");
    }
    setup();
  }


}//*****************END OF LOOP *******************

void UpdateTime() {
  StringDate = ""; // reset StringDate value
  StringTime = ""; // reset StringTime value
  Hour = hour();
  Minute = minute();
  Second = second();
  Year = year();
  Month = month();
  Day = day();
  WeekDay = weekday();

  if ((Month <= 3) || (Month > 10)) { // on est un mois d'hiver
    if ((Month == 3) && (Day - WeekDay > 24)) { //on est dans le dernier mois d'hiver et aprÃ¨s le dernier dimanche
      Hour = Hour + 2;
      StringTime += " UTC+2: ";
      if (Hour >= 22){
        Day = Day + 1;
      }
    }
    else { // toute la periode hivernale
      Hour = Hour + 1;
      StringTime += " UTC+1: ";
        if (Hour >= 23){
        Day = Day + 1;
      }
    }
  }
  else { // on est un mois d'aout
    if ((Month == 10) && (Day - WeekDay > 24)) { //on est dans le dernier mois d'aout et apres le dernier dimanche
      Hour = Hour + 1;
      StringTime += " UTC+1: ";
      if (Hour >= 23){
        Day = Day + 1;
      }
    }
    else { // toute la periode estivale
      Hour = Hour + 2;
      StringTime += " UTC+2: ";
      if (Hour >= 22){
        Day = Day + 1;
      }
    }
  }

  //StringDate += "Date: ";
  if (Day < 10) {
    StringDate += "0";
  }
  StringDate += Day;
  if (Month < 10) {
    StringDate += "/0";
  }
  else {
    StringDate += "/";
  }
  StringDate += Month;
  StringDate += "/";
  StringDate += Year;

  if (Hour < 10) {
    StringTime += "0";
  }
  StringTime += Hour;
  if (Minute < 10) {
    StringTime += ":0";
  }
  else {
    StringTime += ":";
  }
  StringTime += Minute;
  if (Second < 10) {
    StringTime += ":0";
  }
  else {
    StringTime += ":";
  }
  StringTime += Second;
  if (SERIAL_PORT_LOG_ENABLE) {
    //Serial.print("valeure de StringDate ");
    //Serial.println(StringDate);
    //Serial.print("valeure de StringTime ");
    //Serial.println(StringTime);
  }
}//End of Update Time


void GetTimeByUDP() {
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);


  int cb = udp.parsePacket();

  if (!cb) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("no packet yet");
    }
  }
  else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("packet received, length=");
      Serial.println(cb);
    }

    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = makeWord(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = makeWord(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);
    }

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    epoch = secsSince1900 - seventyYears;
    setTime(epoch);

  }
}

void Read_Button(boolean IlsPorte, boolean Verrou)
{
  int DebounseCpt = 0;
  Old_Status_IlsPorte = Status_IlsPorte;
  Old_Status_Verrou = Status_Verrou;
  int Status_IlsPorte_Cpt = 0;
  int Status_Verrou_Cpt = 0;
  int Threshold = 4;
  while (DebounseCpt <= MaxDebounse) {
    DebounseCpt = DebounseCpt + 1;
    if (digitalRead(IlsPorte) == 0x0) Status_IlsPorte_Cpt = Status_IlsPorte_Cpt + 1; // ATTENTION - pour application reel : LOW (porte ouverte -> ILS ouvert -> LOW level via Ext pulldown)
    if (digitalRead(Verrou) == 0x1) Status_Verrou_Cpt = Status_Verrou_Cpt + 1; // ATTENTION - pour application reel : HIGH
    delay(100);
  }

  if (Status_IlsPorte_Cpt > Threshold) Status_IlsPorte = true;
  else Status_IlsPorte = false;
  if (Status_Verrou_Cpt > Threshold ) Status_Verrou = true;
  else Status_Verrou = false;
  if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("MAX debounse = ");
      Serial.println(MaxDebounse);
      Serial.print("Threshold = ");
      Serial.println(Threshold);
      Serial.print("Status_IlsPorte_Cpt = ");
      Serial.println(Status_IlsPorte_Cpt);
      Serial.print("Status_Verrou_Cpt = ");
      Serial.println(Status_Verrou_Cpt);

    }
  if (!((Status_IlsPorte == Old_Status_IlsPorte) && (Status_Verrou == Old_Status_Verrou)))
  {
    Update_needed = true;
  }
}

// send an NTP request to the time server at the given address
//unsigned long sendNTPpacket(IPAddress& address)
void sendNTPpacket(IPAddress& address)
{
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("sending NTP packet...");
  }
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0; // Stratum, or type of clock
  packetBuffer[2] = 6; // Polling Interval
  packetBuffer[3] = 0xEC; // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void sendDailyMail() {
    GetTimeByUDP();
    Read_Button(IlsPorte, Verrou);
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println();
      Serial.println("Daily mail");
    }
    MailContent = String("Daily report: rue de Vaugirard \r\n");
    if (sendEmail(MailFrom, MailTo, MailContent, ThingspeakChannelAdress, ThingspeakWriteAPIKey)) { // Mail sent success
      DailyMailSent = true;
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email sent");
      }
    }
    else // Mail sent failure
    {
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("Email not sent - System frozen for 3 minutes");
      }
      delay(180000); // on bloque l'envoie du Daily mai pour 3 minutes avant prochain envoie
    }
  }//FIN sendDailyMail

byte sendEmail(String FcMailFrom, String FcMailTo, String FcMailContent, String FcThingspeakChannelAdress, String FcThingspeakChannelWriteAPIKey)
{
  if (!AvecMail) return 0; // sort de la procedure sans envoyer le mail si Avec mail=0

  if (client.connect(server_email, 25)) {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("connected");
    }
  } else {
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("connection failed");
    }
    return 0;
  }

  if (!eRcv()) return 0;

  // change to your public ip
  client.println("helo 10.62.66.144");

  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending From");
  }

  // change to your email address (sender)
  client.print("MAIL From: ");
  client.println(FcMailFrom);// contient l'adresse email de l'expediteur "<toto@titi.fr>"

  if (!eRcv()) return 0;

  // change to recipient address
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending To");
  }
  client.print("RCPT To: ");
  client.println(FcMailTo);// contient l'adresse email du destinataire "<toto@titi.fr>"
  if (!eRcv()) return 0;

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending DATA");
  }

  client.println("DATA");
  if (!eRcv()) return 0;


  //*****************Creation du Mail***********************
  client.print("To: You "); // Destinataire
  client.println(FcMailTo); // Destinataire
  client.print("From: Me "); // My address
  client.println(FcMailFrom); // My address
  client.println("Subject: Arduino Report from Maison Paris\r\n"); //sujet du mail
  //Corps du mail
  client.print(FcMailContent); // message specifique de l'application appelante

  if ((Status_IlsPorte == false) && (Status_Verrou == false)) {
    client.println("La porte d'entree est : fermee et verouillee\r\n");
  }
  if ((Status_IlsPorte == false) && (Status_Verrou == true)) {
    client.println("La porte d'entree est : fermee et non verouillee\r\n");
  }
  if ((Status_IlsPorte == true) && (Status_Verrou == false)) {
    client.println("La porte d'entree est : ouverte et verouillee\r\n");
  }
  if ((Status_IlsPorte == true) && (Status_Verrou == true)) {
    client.println("La porte d'entree est : ouverte et non verouillee\r\n");
  }

  client.print(StringDate);
  client.println(StringTime);

  client.println(FcThingspeakChannelAdress);

  client.print("http:///");
  client.println(WiFi.localIP());

  client.print("SW version: "); // Date de compilation
  client.print(Release_Date);
  client.println("   ;   21-Wifi-porte thingspeaks - bis"); // chemin

  //fin du mail
  client.println(".");
  if (!eRcv()) return 0; //test si mail correctement envoye

  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending email");
  }

  client.println("QUIT"); //Deconnection du server mail
  if (!eRcv()) return 0;
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("Sending QUIT");
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
  return 1;
  UpdateTime();
}

byte eRcv(){
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println("QUIT");

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.println("\r\nTimeout");
      }
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    if (SERIAL_PORT_LOG_ENABLE) {
      Serial.write(thisByte);
    }
  }
  client.stop();
  if (SERIAL_PORT_LOG_ENABLE) {
    Serial.println("disconnected");
  }
}

void reconnect()
{
  int connect_counter = 20;
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Connect to the MQTT broker
    if (mqttClient.connect(ThingspeakClientID, ThingspeakUserID, ThingspeakUserPwd))
    {
      Serial.println("connected");
    } else
    {
      connect_counter = connect_counter - 1;
      if ((mqttClient.state() == -3) or (connect_counter < 0))
      {
        Serial.println("-3 : MQTT_CONNECTION_LOST - the network connection was broken ... launch setup");
        setup();
      }
      Serial.print("failed, rc=");
      // Print to know why the connection failed
      // See http://pubsubclient.knolleary.net/api.html#state for the failure code and its reason
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying to connect again
      delay(5000);
    }
  }
}

void mqttpublish() {
  String data = String("field1=" + String(Status_Verrou, 10) + "&field2=" + String(Status_IlsPorte, 10) + "&field3=" + String(rssi, 10));
  // Get the data string length
  int length = data.length();
  char msgBuffer[length];
  char* PublishCmd = "channels/9245/publish/SSUT4K9H6PVUIRNT";
  // Convert data string to character buffer
  data.toCharArray(msgBuffer, length + 1);
  Serial.println(msgBuffer);
  // Publish data to ThingSpeak. Replace <YOUR-CHANNEL-ID> with your channel ID and <YOUR-CHANNEL-WRITEAPIKEY> with your write API key
  if(mqttClient.publish(PublishCmd, msgBuffer)){
    // note the last connection time
    lastConnectionTime = millis();
  }
}

void mqttpublishtry() {
  if (!mqttClient.connected())
  {
    reconnect();
  }
  // Call the loop continuously to establish connection to the server
  mqttClient.loop();
  // If interval time has passed since the last connection, Publish data to ThingSpeak
  if (millis() - lastConnectionTime > postingInterval)
  {
    mqttpublish();
    Update_needed = false;
  }
    if (SERIAL_PORT_LOG_ENABLE) {
    Serial.print("millis() - lastConnectionTime = ");
    Serial.println(millis() - lastConnectionTime );
    }
}

void WifiConnexionManager() {
  if (SERIAL_PORT_LOG_ENABLE) {
  Serial.println("Begin of WifiConnexionManager");
  }
  int numberOfNetworks = WiFi.scanNetworks();

    if (SERIAL_PORT_LOG_ENABLE) {
      for (int i = 0; i < numberOfNetworks; i++) {
          Serial.print(WiFi.RSSI(i));
          Serial.print("dBm  ");
          Serial.print(" ");
          Serial.print("Network name: ");
          Serial.println(WiFi.SSID(i));
      }
    }
for (int i = 0; i < numberOfNetworks; i++) {
    if (WiFi.SSID(i) == ssid1) {
      WiFi.begin(ssid1, password1);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid1);
      }
      break;
    }

    if (WiFi.SSID(i) == ssid2) {
      WiFi.begin(ssid2, password2);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid2);
      }
      break;
    }

    if (WiFi.SSID(i) == ssid0) {
      WiFi.begin(ssid0, password0);
      CurentSSID = WiFi.SSID(i);
      WaitConnexion();
      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi trouvé. temtative de connexion a: ");
        Serial.println(ssid0);
        Serial.print("Wifi.status(): ");
        Serial.println(WiFi.status());
      }
      break;
    }
  }//End FOR
  Serial.println("End of WifiConnexionManager");
}//end WifiConnexionManager


void WifiConnectOwner(char* SSIDowner_fct, char* passwordowner_fct) {
  if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Begin of WifiConnectOwner");
      Serial.println("Already connected to :");
      Serial.println(WiFi.SSID());
  }
  int numberOfNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numberOfNetworks; i++) {
    if ((WiFi.SSID(i) == SSIDowner_fct) && ((String)SSIDowner_fct != CurentSSID)) {
      WiFi.disconnect();
      WiFi.begin(SSIDowner_fct, passwordowner_fct);

      if (SERIAL_PORT_LOG_ENABLE) {
        Serial.print("Wifi Owner trouve . temtative de connexion a: ");
        Serial.println(SSIDowner_fct);
      }
      WaitConnexion();
      if ((WiFi.status() == WL_CONNECTED))
      {
        CurentSSID = WiFi.SSID(i);
        if (SERIAL_PORT_LOG_ENABLE) {
          Serial.print("connecte a: ");
          Serial.println(SSIDowner_fct);
        }

      }
      break;
    }
  }//End FOR
Serial.println("End of WifiConnectOwner");
}//end WifiConnectOwner

void WaitConnexion(){
      int cpt=10;
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("Begin of WaitConnexion");
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      while((cpt > 0) && ((WiFi.status() != WL_CONNECTED))){
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      cpt = cpt -1;
      delay(1000);
      }
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.print("cpt= ");
      Serial.print(cpt);
      Serial.print("   WiFi.status= ");
      Serial.print(WiFi.status());
      Serial.print("   WL_CONNECTED= ");
      Serial.println(WL_CONNECTED);
      Serial.print("test = (cpt >= 0) && ((WiFi.status() != WL_CONNECTED)) =  ");
      Serial.println((cpt > 0) && ((WiFi.status() != WL_CONNECTED)));
      }
      if ((WiFi.status()== WL_CONNECTED))
      {
          if (SERIAL_PORT_LOG_ENABLE) {
             Serial.print("connecte a: ");
             Serial.println(WiFi.SSID());
            }
      }
      if (SERIAL_PORT_LOG_ENABLE) {
      Serial.println("End of WaitConnexion");
      }
}// end Waitconnexion
