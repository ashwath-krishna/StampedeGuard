
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <VL53L1X.h>
#include <algorithm>

VL53L1X sensor;
WebServer server(80);

// Wi-Fi hotspot
const char* ssid = "StampedeGuard";
const char* password = "CHANGE_THIS_TO_A_PRIVATE_PASSWORD";

// Sensor settings
const int DETECT_DROP = 150;
const int RELEASE_DROP = 80;

int baseline = 0;
int lastDistance = -1;
int filteredDistance = 0;

bool sensorReady = false;
bool calibrated = false;
bool personDetected = false;
bool blocked = false;

unsigned long lastValidMillis = 0;
unsigned long lastReadMillis = 0;
unsigned long lastDetectionMillis = 0;
unsigned long blockedSince = 0;

unsigned long peopleCount = 0;

// Dashboard configuration
float standingArea = 100.0;
float cautionLevel = 2.5;
float dangerLevel = 3.5;

// Filtering
int readings[3] = {0, 0, 0};
int readIndex = 0;
int readCount = 0;

// Dashboard HTML
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>StampedeGuard</title>

<style>
body {
  margin: 0;
  padding: 20px;
  background: #f1f5f9;
  font-family: Arial, sans-serif;
  color: #172554;
}
.container {
  max-width: 850px;
  margin: auto;
}
.header {
  background: #102c54;
  color: white;
  padding: 22px;
  border-radius: 14px;
}
.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit,minmax(190px,1fr));
  gap: 15px;
  margin-top: 18px;
}
.card {
  background: white;
  padding: 20px;
  border-radius: 12px;
  box-shadow: 0 2px 8px #0001;
}
.value {
  font-size: 30px;
  font-weight: bold;
  margin-top: 12px;
}
.status {
  font-size: 25px;
  font-weight: bold;
  margin-top: 12px;
}
input {
  width: 90%;
  padding: 11px;
  margin: 8px 0;
  border: 1px solid #bbb;
  border-radius: 6px;
}
button {
  padding: 12px 18px;
  border: 0;
  border-radius: 7px;
  background: #1456a0;
  color: white;
  cursor: pointer;
  margin: 5px;
}
.small {
  color: #64748b;
  font-size: 13px;
}
.safe { color: #15803d; }
.caution { color: #ca8a04; }
.danger { color: #dc2626; }
</style>
</head>

<body>
<div class="container">

<div class="header">
  <h1>StampedeGuard</h1>
  <p>Real-time crowd density monitoring</p>
  <p>Camera-free entry monitoring system</p>
</div>

<div class="grid">

<div class="card">
  <div>People entered</div>
  <div class="value" id="count">0</div>
  <div class="small">Cumulative entry count</div>
</div>

<div class="card">
  <div>Estimated density</div>
  <div class="value" id="density">0.00</div>
  <div class="small">Entry-based estimate / m²</div>
</div>

<div class="card">
  <div>Crowd status</div>
  <div class="status safe" id="crowd">SAFE</div>
  <div class="small">Based on configured thresholds</div>
</div>

<div class="card">
  <div>Sensor distance</div>
  <div class="value" id="distance">--</div>
  <div class="small">Millimetres</div>
</div>

<div class="card">
  <div>Sensor health</div>
  <div class="status" id="health">CHECKING</div>
  <div class="small" id="baseline">Baseline: --</div>
</div>

<div class="card">
  <div>Standing area</div>
  <div class="value" id="areaDisplay">100</div>
  <div class="small">Square metres</div>
</div>

</div>

<div class="card" style="margin-top:18px">
  <h3>Venue configuration</h3>

  <label>Standing area (m²)</label><br>
  <input type="number" id="area" value="100" min="0.1" step="0.1"><br>

  <label>Caution threshold (people/m²)</label><br>
  <input type="number" id="cautionInput" value="2.5" min="0.1" step="0.1"><br>

  <label>Danger threshold (people/m²)</label><br>
  <input type="number" id="dangerInput" value="3.5" min="0.2" step="0.1"><br>

  <button onclick="saveConfig()">Save configuration</button>
  <button onclick="resetCount()">Reset demo count</button>

  <p class="small" id="message"></p>
</div>

<div class="card" style="margin-top:18px">
  <h3>System information</h3>
  <p>Connection: ESP32 Wi-Fi hotspot</p>
  <p>Sensor: VL53L1X</p>
  <p>Controller: ESP32</p>
  <p>Mode: Prototype demonstration</p>
  <p class="small">
    Exits are not counted. Entry count is not actual occupancy.
    This prototype is not a certified safety system.
  </p>
</div>

</div>

<script>
async function updateDashboard() {
  try {
    const response = await fetch('/api');
    const d = await response.json();

    document.getElementById('count').innerText = d.count;
    document.getElementById('density').innerText =
      d.density.toFixed(2);

    document.getElementById('distance').innerText =
      d.distance < 0 ? '--' : d.distance;

    document.getElementById('baseline').innerText =
      'Baseline: ' + (d.baseline < 0 ? '--' : d.baseline + ' mm');

    document.getElementById('areaDisplay').innerText =
      d.area;

    document.getElementById('health').innerText =
      d.health;

    const crowd = document.getElementById('crowd');
    crowd.innerText = d.crowd;
    crowd.className = 'status ' + d.crowd.toLowerCase();

  } catch(e) {
    document.getElementById('health').innerText =
      'CONNECTION LOST';
  }
}

async function saveConfig() {
  const area = document.getElementById('area').value;
  const caution = document.getElementById('cautionInput').value;
  const danger = document.getElementById('dangerInput').value;

  const response = await fetch('/config', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded'
    },
    body: new URLSearchParams({
      area: area,
      caution: caution,
      danger: danger
    })
  });

  document.getElementById('message').innerText =
    await response.text();

  updateDashboard();
}

async function resetCount() {
  if (!confirm('Reset the demonstration count?')) return;

  await fetch('/reset', { method: 'POST' });
  updateDashboard();
}

setInterval(updateDashboard, 1000);
updateDashboard();
</script>

</body>
</html>
)rawliteral";

