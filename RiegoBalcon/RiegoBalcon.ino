#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <rom/rtc.h>
#include <M5ez.h>
uint8_t Events[MAX_EVENTS];

// RGBConverter
#include "RGBConverter.h"
RGBConverter converter=RGBConverter();

// SCALE
#include <HX711_ADC.h>
const int HX711_dout = 15; //mcu > HX711 dout pin
const int HX711_sck = 16; //mcu > HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck);
long vessel_max=40,vessel_abs=0,vessel_tare=0,vessel_percent=0,vessel_liters=0;

// AUTOCONNECT
#undef min
#undef max
#include <WebServer.h>
#include <AutoConnect.h>
typedef WebServer WEBServer;
AutoConnect portal;
AutoConnectConfig config; 

// PREFERENCES
#include <Preferences.h>
Preferences preferences;
String Irrigation;
StaticJsonDocument<4096> Irrigation_json;

// MQTT VARIABLES
#include <MQTT.h>
WiFiClient net;
MQTTClient client(2048);
char MQTT_server[]="homeassistant";
uint16_t MQTT_port=1883;

// FLEXY STEPPER
#include "FlexyStepper.h"

// MOTORS
#define PUMP1 13
#define PUMP2 5

// SCALE
//#include "HX711.h"
//HX711 scale(15, 16); 

// NEOPIXEL HEADERS AND DEFINES
#define NEOPIXEL_PIN   12
#define NUMPIXELS      2
#define FASTLED_INTERNAL
#include <FastLED.h>
CRGB leds[NUMPIXELS];

/*********************************/
// PRINT RESET REASON
/*********************************/
void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1, Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3, Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5, Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

/*********************************/
// SETUP
/*********************************/
void setup() 
  {
  // initialize the M5Stack object
  M5.begin();
  
  Serial.begin(115200);
  
  // PRINT CPU 0 RESET REASON
  Serial.println();
  Serial.println("CPU0 reset reason: ");
  print_reset_reason(rtc_get_reset_reason(0));

  // PRINT CPU 1 RESET REASON
  Serial.println("CPU1 reset reason: ");
  print_reset_reason(rtc_get_reset_reason(1));
 
  // PUMPS PINS OUTPUT
  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);

  // STOP PUMPS
  digitalWrite(PUMP1, LOW);
  digitalWrite(PUMP2, LOW);

  // SCALE CALIBRATED
  //scale.set_scale(10057.7597);
  LoadCell.start(2000, false); //Stabilization time 2000, no tare 
  LoadCell.setGain(64);
  LoadCell.setCalFactor(4986.738925); // set calibration value (float)
  // FILL DATASET AFTER RESET
  LoadCell.refreshDataSet(); 
  
  // BEGIN NEOPIXEL
  FastLED.addLeds<WS2812, NEOPIXEL_PIN, RGB>(leds, NUMPIXELS);

  // M5ez default prefs
  Preferences prefs;
  prefs.begin("M5ez");
  prefs.putBool("clock_on", true);
  prefs.putString("timezone", "Europe/Madrid");
  prefs.putBool("clock12", false);
  prefs.putUChar("inactivity", true);
  prefs.end();

  // Start m5ez
  ez.begin();
  
  preferences.begin("irrigation",true);
  Irrigation=preferences.getString("schedules", "[]");
  deserializeJson(Irrigation_json,Irrigation);
  vessel_tare=preferences.getInt("vessel_tare", vessel_tare);
  vessel_max=preferences.getInt("vessel_max", vessel_max);
  Serial.println("\nschedules: "+Irrigation);
  Serial.println("vessel_tare: "+String(vessel_tare));  
  Serial.println("vessel_max: "+String(vessel_max));
  preferences.end();

  //webServer.on("/", onRoot);  // Register the root page redirector.

  // SCREEN TASK CREATION.
  xTaskCreatePinnedToCore(
                          TaskSCREEN
                          ,  "TaskSCREEN"   // A name just for humans
                          ,  16000  // This stack size can be checked & adjusted by reading the Stack Highwater
                          ,  NULL
                          ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                          ,  NULL 
                          ,  1);
                          
  // MQTT TASK CREATION.
  xTaskCreatePinnedToCore(
                          TaskMQTT
                          ,  "TaskSCREEN"   // A name just for humans
                          ,  16000  // This stack size can be checked & adjusted by reading the Stack Highwater
                          ,  NULL
                          ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                          ,  NULL 
                          ,  1);
                          
   // VESSEL TASK CREATION.
  xTaskCreatePinnedToCore(
                          TaskVESSEL
                          ,  "TaskSCREEN"   // A name just for humans
                          ,  16000  // This stack size can be checked & adjusted by reading the Stack Highwater
                          ,  NULL
                          ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                          ,  NULL 
                          ,  1);   
  // TIME EVENT TASK CREATION.
  xTaskCreatePinnedToCore(
                          TaskEVENT
                          ,  "TaskEVENT"   // A name just for humans
                          ,  16000  // This stack size can be checked & adjusted by reading the Stack Highwater
                          ,  NULL
                          ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                          ,  NULL 
                          ,  1);                       
  Serial.println("SETUP END...");
  }

