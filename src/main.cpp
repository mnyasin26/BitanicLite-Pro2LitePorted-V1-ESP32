#include <Arduino.h>
#include <dev_config.h>
#include <header/connection.h>
#include <header/StorageHandler_EEPROM.h>
#include <header/processSchedule.h>
#include <header/displayLCDHandler.h>
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>
#include <SPI.h>
#include <OneButton.h>
#include <OverTheAir.h>

#include <ConfiguratorMKR.h>

// #define SET_LOOP_TASK_STACK_SIZE 4096

ConfiguratorMKR configurator(CONFIGURATOR_HOST, true);
OverTheAir ota;
int batas = 6;

StaticJsonDocument<JSON_SIZE> doc;
processSchedule processor;
Connection con;
StorageHandler_EEPROM storage;
extern TimeHandler time_h;
extern statusHandler status_h;
extern inputHandler input_h;
extern outputHandler ledPemupukan;
displayLCDHandler display_h;
Scheduler runner;

int counterForTime = 6;

unsigned long long lastSync = 0;
unsigned long long secondStamp = 0;
unsigned long long lastSecondStamp = 0;
int counterSecond = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;

bool stateConBefore = false;

OneButton button(0, true);

void Task1code(void *pvParameters);

void Task2code(void *pvParameters);
void checkMem();
void syncronize();
void syncLoop();

void restartWiFiManager();

int stateDisplay = 0;
unsigned long msCounter = 0;
int secondCounter = 0;

int cycle = 0;

void setup()
{

  Serial.begin(115200);
  delay(100);
  configurator.mountLittleFS();
  Serial.println("anjay");

  // this is only for the first time flash of the device after that it will be commented
  // abis di upload. comment baris nomor 53, terus upload lagi
  // configurator.writeSerialNumber("BT-LT-180924-V1.0-0001"); // BT01
  // configurator.getSerialNumber();

  configurator.loadSerialNumber();

  configurator.addVariable("id", DEVICE_ID);
  configurator.addVariable("mqtt_serv", MQTT_SERVER);
  configurator.addVariable("mqtt_topic_pub", MQTT_TOPIC_PUBLISH);
  configurator.addVariable("mqtt_topic_sub", MQTT_TOPIC_SUBSCRIBE);
  configurator.addVariable("wifi_ap_name", WIFI_AP_NAME);
  configurator.addVariable("wifi_ap_pass", WIFI_AP_PASSWORD);
  configurator.addVariable("sync_interval", DEFAULT_SYNC_INTERVAL);

  configurator.loadVariables();

  // pinMode(LED_PIN_PEMUPUKAN, OUTPUT);
  pinMode(LED_PIN_PENYIRAMAN, OUTPUT);

  // digitalWrite(LED_PIN_PEMUPUKAN, LOW);
  digitalWrite(LED_PIN_PENYIRAMAN, LOW);
  DEBUG_PRINTLN("Starting...");
  status_h.setID(configurator.getVariable("id"));
  status_h.setFirmwareVersion(FIRMWARE_VERSION);
  status_h.setActivation(true);
  status_h.setTotalOutput(6);
  status_h.setTotalInput(4);

  // outputHandler mainPump(2);

  // output.addOutput(waterPump, "waterPump");
  // output.addOutput(NutrientPump, "NutrientPump");
  // output.addOutput(lahan1, "lahan1");
  // output.addOutput(lahan2, "lahan2");

  display_h.initLCD();
  processor.buzzerBeep(500);
  delay(500);
  processor.buzzerBeep(500);
  display_h.displayWifiConnecting();

  DEBUG_PRINTLN("Connecting to WiFi...");
  DEBUG_PRINTLN(con.getSavedWifiSSID());
  DEBUG_PRINTLN(con.getSavedWifiPassword());
  WiFi.mode(WIFI_STA);
  WiFi.begin(con.getSavedWifiSSID().c_str(), con.getSavedWifiPassword().c_str());
  delay(1000);
  if (WiFi.status() != WL_CONNECTED)
  {
    processor.buzzerBeep(1000);
    DEBUG_PRINTLN("WiFi failed to connect");
    display_h.displayWifiNotConnected();
    display_h.displayWifiManagerStarted(configurator.getVariable("wifi_ap_name"), 60);
    con.startWiFiManager(configurator.getVariable("wifi_ap_name"), configurator.getVariable("wifi_ap_pass"), WIFI_SET_ONDEMAND, 60);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    processor.buzzerBeep(200);
    delay(100);
    processor.buzzerBeep(200);
    delay(100);
    processor.buzzerBeep(200);
    display_h.displayWifiConnected();
    DEBUG_PRINTLN("WiFi connected");
    ledPemupukan.on();
    // digitalWrite(LED_PIN_PEMUPUKAN, HIGH);
    stateConBefore = true;

    con.connectMQTT(configurator.getVariable("mqtt_serv"), 1883, configurator.getVariable("id"));
    con.mqttSubscribe(configurator.getVariable("mqtt_topic_sub"));
  }
  else
  {
    stateConBefore = false;
    processor.buzzerBeep(1000);
    display_h.displayContinueWithoutWiFi();
    DEBUG_PRINTLN("WiFi failed to connect");
    DEBUG_PRINTLN("Continue without WiFi...");
    ledPemupukan.off();
    // digitalWrite(LED_PIN_PEMUPUKAN, LOW);
    // WiFi.disconnect();
  }

  display_h.initDisplayInformation();
  // con.startWiFiManager(WIFI_AP_NAME, WIFI_AP_PASSWORD, WIFI_SET_AUTOCONNECT, 120);
  delay(100);
  time_h.initRTC();
  if (WiFi.status() == WL_CONNECTED)
  {
    time_h.syncTime();
  }

  delay(100);

  // load config
  input_h.initSensor();

  doc = storage.loadConfig();
  status_h.updateFromLoadConfig(doc);

  // xTaskCreatePinnedToCore(
  //     Task1code, /* Task function. */
  //     "Task1",   /* name of task. */
  //     4096,      /* Stack size of task */
  //     NULL,      /* parameter of the task */
  //     1,         /* priority of the task */
  //     &Task1,    /* Task handle to keep track of created task */
  //     1);

  // xTaskCreatePinnedToCore(
  //     Task2code, /* Task function. */
  //     "Task2",   /* name of task. */
  //     11000,     /* Stack size of task */
  //     NULL,      /* parameter of the task */
  //     1,         /* priority of the task */
  //     &Task2,    /* Task handle to keep track of created task */
  //     1);

  // runner.startNow();
  // runner.addTask(taskConnection);
  // taskConnection.enable();
  DEBUG_PRINTLN(getArduinoLoopTaskStackSize());
  DEBUG_PRINTLN(getCpuFrequencyMhz());

  syncronize();

  configurator.loadVariables();

  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      1);

  // setCpuFrequencyMhz(80);
  // getCpuFrequencyMhz();
  // Stacksiz
  // CONFIG_ARDUINO_LOOP_STACK_SIZE
  DEBUG_PRINTLN("Config loaded");
  button.attachClick(restartWiFiManager);

  msCounter = millis();
}