// Get filtered sensor status
String getHealth() {
  if (!sensorReady || !calibrated ||
      millis() - lastValidMillis > 1500) {
    return "NO SENSOR DATA";
  }

  if (blocked) return "BLOCKED";

  return "ONLINE";
}

// Crowd warning status
String getCrowdStatus(float density) {
  if (density >= dangerLevel) return "DANGER";
  if (density >= cautionLevel) return "CAUTION";
  return "SAFE";
}

// API response
void handleAPI() {
  float density = peopleCount / standingArea;

  String json = "{";

  json += "\"count\":" + String(peopleCount) + ",";
  json += "\"distance\":" + String(lastDistance) + ",";
  json += "\"baseline\":" + String(calibrated ? baseline : -1) + ",";
  json += "\"density\":" + String(density, 2) + ",";
  json += "\"area\":" + String(standingArea, 1) + ",";
  json += "\"health\":\"" + getHealth() + "\",";
  json += "\"crowd\":\"" + getCrowdStatus(density) + "\"";

  json += "}";

  server.send(200, "application/json", json);
}

// Save venue settings
void handleConfig() {
  float newArea = server.arg("area").toFloat();
  float newCaution = server.arg("caution").toFloat();
  float newDanger = server.arg("danger").toFloat();

  if (newArea <= 0 || newArea > 100000 ||
      newCaution <= 0 || newDanger <= newCaution ||
      newDanger > 1000) {

    server.send(400, "text/plain",
                "Invalid settings. Check all values.");
    return;
  }

  standingArea = newArea;
  cautionLevel = newCaution;
  dangerLevel = newDanger;

  server.send(200, "text/plain",
              "Configuration saved successfully.");
}

// Reset demonstration count
void handleReset() {
  peopleCount = 0;
  personDetected = false;

  server.send(200, "text/plain", "Count reset.");
}

// Sensor calibration
void calibrateSensor() {
  Serial.println("Calibrating empty area...");

  uint16_t samples[40];
  int n = 0;

  for (int i = 0; i < 40; i++) {
    delay(100);

    int d = sensor.read();

    if (!sensor.timeoutOccurred() &&
        d > 0 && d < 4000) {
      samples[n++] = d;
    }
  }

  if (n < 30) {
    Serial.println("Calibration failed!");
    return;
  }

  std::sort(samples, samples + n);

  baseline = samples[n / 2];
  calibrated = true;

  Serial.print("Baseline: ");
  Serial.print(baseline);
  Serial.println(" mm");
}

// Setup
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  sensor.setTimeout(500);

  if (sensor.init()) {
    sensorReady = true;

    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(100);

    calibrateSensor();

  } else {
    Serial.println("VL53L1X not detected!");
  }

  // Start Wi-Fi hotspot
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.println("StampedeGuard Wi-Fi started!");
  Serial.print("Connect to: ");
  Serial.println(ssid);

  Serial.print("Dashboard IP: ");
  Serial.println(WiFi.softAPIP());

  // Start web server
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/api", HTTP_GET, handleAPI);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/reset", HTTP_POST, handleReset);

  server.begin();

  Serial.println("Dashboard server ready!");
}

// Main loop
void loop() {
  server.handleClient();

  if (sensorReady &&
      millis() - lastReadMillis >= 100) {

    lastReadMillis = millis();

    int distance = sensor.read();

    if (!sensor.timeoutOccurred() &&
        distance > 0 && distance < 4000) {

      lastValidMillis = millis();
      lastDistance = distance;

      // Three-reading moving average
      readings[readIndex] = distance;
      readIndex = (readIndex + 1) % 3;

      if (readCount < 3) readCount++;

      long total = 0;

      for (int i = 0; i < readCount; i++) {
        total += readings[i];
      }

      filteredDistance = total / readCount;

      // Blocked sensor check
      if (filteredDistance < 150) {
        if (blockedSince == 0) {
          blockedSince = millis();
        }

        if (millis() - blockedSince >= 5000) {
          blocked = true;
        }
      } else {
        blockedSince = 0;
        blocked = false;
      }

      // Person/object passage detection
      if (calibrated && !personDetected &&
          filteredDistance < baseline - DETECT_DROP &&
          millis() - lastDetectionMillis > 700) {

        peopleCount++;
        personDetected = true;
        lastDetectionMillis = millis();

        Serial.println("OBJECT DETECTED!");
        Serial.print("Total entries: ");
        Serial.println(peopleCount);
      }

      // Rearm after the lane becomes clear
      if (personDetected &&
          filteredDistance > baseline - RELEASE_DROP) {

        personDetected = false;

        Serial.println("Lane clear.");
      }
    }
  }

  delay(2);
}