/*********************************/
// Main program loop
/*********************************/
uint32_t lastUpdate = 0;
uint32_t overFlowCounter = 0;
uint64_t LNow=0;
uint64_t STOPPUMP1=0;
uint64_t STOPPUMP2=0;
void loop() 
  {
  // LARGE MILLIS COUNTER
  uint32_t now = millis();

  // EVERY TIME MILLIS 32 COUNTER OVERFLOW
  if (now < lastUpdate) 
    {
    overFlowCounter++;
    lastUpdate = now;
    Serial.print("lastupdate = ");
    Serial.println(lastUpdate);
    Serial.print("overFlowCounter = ");
    Serial.println(overFlowCounter);
    }
    
  // STORE LARGE MILLIS COUNTER
  LNow=((uint64_t)overFlowCounter * 4294967296) + (uint64_t)now; 

  // IF TIME TO STOP PUMP HAS PASED
  if (STOPPUMP1<LNow) digitalWrite(PUMP1, LOW);
  if (STOPPUMP2<LNow) digitalWrite(PUMP2, LOW);
  }

/***********************************************************/
//         THREAD TASK TO MANAGE SCREEN MENUS
/***********************************************************/
void TaskSCREEN( void *pvParameters ) 
  {  
  // PRINT THREAD STARTUP
  Serial.println("TaskSCREEN STARTED");  
  
  for (;;) 
    {    
    ez.msgBox("M5ez minimal program", "Hello World !", "Settings",false);
    ez.settings.menu();  
    delay(10);
    }
  }