unsigned long lastTime = 0;

void loop()
{

  if (millis() - lastTime > 500)
  {
    time_h.getSourceTime();
    lastTime = millis();
  }

  // DEBUG_PRINTLN("Test Point 1");
  display_h.displayClock(time_h.getDate(), time_h.getTimeHourMinute());
  // DEBUG_PRINTLN("Test Point 2");
  // display_h.displayConnectionStatus(con.isConnected() ? "+" : "-");
  // DEBUG_PRINTLN("Test Point 3");
  // display_h.displaySuhu(String(input_h.getDHT1Temp(), 1));
  // DEBUG_PRINTLN("Test Point 4");
  // display_h.displayKelembaban(String(input_h.getDHT1Hum(), 1));
  // DEBUG_PRINTLN("Test Point 5");

  // DEBUG_PRINTLN("Test Point 6");
  // display_h.displayStatusPendjadwalan(status_h.getPenjadwalanOn() ? "+" : "-");
  // DEBUG_PRINTLN("Test Point 7");
  if (counterForTime >= 0)
  {
    if (cycle == 0)
    {
      display_h.displayBaris(1, display_h.makeBarisFrom2String(String() + "\1:" + String(input_h.getDHT1Temp(), 1) + "\xDF" + "C", "\2:" + String(input_h.getDHT1Hum(), 1) + "%", 0, 9));
    }
    else if (cycle == 1)
    {
      // display_h.displayTipe(status_h.getTipeTerdekatHariIni());
      // display_h.displayWaktuPendjadwalanTerdekat(status_h.getWaktuPendajwalanTerdekatHariIni());

      display_h.displayBaris(1, display_h.makeBarisFrom2String(String() + "T:" + status_h.getTipeTerdekatHariIni(), "W:" + status_h.getWaktuPendajwalanTerdekatHariIni(), 0, 9));
    }
  }

  if (counterForTime > batas)
  {
    counterForTime = 0;

    cycle++;
    if (cycle > 1)
    {
      cycle = 0;
    }
  }

  if (millis() - msCounter > 500)
  {
    // secondCounter++;
    counterForTime++;
    msCounter = millis();
  }

  // if (secondCounter <= 2)
  // {
  //   display_h.displayTipe(status_h.getTipeTerdekatHariIni());
  //   display_h.displayWaktuPendjadwalanTerdekat(status_h.getWaktuPendajwalanTerdekatHariIni());

  // }
  // else if (secondCounter <= 4)
  // {
  //   display_h.displaySuhu(String(input_h.getDHT1Temp(), 1));
  //   display_h.displayKelembaban(String(input_h.getDHT1Hum(), 1));
  //   /* code */
  // }
  // else
  // {
  //   secondCounter = 0;
  // }

  // DEBUG_PRINTLN("Test Point 8");
  // display_h.displayLahan1(status_h.getLahan1() ? "+" : "-");
  // DEBUG_PRINTLN("Test Point 9");
  // display_h.displayLahan2(status_h.getLahan2() ? "+" : "-");
  // DEBUG_PRINTLN("Test Point 10");
  // display_h.displayLahan3(status_h.getLahan3() ? "+" : "-");
  // display_h.displayLahan4(status_h.getLahan4() ? "+" : "-");
  display_h.setInit(false);
  // DEBUG_PRINTLN("Test Point 11");

  button.tick();
  // DEBUG_PRINTLN("Test Point 12");

  // save config
  String temp = con.getLastMessage();
  if (temp != "")
  {
    deserializeJson(doc, temp);
    // serializeJson(doc, temp);
    // DEBUG_PRINTLN(temp);
    DEBUG_PRINTLN(doc["mode"].as<String>());
    if (doc["mode"].as<String>() == "auto")
    {
      storage.saveConfig(doc);
      delay(100);
      doc.clear();
      doc = storage.loadConfig();
      String temp;
      serializeJson(doc, temp);
      // DEBUG_PRINTLN(temp);
      status_h.updateFromLoadConfig(doc);
      status_h.setIsStatusChanged(true);
      processor.resetQueue();

      processor.buzzerBeep(500);
    }
    else if (doc["mode"].as<String>() == "manual")
    {
      processor.buzzerBeep(500);

      // Todo: manual mode
      processor.processManualMode(doc);
      doc.clear();

      doc = storage.loadConfig();
      /* code */
    }
  }

  if (processor.getIsSeconds2(120))
  {
    DEBUG_PRINTLN("Reinit LCD");
    display_h.reinitLCD();
  }

  if (processor.getIsSeconds(10))
  {
    if (con.isMqttConnected())
    {
      DEBUG_PRINTLN("Sending Data...");
      String temp;

      serializeJson(status_h.getStatusJSON(), temp);
      con.mqttPublish(configurator.getVariable("mqtt_topic_pub"), temp);
      time_h.updateTime();
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      DEBUG_PRINTLN("WiFi Not Connected, Try to reconnecting");
      con.reconnectWiFi();
    }
  }

  if (status_h.isStatusChanged())
  {
    processor.resetCounter();
    if (con.isMqttConnected())
    {
      DEBUG_PRINTLN("Sending Data Immedietly...");

      String temp;

      serializeJson(status_h.getStatusJSON(), temp);
      con.mqttPublish(configurator.getVariable("mqtt_topic_pub"), temp);
    }
    else
    {
      DEBUG_PRINTLN("internet not connected, Waiting Connecion...");
    }
  }

  if (processor.getIsSecondsBeat())
  {
    // time_h.getSourceTime();
    display_h.updateSecondsTick();

    con.connectionLoopMQTT();
  }

  processor.processAutoMode(doc);
  // runner.execute();
  con.connectionLoop();

  if (con.isConnected() != stateConBefore)
  {
    stateConBefore = con.isConnected();
    if (stateConBefore)
    {
      processor.buzzerBeep(200);
      delay(100);
      processor.buzzerBeep(200);
      delay(100);
      processor.buzzerBeep(200);
    }
    else
    {
      processor.buzzerBeep(1000);
    }
  }
}

