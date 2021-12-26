#include "Config.h"


// -- Callback method declarations.
void wifiConnected();
void configSaved();
boolean formValidator();

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];
char mqttTopicValue[STRING_LEN];
char checkIntervalValue[NUMBER_LEN];

boolean needReset = false;

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
IotWebConfParameter mqttServerParam = IotWebConfParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
IotWebConfParameter mqttUserNameParam = IotWebConfParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN);
IotWebConfParameter mqttUserPasswordParam = IotWebConfParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "password");
IotWebConfParameter mqttTopicParam = IotWebConfParameter("MQTT topic", "mqttTopic", mqttTopicValue, STRING_LEN, "text", NULL, "inverter/status");
IotWebConfParameter checkIntervalParam = IotWebConfParameter("Check inverval in sec", "checkInterval", checkIntervalValue, NUMBER_LEN, "number", "1..3600", "300", "min='1' max='3600' step='1'");

boolean isNeedReset(){
    return needReset;
}
char* getMqttServerValue(){
    return mqttServerValue;
}
char* getMqttUserNameValue(){
    return mqttUserNameValue;
}
char* getMqttUserPasswordValue(){
    return mqttUserPasswordValue;
}
char* getMqttTopicValue(){
    return mqttTopicValue;
}
int getCheckInterval(){
    return atoi(checkIntervalValue);
}

void configSetup(){
  iotWebConf.skipApStartup();
  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameter(&mqttServerParam);
  iotWebConf.addParameter(&mqttUserNameParam);
  iotWebConf.addParameter(&mqttUserPasswordParam);
  iotWebConf.addParameter(&mqttTopicParam);
  iotWebConf.addParameter(&checkIntervalParam);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  // iotWebConf.setupUpdateServer(&httpUpdater);

  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServerValue[0] = '\0';
    mqttUserNameValue[0] = '\0';
    mqttUserPasswordValue[0] = '\0';
    mqttTopicValue[0] = '\0';
    checkIntervalValue[0] = '\0';
  }

  server.on("/", [] { iotWebConf.handleConfig(); });
  server.onNotFound([]() { iotWebConf.handleNotFound(); });


}

void configLoop(){
  iotWebConf.doLoop();

  if (needReset){
#ifndef DEBUG_DISABLED
    Debug.println("Rebooting after 1 second.");
#endif
    iotWebConf.delay(1000);
    ESP.restart();
  }

}

boolean isOnline(){
   return iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE;
}

char* getThingName(){
   return iotWebConf.getThingName(); 
}

void configSaved()
{
#ifndef DEBUG_DISABLED
  Debug.println("Configuration was updated.");
#endif
  needReset = true;
}

boolean formValidator()
{
#ifndef DEBUG_DISABLED
  Debug.println("Validating form.");
#endif
  boolean valid = true;

  int l = server.arg(mqttServerParam.getId()).length();
  if (l < 3)
  {
    mqttServerParam.errorMessage = "Please provide at least 3 characters!";
    valid = false;
  }

  return valid;
}