/***********************************************************/
//         THREAD TASK TO MANAGE MQTT CLIENT
/***********************************************************/
String HomeAssitantMQTTBase="homeassistant";
boolean PUMP1_move_pre=0,PUMP2_move_pre=0;
float vessel_tare_pre=0,vessel_percent_pre=0,vessel_liters_pre=0;
void TaskMQTT( void *pvParameters ) 
  {  
  char vessel_percent_array[10];
  char vessel_liters_array[10];
  StaticJsonDocument<2048> publish_sensors;
  bool modifiedjson=1;
  publish_sensors["pump1"]="Off";
  publish_sensors["pump2"]="Off";
  publish_sensors["vessel_liters"]=0;
  publish_sensors["vessel_percent"]=0;
  
  // AUTOCONET PARAMETERS
  config.autoReconnect = true;
  config.apid = "RIEGO";
  config.psk="Queralt2016";
  portal.config(config);
  
  // PORTAL BEGIN
  portal.begin();
  WEBServer& webServer = portal.host();
  


  // PRINT THREAD STARTUP
  Serial.println("TaskMQTT STARTED"); 
  
  for (;;) 
    {    
    // TASK DELAY
    delay(1);
    
    // autoconnect
    portal.handleClient();

    // RECONNECT MQTT
    if (!client.connected()) connect();

    // CHECK IF PUMPS ARE MOVING
    boolean PUMP1_move=digitalRead(PUMP1);
    boolean PUMP2_move=digitalRead(PUMP2);

    // PUBLISH PUMPS MOVEMENT ON CHANGE
    if (PUMP1_move==1 && PUMP1_move_pre==0) {publish_sensors["pump1"]="On";modifiedjson=1;}
    if (PUMP1_move==0 && PUMP1_move_pre==1) {publish_sensors["pump1"]="Off";modifiedjson=1;}
    if (PUMP2_move==1 && PUMP2_move_pre==0) {publish_sensors["pump2"]="On";modifiedjson=1;}
    if (PUMP2_move==0 && PUMP2_move_pre==1) {publish_sensors["pump2"]="Off";modifiedjson=1;}

    // STORE PREVIOUS PUMPS VALUES
    PUMP1_move_pre=PUMP1_move;
    PUMP2_move_pre=PUMP2_move;

    // PUBLISH VESSEL DATA ON CHANGE
    if (vessel_percent!=vessel_percent_pre) {publish_sensors["vessel_percent"]=vessel_percent;modifiedjson=1;}
    if (vessel_liters!=vessel_liters_pre) {publish_sensors["vessel_liters"]=(((float)vessel_liters)/10);modifiedjson=1;}
    
    // STORE PREVIOUS SCALE VALUES
    vessel_tare_pre=vessel_tare;
    vessel_percent_pre=vessel_percent;
    vessel_liters_pre=vessel_liters;

    // IF THERE ARE DATA TO PUBLISH
    if (modifiedjson)
      {
      String payload;
      String topic=HomeAssitantMQTTBase+"/sensor/irrigation/state";
      payload="";
      serializeJson(publish_sensors, payload);
      payload=String(payload); 
       
      // PUBLISH STATUS MESSAGE
      client.publish(topic, payload,true,1);

      // RESET DATA TO SEND SEMAPHORE
      modifiedjson=0;      
      }
    
    // ATTEND SUBSCRIVE EVENTS
    client.loop();
    }
  }

 /***********************************************************/
//                    CONNECT
/***********************************************************/
void connect() 
  {
  //START MDNS RESPONDER
  if (!MDNS.begin("riegobalcon")) {Serial.println("Error setting up MDNS responder!");while(1) {delay(1000);}}
  Serial.println("mDNS responder started.");

  // Query Server IP
  IPAddress MQTT_server_IP(158,42,144,151), BadIP(0,0,0,0);
  Serial.println("\nQuery SERVER IP:");
  do {MQTT_server_IP = MDNS.queryHost(MQTT_server,2000);Serial.println(MQTT_server_IP.toString());delay(1000);}while (MQTT_server_IP==BadIP);

  // Setup MQTT Conection
  char IP[] = "xxx.xxx.xxx.xxx";  
  MQTT_server_IP.toString().toCharArray(IP, 16);
  client.begin(IP, MQTT_port, net);
  client.onMessage(messageReceived);

  // Stablish connection
  Serial.print("connecting...");
  while (!client.connect("RiegoBalcon", "mqtt", "Mqtt2020")) {Serial.print(".");delay(1000);}
  Serial.println("\nconnected!");

  // Call auto discover
  auto_discover();
  }