// void Task2code(void *pvParameters)
// {
//   DEBUG_PRINT("Task2 running on core ");
//   DEBUG_PRINTLN(xPortGetCoreID());

//   for (;;)
//   {

//   }
// }

void restartWiFiManager()
{
  con.startWiFiManager(configurator.getVariable("wifi_ap_name"), configurator.getVariable("wifi_ap_pass"), WIFI_SET_ONDEMAND, 60);
}

// Task1code: blinks an LED every 1000 ms
// void Task1code(void *pvParameters)
// {
//   DEBUG_PRINT("Task1 running on core ");
//   DEBUG_PRINTLN(xPortGetCoreID());

//   for (;;)
//   {

//   }
// }

/**
 * @brief Synchronizes the device with the server.
 *
 * This function will be called every x seconds to synchronize the device with the server.
 * You can put your code here to perform the synchronization.
 *
 * If the WiFi is connected, it checks if the server is reachable by pinging it.
 * If the server is reachable, it checks for firmware updates using the firmware version and hardware version.
 * If there are firmware updates available, it updates the firmware.
 * After the firmware update, it performs additional actions.
 * Finally, it synchronizes the variables with the server.
 *
 * @note This function assumes the existence of the following objects: WiFi, configurator, and ota.
 *
 * @note The server URL and API endpoints used in this function are placeholders and should be replaced with the actual server information.
 */
