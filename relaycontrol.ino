#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define datapin 2
#define clockpin 3
#define latchpin 4
int current = 0;		// 00000000
const int relayValues[]={
  1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
int relayCheck[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int changeVal = 0;
//char OnOrOff[] = {"off","on"};
//char DataResult[100]="Publishing data - relay "; //24
String OnOrOff;
String DataMash;
String DataResult;
byte mac[] = {  
  0xDE, 0xC0, 0xDE, 0xC0, 0xFF, 0xEE };
byte mqttserver[] = { 
  192, 168, 2, 7 };
byte ip[] = { 
  192, 168, 2, 101 };

//char DataMash[30]="                             ";

EthernetClient ethClient;
PubSubClient clientr(mqttserver, 1883, callback);

/////////////////// calculateRelayVal ///////////////////
int calculateRelayVal(int newVal) {

  // if newVal==0 then return value as 11111111
  // newVal has relay we need to toggle
  // referenced in relayCheck as newVal-1
  int intRelayToWorkWith = relayValues[(newVal-1)];

  if(relayCheck[(newVal-1)]==0) {
    // relay is currently off, turn on
    changeVal = current | intRelayToWorkWith;
    relayCheck[(newVal-1)]=1;
  } 
  else {
    // relay is currently on, turn off
    changeVal = current ^ intRelayToWorkWith;
    relayCheck[(newVal-1)]=0;
  }

  current = changeVal;
  return current;
}

/////////////////// doWork ///////////////////
void doWork(int val) {

  // as relays need a '0' to turn on and a '1' to turn off, need to NOT this value
  unsigned int realVal = ~val;

  uint8_t rVlow = realVal & 0xff;
  uint8_t rVhigh = (realVal >> 8);

  digitalWrite(latchpin,LOW);
  shiftOut(datapin,clockpin,MSBFIRST,rVhigh);
  shiftOut(datapin,clockpin,MSBFIRST,rVlow);
  digitalWrite(latchpin,HIGH);

}

char* statusCheck() {
  char ReturnVar[40];
  String strGetStatus="";
  for(int i=0;i<16;i++) {
    strGetStatus+=relayCheck[i];
  } 
  strGetStatus.toCharArray(ReturnVar,40);
  return ReturnVar;
}

/////////////////// callback ///////////////////
static void callback(char* topic, byte* payload, unsigned int length) {
  char PublishData[40];
  char tina[4];
  String tp = (char*)payload;
  (tp.substring(0,length)).toCharArray(tina,4);
  unsigned int readVar = (unsigned int)atoi(tina);

  if(readVar<0 || (readVar>16 && (readVar!=999 && readVar != 998))) {
    // if input is outside of specification (greater than 16 and not 999 (status check) or less than zero then make -1
    // -1 is used in the switch below to indicate invalid input
    readVar=-1; 
  }
  switch(readVar) {
  case 999:  // Status
    {
      /*
      String strGetStatus="S:";
       for(int i=0;i<16;i++){
       strGetStatus+=relayCheck[i];
       }
       strGetStatus.toCharArray(PublishData,24);
       //    clientr.publish("RelayControlReport",PublishData);
       clientr.publish("WebControl",PublishData);
       */
      clientr.publish("WebControl",statusCheck());
    } 
    break;
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
    {
      if(readVar>=0 && readVar<=16) {
        int registerValue=0;
        if(readVar > 0) { 
          registerValue = calculateRelayVal(readVar); 
        } 
        else {
          current=0;
          for(int i=0;i<16;i++){
            relayCheck[i]=0;
          }
        }
        doWork(registerValue);

        /*
        if(relayCheck[(readVar-1)]==1){
         OnOrOff=" on";
         } 
         else {
         OnOrOff=" off"; 
         }
         
         if(readVar!=0){
         DataMash="Relay: ";
         DataResult = DataMash + readVar + OnOrOff;
         DataResult.toCharArray(PublishData,24); 
         } 
         else {
         DataResult="All relays off";
         DataResult.toCharArray(PublishData,24);
         }
         clientr.publish("WebControl",PublishData);
         */
        clientr.publish("WebControl",statusCheck());
      }
    }
    break;
  case 998:  // All On
    {
      for(int i=0;i<16;i++) {
        relayCheck[i]=1;
      }
      doWork(-1);
    }
    clientr.publish("WebControl",statusCheck());
    break;
  default:
    break;
  }
}

/////////////////// setup ///////////////////
void setup()
{
  pinMode(datapin,OUTPUT);
  pinMode(clockpin,OUTPUT);
  pinMode(latchpin,OUTPUT);

  digitalWrite(latchpin,LOW);
  shiftOut(datapin,clockpin,MSBFIRST,-1);
  shiftOut(datapin,clockpin,MSBFIRST,-1);
  digitalWrite(latchpin,HIGH);

  delay(500);
  Serial.begin(9600);
  Serial.println(F("Scouris - MQTT Control, booting up."));
  Ethernet.begin(mac,ip);

  // below is for when running with DHCP
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(F(".")); 
  }
  Serial.println("");

  if(clientr.connect("arduinoClient")) {
    clientr.publish("checkInTopic","Relay Controller Online");
    //    clientr.publish("RelayControlReport","Relay Controller Online");
    clientr.publish("WebControl","Controller Online");
    clientr.subscribe("RelayControl"); 
  }
}

/////////////////// loop ///////////////////
void loop()
{
  clientr.loop();
}