/***********************************************************/
//                    NEW MESSAGE
/***********************************************************/
String SchedulesTopic="irrigation/esclavas";
String PumpsTopic=HomeAssitantMQTTBase+"/switch/pump%d_%d/set";
String VesselTareTopic=HomeAssitantMQTTBase+"/switch/vesseltare/set";
String UptimeTopic="$SYS/broker/uptime";
String VesselTareNumberTopic="irrigation/esclavas/tare";
String VesselMaxNumberTopic="irrigation/esclavas/max";
void messageReceived(String &topic, String &payload) 
  {
  // JSON Object
  StaticJsonDocument<2048> set_message;

  int p=0,t=0;
  if (sscanf(topic.c_str(),PumpsTopic.c_str(),&p,&t)==2)
    {
    if(payload=="ON")
      {
      Serial.println("PUMP"+String(p)+" "+String(t)+"min");
      if (p==1)
        {
        // MOVE PUMP 1
        if (t==0) {STOPPUMP1=0;digitalWrite(PUMP1, LOW);}
        else {STOPPUMP1=LNow+(60000*t);digitalWrite(PUMP1, HIGH);}//PUMP1.setTargetPositionRelativeInRevolutions(-500*t); // 500 = 450 ml
        }
      if (p==2)
        {
        // MOVE PUMP 2
        if (t==0) {STOPPUMP2=0;digitalWrite(PUMP2, LOW);}
        else {STOPPUMP2=LNow+(60000*t);digitalWrite(PUMP2, HIGH);}//PUMP2.setTargetPositionRelativeInRevolutions(-500*t);
        }
      }
    
    }

  // NEW SCHEDULES
  if (topic==SchedulesTopic)
    {
    preferences.begin("irrigation",false);
    preferences.putString("schedules", payload);
    Irrigation=payload;
    deserializeJson(Irrigation_json,Irrigation);
    preferences.end();
    }

  // TARE SCALE
  if (topic==VesselTareTopic && payload=="ON") 
    {
    // SET NEW TARE
    vessel_tare=vessel_abs;
    
    // STORE TARE IN PREFERENCES
    preferences.begin("irrigation",false);
    preferences.putInt("vessel_tare", vessel_tare);
    preferences.end();

    // PUBLISH NEW TARE
    client.publish(VesselTareNumberTopic, String(vessel_tare),true,1);
    }

  // GET TARE IN NUMBERS
  if (topic==VesselTareNumberTopic) 
    {
    // SET NEW TARE
    vessel_tare=payload.toInt();
    
    // STORE TARE IN PREFERENCES
    preferences.begin("irrigation",false);
    preferences.putInt("vessel_tare", vessel_tare);
    preferences.end();   
    }

  // GET MAX IN NUMBERS
  if (topic==VesselMaxNumberTopic) 
    {
    // SET NEW TARE
    vessel_max=payload.toInt();
    
    // STORE TARE IN PREFERENCES
    preferences.begin("irrigation",false);
    preferences.putInt("vessel_max", vessel_max);
    preferences.end();   
    }

  if (topic!=UptimeTopic) Serial.println("incoming: " + topic + " - " + payload); 
  }

