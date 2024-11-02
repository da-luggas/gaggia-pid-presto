#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <max6675.h>
#include <PID_v1.h>
#include <EEPROM.h>
#include <pidautotuner.h>

// Define addresses for EEPROM
const int ADDR_BREW_TEMP = 0;
const int ADDR_STEAM_TEMP = 8;
const int ADDR_KP = 16;
const int ADDR_KI = 24;
const int ADDR_KD = 32;
const int EEPROM_INIT_ADDR = 40;
const int EEPROM_SIZE = 48;
byte EEPROM_INIT_FLAG = 0x39;

// Define thermocouple and relay pin numbers
const int THERMO_DO_PIN = 12;
const int THERMO_CS_PIN = 15;
const int THERMO_CLK_PIN = 14;
const int RELAY_PIN = 16;

// Define variables holding user configured temperatures
double brewTemp = 95, steamTemp = 165;

// Define Variables we'll be connecting to
double Setpoint = brewTemp, Input, Output;
const double MaxBoilerTemp = 170;

// Specify the links and initial tuning parameters
double Kp = 1, Ki = 1, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Set time variables for non-blocking delays
unsigned long WindowSize = 2000;
unsigned long windowStartTime;
unsigned long lastTempReadTime = 0;

// Initialize thermocouple
MAX6675 thermocouple(THERMO_CLK_PIN, THERMO_CS_PIN, THERMO_DO_PIN);

// Initialize Webserver
ESP8266WebServer server(80);

//////////////////////
// HELPER FUNCTIONS //
//////////////////////

void writeToEEPROM()
{
  EEPROM.put(ADDR_BREW_TEMP, brewTemp);
  EEPROM.put(ADDR_STEAM_TEMP, steamTemp);
  EEPROM.put(ADDR_KP, Kp);
  EEPROM.put(ADDR_KI, Ki);
  EEPROM.put(ADDR_KD, Kd);
  EEPROM.commit();
}