void syncronize()
{
  // syncronize the device with the server
  // this function will be called every x seconds
  // you can put your code here
  // do something
  configurator.init();

  if (WiFi.status() == WL_CONNECTED)
  {

    // check if the server is reachable
    bool res = configurator.pingServer("/api/device-sync/ping");

    if (res)
    {
      // check for firmware updates

      res = configurator.syncFirmware(FIRMWARE_VERSION, HARDWARE_VERSION, "/api/device-sync/firmware-version");

      if (!res)
      {
        // update firmware
        // res = configurator.updateFirmware("/api/device-sync/firmware-update");
        res = ota.update("configurator.makerindo.co.id", 443, (String) "/api/device-sync/firmware?serial_number=" + configurator.getSerialNumber(), true);

        if (res)
        {
          // do something after firmware update
          ESP.restart();
        }
      }

      // sync variables
      res = configurator.syncVariables("/api/device-sync/check");

      if (configurator.getIsVariableGotUpdated())
      {
        ESP.restart();
      }
    }
    else
    {
      Serial.println(F("Server not reachable"));
    }
  }
  else
  {
    Serial.println(F("WiFi not connected"));
  }
}

/**
 * @brief Synchronizes the loop based on a specified interval.
 *
 * This function checks if a specified interval has passed and performs synchronization if necessary.
 * It increments a counter every second and when the counter reaches 5, it checks if the sync interval
 * condition is satisfied. If the condition is met, it performs synchronization and resets the counter.
 *
 * @note This function relies on the `configurator` object to retrieve the sync interval and perform synchronization.
 *
 * @see configurator
 */
void syncLoop()
{
  secondStamp = millis() / 1000;
  if (secondStamp != lastSecondStamp)
  {
    lastSecondStamp = secondStamp;
    counterSecond++;
  }

  // sync every 5 seconds and if the SYNC_INTERVAL is satisfied
  if (counterSecond >= 5)
  {
    int syncInterval = configurator.getVariable("sync_interval").toInt();
    if (configurator.syncEvery(syncInterval > 10 ? syncInterval : 10800))
    {
      Serial.println("Syncing...");

      lastSync = millis();
      lastSecondStamp = secondStamp;
      unsigned long timeStampSync = millis();
      if (WiFi.status() == WL_CONNECTED)
      {
        syncronize();
        Serial.println("WiFi connected");
        /* code */
      }
      else
      {
        Serial.println("WiFi not connected");
      }

      Serial.println("Sync Time: " + (String)(millis() - timeStampSync) + " ms");
      Serial.println("Machine Time: " + (String)(millis() / 1000) + " s");
      checkMem();
    }
    counterSecond = 0;
  }
}

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  configurator.init();

  // esp_task_wdt_delete(NULL); // Remove this task from the watchdog

  for (;;)
  {
    syncLoop();
  }
}

void checkMem()
{
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
}