/***********************************************************/
//                    AUTO DISCOVER
/***********************************************************/
void auto_discover()
  {
  // JSON Object
  StaticJsonDocument<2048> discover;
  discover.createNestedObject("device");
   
   // Device parameters
  discover["device"]["manufacturer"]="Charlie";
  discover["device"]["model"]="Custom Irrigation";
  
  // Discover common parameters
  discover["icon"]="mdi:sprinkler-variant";
  discover["stat_t"]=HomeAssitantMQTTBase+"/sensor/irrigation/state";

  //////////////////////
  // PUMP1 Parameters
  //////////////////////
  discover["name"]="Bomba 1";
  discover["unique_id"]="Bomba1";
  discover["device"]["identifiers"]="Bomba1_pump";
  discover["value_template"]="{{ value_json.pump1}}";
  
  String payload;
  String topic=HomeAssitantMQTTBase+"/sensor/irrigationP1/config";
  payload="";
  serializeJson(discover, payload); 
  Serial.println("config: " + topic + " - " + payload);

  // Publish config message
  client.publish(topic, payload,true,1);

  //////////////////////
  // PUMP2 Parameters
  //////////////////////
  discover["name"]="Bomba 2";
  discover["unique_id"]="Bomba2";
  discover["device"]["identifiers"]="Bomba2_pump";
  discover["value_template"]="{{ value_json.pump2}}";
  
  topic=HomeAssitantMQTTBase+"/sensor/irrigationP2/config";
  payload="";
  serializeJson(discover, payload); 
  Serial.println("config: " + topic + " - " + payload);

  // Publish config message
  client.publish(topic, payload,true,1);
  
  //////////////////////
  // VESSEL LITTERS
  //////////////////////
  discover["icon"]="mdi:scale";
  discover["name"]="Vasija Litros";
  discover["unique_id"]="Vessel_liters";  
  discover["unit_of_measurement"]="L";
  discover["device"]["identifiers"]="Vessel_liters_scales";
  discover["value_template"]="{{ value_json.vessel_liters}}";
  
  topic=HomeAssitantMQTTBase+"/sensor/irrigationL/config";
  payload="";
  serializeJson(discover, payload); 
  Serial.println("config: " + topic + " - " + payload);

  // Publish config message
  client.publish(topic, payload,true,1);

  //////////////////////
  // VESSEL %
  //////////////////////
  discover["icon"]="mdi:scale";
  discover["name"]="Vasija";
  discover["unique_id"]="Vessel_percent";
  discover["unit_of_measurement"]="%";
  discover["device"]["identifiers"]="Vessel_percent_scales";
  discover["value_template"]="{{ value_json.vessel_percent}}";
  
  topic=HomeAssitantMQTTBase+"/sensor/irrigationPERCENT/config";
  payload="";
  serializeJson(discover, payload); 
  Serial.println("config: " + topic + " - " + payload);

  // Publish config message
  client.publish(topic, payload,true,1);

  //////////////////////
  // SWITCH TARE
  //////////////////////
  discover["icon"]="mdi:scale-balance";
  discover["name"]="Tarar Vasija";
  discover["unique_id"]="Vessel_tare";
  discover["device"]["identifiers"]="Vessel_tare_switch";
  discover["~"]=HomeAssitantMQTTBase+"/switch/vesseltare";
  discover["cmd_t"]="~/set";
  discover["stat_t"]="~/state";
  discover.remove("unit_of_measurement");
  discover.remove("value_template");
  
  topic=HomeAssitantMQTTBase+"/switch/vesseltare/config";
  payload="";
  serializeJson(discover, payload); 
  Serial.println("config: " + topic + " - " + payload);
  
  // Subscribe to set messages
  client.subscribe(VesselTareTopic);
  // Publish config message
  client.publish(topic, payload,true,1);

  //////////////////////
  // PUMPS SWITCHES
  //////////////////////
  discover["icon"]="mdi:scale-balance";
  discover["cmd_t"]="~/set";
  discover["stat_t"]="~/state";
  for (int p=1;p<=2;p++)
  for (int m=0;m<=5;m++)
    {
    if (m==0) discover["name"]="Bomba"+String(p)+" STOP";
    else discover["name"]="Bomba"+String(p)+" "+String(m)+" min";
    discover["unique_id"]="Pump"+String(p)+"_"+String(m)+"min_switch";
    discover["device"]["identifiers"]="Pump"+String(p)+"_"+String(m)+"min_switch";
    discover["~"]=HomeAssitantMQTTBase+"/switch/pump"+String(p)+"_"+String(m);

    // Subscribe to set messages
    client.subscribe(HomeAssitantMQTTBase+"/switch/pump"+String(p)+"_"+String(m)+"/set");

    // Generate topic and pyload strins
    payload="";
    String topic=HomeAssitantMQTTBase+"/switch/pump"+String(p)+"_"+String(m)+"/config";
    serializeJson(discover, payload); 

    Serial.println("config: " + topic + " - " + payload);

    // Publish config message
    client.publish(topic, payload,true,1);
    delay(10);
    }

  // Subscribe to schedules messages
  client.subscribe(SchedulesTopic);
  client.subscribe(VesselTareNumberTopic);
  client.subscribe(VesselMaxNumberTopic);
  
  client.subscribe(UptimeTopic);
  }