void controlRelay()
{
  // Safety shutdown when temperature is too high
  if (Input > MaxBoilerTemp)
  {
    digitalWrite(RELAY_PIN, LOW);
    return;
  }
  // Turn the output pin on/off based on pid output
  if (millis() - windowStartTime > WindowSize)
  { // Time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output * WindowSize / 100.0 > millis() - windowStartTime)
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else
  {
    digitalWrite(RELAY_PIN, LOW);
  }
}

void readThermocouple()
{
  unsigned long currentMillis = millis();

  // Read temperature only every 250ms (Maxiumum of MAX6675)
  if (currentMillis - lastTempReadTime >= 250)
  {
    lastTempReadTime = currentMillis;
    Input = thermocouple.readCelsius();
  }
}

// Webserver functions

String buildWebsite()
{
  String mode = (Setpoint == steamTemp) ? "Steaming" : "Brewing";
  String checkboxChecked = (Setpoint == steamTemp) ? "checked" : "";
  String updateScript =
      "<script>"
      "    function updateTemperature() {"
      "        var xhttp = new XMLHttpRequest();"
      "        xhttp.onreadystatechange = function() {"
      "            if (this.readyState == 4 && this.status == 200) {"
      "                document.getElementById('currentTemp').innerHTML = this.response + ' °C';"
      "                document.getElementById('progress').value = Number(this.response);"
      "            };"
      "        };"
      "        xhttp.open('GET', '/getCurrentTemp', true);"
      "        xhttp.send();"
      "    }"
      "    setInterval(updateTemperature, 1000);"
      "</script>";

  String changeScript =
      "<script>"
      "    function changeMode() {"
      "       var mode = document.getElementById('steamMode').checked ? 1 : 0;"
      "       var xhttp = new XMLHttpRequest();"
      "        xhttp.onreadystatechange = function() {"
      "            if (this.readyState == 4 && this.status == 200) {"
      "                location.reload();"
      "            };"
      "        };"
      "       xhttp.open('GET', '/changeMode?steamMode=' + mode, true);"
      "       xhttp.send();"
      "    }"
      "</script>";

  String htmlPage =
      "<!doctype html>"
      "<head>"
      "    <meta charset='utf-8'>"
      "    <meta name='viewport' content='width=device-width, initial-scale=1'>"
      "    <link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/@picocss/pico@1/css/pico.min.css'>"
      "    <link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.3/font/bootstrap-icons.min.css'>"
      "    <link rel='apple-touch-icon' href='https://raw.githubusercontent.com/da-luggas/gaggia-pid-presto/main/icon.png'>"
      "    <style>"
      "        [data-theme='light'],"
      "        :root:not([data-theme='dark']) {"
      "          --primary: #846C5B;"
      "          --primary-hover: #846C5B;"
      "          --primary-focus: rgba(132, 108, 91, 0.25);"
      "          --primary-inverse: #FFF;"
      "        }"
      "    </style>"
      "    <title>Gaggia Classic</title>" +
      updateScript + changeScript +
      "</head>"
      "<body>"
      "    <main class='container'>"
      "        <hgroup>"
      "            <h2>Gaggia Classic</h2>"
      "            <p>" +
      mode + " Temperature: <strong>" + String(int(Setpoint)) + " °C</strong></p>"
                                                                "        </hgroup>"
                                                                "        <h1 id='currentTemp' style='margin-bottom: 0'>" +
      String(int(Input)) + " °C</h1>"
                           "        <progress id='progress' value='" +
      String(int(Input)) + "' max='" + String(int(Setpoint)) + "'></progress>"
                                                               "        <label for='steamMode'>"
                                                               "            <input type='checkbox' id='steamMode' name='steamMode' role='switch' " +
      checkboxChecked + " onChange='changeMode()'>"
                        "            <i class='bi-wind'></i> Steam Mode"
                        "        </label>"
                        "        <h1></h1>"
                        "        <details>"
                        "            <summary>Temperature Settings</summary>"
                        "            <form action='/setTemps' method='GET'>"
                        "            <label for='brewTemp'>"
                        "                Brewing Temperature (°C)"
                        "                <input type='text' id='brewTemp' name='brewTemp' placeholder='" +
      String(int(brewTemp)) + "'>"
                              "            </label>"
                              "            <label for='steamTemp'>"
                              "                Steaming Temperature (°C)"
                              "                <input type='text' id='steamTemp' name='steamTemp' placeholder='" +
      String(int(steamTemp)) + "'>"
                               "            </label>"
                               "            <button type='submit'>Save</button>"
                               "            </form>"
                               "        </details>"
                               "        <details>"
                               "            <summary>PID Settings</summary>"
                               "            <form action='/setPID' method='GET'>"
                               "            <label for='Kp'>"
                               "                Kp"
                               "                <input type='text' id='Kp' name='Kp' placeholder='" +
      String(Kp) + "'>"
                   "            </label>"
                   "            <label for='Ki'>"
                   "                Ki"
                   "                <input type='text' id='Ki' name='Ki' placeholder='" +
      String(Ki) + "'>"
                   "            </label>"
                   "            <label for='Kd'>"
                   "                Kd"
                   "                <input type='text' id='Kd' name='Kd' placeholder='" +
      String(Kd) + "'>"
                   "            </label>"
                   "            <a href='/autotune' role='button' class='outline' style='display:block'>Rerun Autotune</a>"
                   "            <small>When pressed, autotune will run again on the next boot. The process takes around 10 min. During that time, the web interface will not be available.<br /><br /></small>"
                   "            <button type='submit'>Save</button>"
                   "            </form>"
                   "        </details>"
                   "    </main>"
                   "</body>"
                   "</html>";
  return htmlPage;
}

void onConnect()
{
  server.send(200, "text/html", buildWebsite());
}

void getCurrentTemp()
{
  server.send(200, "text/plain", String(int(Input)));
}

void setTemps()
{
  if (server.hasArg("steamTemp") && server.arg("steamTemp") != "")
  {
    steamTemp = server.arg("steamTemp").toDouble();
    if (Setpoint == steamTemp)
    {
      Setpoint = steamTemp;
    }
  }
  if (server.hasArg("brewTemp") && server.arg("brewTemp") != "")
  {
    brewTemp = server.arg("brewTemp").toDouble();
    if (Setpoint != steamTemp)
    {
      Setpoint = brewTemp;
    }
  }

  writeToEEPROM();
  server.send(200, "text/html", buildWebsite());
}

void changeMode()
{
  if (server.hasArg("steamMode"))
  {
    if (server.arg("steamMode") == "1")
    {
      Setpoint = steamTemp;
    }
    else
    {
      Setpoint = brewTemp;
    }
  }
  server.send(200, "text/html", buildWebsite());
}

void manualAutotune()
{
  EEPROM_INIT_FLAG++;
  EEPROM.put(EEPROM_INIT_ADDR, EEPROM_INIT_FLAG);
  EEPROM.commit();

  server.send(200, "text/html", buildWebsite());
}

void setPID()
{
  if (server.hasArg("Kp") && server.arg("Kp") != "")
  {
    Kp = server.arg("Kp").toDouble();
  }
  if (server.hasArg("Ki") && server.arg("Ki") != "")
  {
    Ki = server.arg("Ki").toDouble();
  }
  if (server.hasArg("Kd") && server.arg("Kd") != "")
  {
    Kd = server.arg("Kd").toDouble();
  }

  myPID.SetTunings(Kp, Ki, Kd);
  writeToEEPROM();

  server.send(200, "text/html", buildWebsite());
}

void notFound()
{
  server.send(404, "text/plain", "Not found");
}

///////////////
// MAIN LOOP //
///////////////

void setup()
{
  Serial.begin(9600);
  // Read saved values from EEPROM
  EEPROM.begin(EEPROM_SIZE);

  byte initFlag = 0;
  EEPROM.get(EEPROM_INIT_ADDR, initFlag);

  // Set relay pin to low initially
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // WiFiManager
  WiFiManager wifiManager;

  // Fetches ssid and pass from eeprom and tries to connect
  // If it does not connect it starts an access point with the specified name
  wifiManager.autoConnect("Gaggia Classic");

  // Start web server
  server.on("/", onConnect);
  server.on("/setTemps", setTemps);
  server.on("/changeMode", changeMode);
  server.on("/getCurrentTemp", getCurrentTemp);
  server.on("/setPID", setPID);
  server.on("/autotune", manualAutotune);
  server.onNotFound(notFound);

  // Wait for MAX chip to stabilize
  delay(2000);

  windowStartTime = millis();

  if (initFlag != EEPROM_INIT_FLAG)
  {
    // EEPROM is not initialized or data is corrupted, so set default values

    unsigned long lastAutotuneTime = 0;
    unsigned long autotuneWindowStartTime = micros();

    PIDAutotuner tuner = PIDAutotuner();

    tuner.setTargetInputValue(brewTemp);
    tuner.setLoopInterval(250 * 1000);
    tuner.setOutputRange(0, 100);
    tuner.setZNMode(PIDAutotuner::ZNModeBasicPID);

    tuner.startTuningLoop(micros());

    while (!tuner.isFinished())
    {
      unsigned long currentMicros = micros();

      // Ensure the loop runs every 250ms
      if (currentMicros - lastAutotuneTime >= (250 * 1000))
      {
        lastAutotuneTime = currentMicros;

        // Read temperature
        Input = thermocouple.readCelsius();

        // Tune PID
        Output = tuner.tunePID(Input, currentMicros);

        // Set output (basically like controlRelay() but in microseconds)
        // Safety shutdown when temperature is too high
        if (Input > MaxBoilerTemp)
        {
          digitalWrite(RELAY_PIN, LOW);
        }
        // Turn the output pin on/off based on pid output
        if (micros() - autotuneWindowStartTime > WindowSize * 1000)
        { // Time to shift the Relay Window
          autotuneWindowStartTime += WindowSize * 1000;
        }
        if (Output * WindowSize * 1000 / 100.0 > micros() - autotuneWindowStartTime)
        {
          digitalWrite(RELAY_PIN, HIGH);
        }
        else
        {
          digitalWrite(RELAY_PIN, LOW);
        }
      }

      yield();
    }
    digitalWrite(RELAY_PIN, LOW); // Turn off the output

    // Store the tuned PID values
    Kp = tuner.getKp();
    Ki = tuner.getKi();
    Kd = tuner.getKd();
    myPID.SetTunings(Kp, Ki, Kd);

    writeToEEPROM();

    EEPROM.put(EEPROM_INIT_ADDR, EEPROM_INIT_FLAG);
    EEPROM.commit();
  }
  else
  {
    // Read the stored values
    EEPROM.get(ADDR_BREW_TEMP, brewTemp);
    EEPROM.get(ADDR_STEAM_TEMP, steamTemp);
    EEPROM.get(ADDR_KP, Kp);
    EEPROM.get(ADDR_KI, Ki);
    EEPROM.get(ADDR_KD, Kd);

    myPID.SetTunings(Kp, Ki, Kd);
  }

  Setpoint = brewTemp;

  server.begin(); // Makes web server available

  // Tell the PID to range between 0 and 100 (percentage)
  myPID.SetOutputLimits(0, 100);

  myPID.SetSampleTime(250);

  // Turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void loop()
{
  readThermocouple();
  myPID.Compute();
  controlRelay();
  server.handleClient();
}