/***********************************************************/
//         THREAD TASK TO MANAGE VESSEL LEVEL
/***********************************************************/
void TaskVESSEL( void *pvParameters ) 
  {  
  // PRINT THREAD STARTUP
  Serial.println("TaskVESSEL STARTED"); 
  
  for (;;) 
    {   
    // Get absolute measurement
    if (LoadCell.update()) {vessel_abs=round(LoadCell.getData()*10);}
    Serial.println("data: "+String(LoadCell.getData()));
    // Tare scale
    vessel_liters=vessel_abs-vessel_tare;
    if (vessel_liters<0) vessel_liters=0;

    //percent scale
    vessel_percent=(vessel_liters*100)/(vessel_max*10);
    if (vessel_percent>100) vessel_percent=100;

    // CONVERT WEIGTH RANGE TO COLOR
    byte RGB[3];
    float H=(((float)vessel_percent)*0.333)/100.0;
    if (H>0.333) H=0.333;
    H=0.333-H;
    converter.hsvToRgb(H, 1, 1, RGB);
    
    // SET LEDS COLOR
    leds[0] = CRGB( RGB[0], RGB[1], RGB[2]);
    leds[1] = CRGB( RGB[0], RGB[1], RGB[2]);

    // Show LEDS
    FastLED.show();
  
    delay(200);
    }
  }


/***********************************************************/
//         THREAD TASK TO MANAGE IRRIGATION
/***********************************************************/
void TaskEVENT( void *pvParameters ) 
  {  
  // FOR MINUTE CHANGE
  uint8_t minutos_pre=0;

  // PRINT THREAD STARTUP
  Serial.println("TaskVESSEL STARTED"); 
  
  for (;;) 
    {
    // GET NUMERIC DATETIME
    time_t ahora=ez.clock.tz.now();
    uint8_t segundos=second(ahora);
    uint8_t minutos=minute(ahora);
    uint8_t horas=hour(ahora);
    uint8_t dia=day(ahora);
    uint8_t mes=month(ahora);
    uint8_t diasemana=weekday(ahora);

    if (waitForSync(0))
    if (minutos!=minutos_pre)
      {
      //STORE PREVIOUR MINUTES
      minutos_pre=minutos;
      // CONVERTIR DE PRIMER DIA EL DOMINGO A PRIMER DIA EL LUNES
      if (diasemana==1) diasemana=7;
      else diasemana--;
  
      // CALCULAR LA ESTACIÓN DEL AÑO
      uint16_t diaano31=(mes*31)+dia;
      uint8_t estacion=4;
      if (diaano31>=(3*31)+20) estacion=1;
      if (diaano31>=(6*31)+21) estacion=2;
      if (diaano31>=(9*31)+22) estacion=3;
      if (diaano31>=(12*31)+21) estacion=4;

      // SCHEDULES BUCLE
      for (int s=0;s<Irrigation_json.size();s++)
        {
        int out=Irrigation_json[s]["OUT"].as<int>();
        int start_hora=Irrigation_json[s]["START_TIME"]["HOUR"].as<int>();
        int start_minutos=Irrigation_json[s]["START_TIME"]["MINUTES"].as<int>();
        int run_time=Irrigation_json[s]["MINUTES"].as<int>();
        bool run_estacion=Irrigation_json[s]["SEASONS"][String(estacion)].as<bool>();
        bool run_dia=Irrigation_json[s]["DAYS"][String(diasemana)].as<bool>();

        if (out>0)
        if (run_dia==1)
        if (run_estacion==1)
        if (start_hora==horas)
        if (start_minutos==minutos)
          {
          if (out==1) {STOPPUMP1=LNow+(60000*run_time);digitalWrite(PUMP1, HIGH);}
          if (out==2) {STOPPUMP2=LNow+(60000*run_time);digitalWrite(PUMP2, HIGH);}
          }
        
        //Serial.println(String(start_hora)+":"+String(start_minutos)+" "+String(horas)+":"+String(minutos));

        }
      
      
      //Serial.println(String(horas)+":"+String(minutos)+" Estación: "+String(estacion));
      }
      
    delay(500);
       
    } 
  }
