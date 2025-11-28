// Final .ino — Modern arayüz
// Röle: GPIO5, LED: GPIO2 (LED röle ile senkron).
// Röle aktif LOW: RELAY_ON = LOW.

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---------- WiFi ----------
const char* ssid = "TA3KRT";
const char* password = "?sngMuDvk@#";

// Statik IP (kullandığın)
IPAddress local_IP(192, 168, 0, 20);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------- Pins ----------
#define RELAY_PIN 5
#define LED_PIN 2

// Röle aktif - Durum:
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

// ---------- Server ----------
WebServer server(80);

// ---------- State ----------
volatile bool roleDurum = false;       // röle durumu
int heatTimeMin = 40;                  // dakika (varsayılan)
unsigned long heatStartTime = 0;       // millis() başladığı an (röle açılınca)
unsigned long relayUptimeStart = 0;    // uptime başlangıcı (millis)
unsigned long lastWifiAttempt = 0;
int wifiFailCount = 0;
const unsigned long wifiAttemptInterval = 3000; // ms
const int maxWifiFails = 3;

// HTML 
const char PAGE_HTML[] PROGMEM = R"====(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>Isitici Kontrol Paneli</title>
<style>
  :root{
    --bg:#0f1724;
    --card:#0b1320;
    --accent:#ff8800;
    --muted:#9aa7b2;
    --success:#3fc07f;
    --danger:#ff5c5c;
    --glass: rgba(255,255,255,0.03);
  }

  html,body{
    height:100%;
    margin:0;
    font-family: Inter, "Segoe UI", Roboto, Arial, sans-serif;
    background: linear-gradient(180deg,#071024 0%, #0b1220 60%);
    color:#e6eef6;
  }

  .wrap{
    max-width:980px;
    margin:28px auto;
    padding:24px;
  }

  .header{
    display:flex;
    align-items:center;
    justify-content:space-between;
    gap:16px;
    margin-bottom:20px;
  }

  .brand{
    display:flex;
    gap:14px;
    align-items:center;
  }

  .logo{
    width:64px;
    height:64px;
    border-radius:14px;
    background: linear-gradient(135deg,#1f5fff 0%, #6b8bff 100%);
    display:flex;
    align-items:center;
    justify-content:center;
    box-shadow: 0 8px 24px rgba(27,42,90,0.6);
  }
  .logo svg{ width:36px; height:36px; }

  .title h1{
    margin:0;
    font-size:20px;
  }
  .title p{
    margin:0;
    color:var(--muted);
    font-size:13px;
  }

  .status-bubble{
    background: linear-gradient(90deg,#06283a,#063a2f 70%);
    padding:10px 14px;
    border-radius:12px;
    display:flex;
    gap:12px;
    align-items:center;
    box-shadow: 0 6px 18px rgba(6,18,34,0.6);
  }
  .status-dot{
    width:12px;height:12px;border-radius:50%;background:var(--danger);
  }
  .status-text{ font-size:14px; color:var(--muted); }

  .grid{
    display:grid;
    grid-template-columns: 1fr 320px;
    gap:20px;
  }

  .card{
    background: linear-gradient(180deg, rgba(255,255,255,0.02), rgba(255,255,255,0.01));
    border-radius:16px;
    padding:22px;
    box-shadow: 0 10px 30px rgba(3,8,18,0.7);
    border: 1px solid rgba(255,255,255,0.03);
  }

  .controls{
    display:flex;
    gap:18px;
    flex-wrap:wrap;
    margin-bottom:18px;
  }

  .big-btn{
    width:140px;
    height:84px;
    border-radius:14px;
    border:none;
    cursor:pointer;
    color:#fff;
    font-weight:700;
    font-size:18px;
    display:flex;
    gap:12px;
    align-items:center;
    justify-content:center;
  }
  .btn-on{ background: linear-gradient(180deg,#1fbf62,#129347); }
  .btn-off{ background: linear-gradient(180deg,#ff6b6b,#e63f3f); }

  .input-row{
    display:flex;
    align-items:center;
    gap:12px;
    margin-top:6px;
  }

  .number{
    width:140px;
    padding:12px 14px;
    border-radius:10px;
    background:var(--glass);
    border: 1px solid rgba(255,255,255,0.04);
    color:var(--muted);
    font-size:18px;
    text-align:center;
  }

  .save-btn{
    background: linear-gradient(180deg,#ff8800, #ff6f00);
    border: none;
    color:white;
    padding:12px 18px;
    border-radius:12px;
    font-weight:700;
    cursor:pointer;
    font-size:16px;
  }

  .small-muted{ color:var(--muted); font-size:13px; }

  .panel{
    display:flex;
    flex-direction:column;
    gap:14px;
  }

  .card-2{
    background: linear-gradient(180deg, rgba(255,255,255,0.015), rgba(255,255,255,0.01));
    border-radius:12px;
    padding:18px;
    border:1px solid rgba(255,255,255,0.025);
  }

  .big-timer{
    display:flex;
    align-items:center;
    gap:14px;
    justify-content:space-between;
  }

  .timer-circle{
    width:120px;
    height:120px;
    border-radius:999px;
    display:flex;
    flex-direction:column;
    align-items:center;
    justify-content:center;
    background: radial-gradient(circle at 30% 20%, rgba(255,255,255,0.03), transparent 30%);
  }

  .progress-outer{
    height:12px;
    background: rgba(255,255,255,0.03);
    border-radius:999px;
    overflow:hidden;
    width:100%;
  }
  .progress-inner{
    height:100%;
    width:100%; /* full dolu basla */
    background: linear-gradient(90deg,#68e08b, #3fc07f);
    transition: width 0.5s linear;
  }
</style>
</head>
<body>
  <div class="wrap">

    <div class="header">
      <div class="brand">
        <div class="logo">
          <svg viewBox="0 0 24 24">
            <path d="M12 2C12 2 9 6 9 9.5C9 12 11.5 13 12 14.5C12.5 13 15 12 15 9.5C15 6 12 2 12 2Z" fill="white"/>
          </svg>
        </div>
        <div class="title">
          <h1>TA3KRT - Kombi Kontrol Paneli</h1>
          <p>ESP32 arayuz</p>
        </div>
      </div>

      <div class="status-bubble" id="wifiStatus">
        <div class="status-dot" id="statusDot"></div>
        <div class="status-text" id="statusText">Baglanti: Yok</div>
      </div>
    </div>

    <div class="grid">

      <!-- Sol -->
      <div class="card">
        <div class="controls">
          <button class="big-btn btn-on" onclick="onClick()">AC</button>
          <button class="big-btn btn-off" onclick="offClick()">KAPAT</button>

          <div style="flex:1"></div>

          <div>
            <div class="small-muted">Role acik kalma suresi (dk)</div>
            <div class="input-row">
              <input id="timeNumber" class="number" type="number" value="40" />
              <button class="save-btn" onclick="saveClick()">Kaydet</button>
            </div>
          </div>
        </div>

        <div style="margin-top:18px">
          <div style="display:flex;justify-content:space-between;">
            <div>
              <div class="small-muted">Role Durumu</div>
              <div id="relayState" style="font-size:18px;font-weight:800">KAPALI</div>
            </div>

            <div style="text-align:right">
              <div class="small-muted">Role acik kalma suresi</div>
              <div id="heatTimeTxt" style="font-size:18px;font-weight:800">40 dk</div>
            </div>
          </div>

          <div style="margin-top:18px;">
            <div class="small-muted">Kalan Sure</div>
            <div id="timeLeft" style="font-size:24px;font-weight:800">00:00</div>
            <div style="margin-top:10px" class="progress-outer">
              <div class="progress-inner" id="progressInner"></div>
            </div>
          </div>
        </div>
      </div>

      <!-- Sag -->
      <div class="panel">
        <div class="card-2">
          <div class="big-timer">
            <div class="timer-circle">
              <div class="timer-num" id="bigTimer">00:00</div>
              <div class="timer-sub">Kalan Sure</div>
            </div>
            <div style="flex:1;padding-left:16px">
              <div class="small-muted">Genel bilgi</div>
              <div style="margin-top:8px">
                <div><strong>Uptime:</strong> <span class="small-muted" id="uptimeVal">00:00:00</span></div>
                <div style="margin-top:6px"><strong>WiFi:</strong> <span class="small-muted" id="wifiText">--</span></div>
                <div style="margin-top:6px"><strong>IP:</strong> <span class="small-muted" id="ipText">--</span></div>
              </div>
            </div>
          </div>
        </div>

        <!-- Düzeltilen bölüm -->
        <div class="card-2" style="text-align:center">
          <div style="font-size:13px;color:var(--muted)">Versiyon</div>
          <div style="font-weight:800;margin-top:6px">v2.0</div>
          <div style="font-weight:800;margin-top:6px">TA3KRT - Sercan Kurt</div>   
        </div>

      </div>

    </div>

    <div class="foot small-muted">Sercan Kurt tarafindan tasarlanmistir. Son guncelleme: 28/11/2025</div>
  </div>

<script>
/* Client-side: ESP'den gerçek veriyi alır. Butonlar /on?min= ve /off çağırır. */
async function fetchData(){
  try{
    const r = await fetch('/data');
    const d = await r.json();

    // wifi status
    document.getElementById('statusText').textContent = d.wifi === 'Baglandi' ? 'Baglanti: Var' : 'Baglanti: Yok';
    document.getElementById('statusDot').style.background = d.wifi === 'Baglandi' ? 'var(--success)' : 'var(--danger)';
    document.getElementById('wifiText').textContent = d.wifi;
    document.getElementById('ipText').textContent = d.ip ? d.ip : '--';

    // relay / time
    document.getElementById('relayState').textContent = d.relay === 'Acik' ? 'ACIK' : 'KAPALI';
    document.getElementById('heatTimeTxt').textContent = d.total_min + ' dk';

    // kalan sure
    let rem = d.remain; // saniye
    let mm = String(Math.floor(rem/60)).padStart(2,'0');
    let ss = String(rem%60).padStart(2,'0');
    document.getElementById('timeLeft').textContent = mm + ':' + ss;
    document.getElementById('bigTimer').textContent = mm + ':' + ss;

    // progress
    let pct = d.total_sec ? (d.remain / d.total_sec) * 100 : 0;
    document.getElementById('progressInner').style.width = pct + '%';

    // uptime
    let s = d.uptime_sec || 0;
    let h = String(Math.floor(s/3600)).padStart(2,'0');
    let m = String(Math.floor((s%3600)/60)).padStart(2,'0');
    let ssU = String(s%60).padStart(2,'0');
    document.getElementById('uptimeVal').textContent = h+':'+m+':'+ssU;

  }catch(e){
    // hata -> wifi false goster
    document.getElementById('statusText').textContent = 'Baglanti: Yok';
    document.getElementById('statusDot').style.background = 'var(--danger)';
    document.getElementById('wifiText').textContent = 'Baglanamadi';
    document.getElementById('ipText').textContent = '--';
  }
}

setInterval(fetchData,1000);
fetchData();

function saveClick(){
  const mins = Number(document.getElementById('timeNumber').value) || 1;
  fetch('/save?min=' + mins);
  document.getElementById('heatTimeTxt').textContent = mins + ' dk';
}

/* on: min param ile gönderiyoruz */
function onClick(){
  const mins = Number(document.getElementById('timeNumber').value) || 1;
  // ESP'ye bildir ve aç
  fetch('/on?min=' + mins);
}

/* off */
function offClick(){
  fetch('/off');
}
</script>

</body>
</html>
)====";

// ---------- Helpers ----------
void setRelay(bool on) {
  roleDurum = on;
  digitalWrite(RELAY_PIN, on ? RELAY_ON : RELAY_OFF);
  digitalWrite(LED_PIN, on ? HIGH : LOW);

  if (on) {
    heatStartTime = millis();
    relayUptimeStart = millis();
  } else {
    heatStartTime = 0;
    relayUptimeStart = 0;
  }
}

// ---------- Handlers ----------
void handleRoot() {
  server.send_P(200, "text/html", PAGE_HTML);
}

void handleData() {
  // calc remain
  unsigned long remainSec = 0;
  unsigned long totalSec = (unsigned long)heatTimeMin * 60UL;
  if (roleDurum && heatStartTime != 0) {
    unsigned long totalMs = (unsigned long)heatTimeMin * 60000UL;
    unsigned long elapsed = millis() - heatStartTime;
    unsigned long remainMs = (elapsed >= totalMs) ? 0 : (totalMs - elapsed);
    remainSec = remainMs / 1000;
  }

  unsigned long uptimeRelay = 0;
  if (roleDurum && relayUptimeStart != 0) {
    uptimeRelay = (millis() - relayUptimeStart) / 1000;
  }

  StaticJsonDocument<256> doc;
  doc["wifi"] = (WiFi.status() == WL_CONNECTED) ? "Baglandi" : "Baglanamadi";
  doc["relay"] = roleDurum ? "Acik" : "Kapali";
  doc["total_sec"] = totalSec;
  doc["total_min"] = heatTimeMin;
  doc["remain"] = remainSec;
  doc["uptime_sec"] = uptimeRelay;
  doc["ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : String("");

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleOn() {
  // optional min parameter
  if (server.hasArg("min")) {
    int v = server.arg("min").toInt();
    if (v < 1) v = 1;
    if (v > 240) v = 240;
    heatTimeMin = v;
  }
  setRelay(true);
  server.send(200, "text/plain", "OK");
}

void handleOff() {
  setRelay(false);
  server.send(200, "text/plain", "OK");
}

void handleSave() {
  if (server.hasArg("min")) {
    int v = server.arg("min").toInt();
    if (v < 1) v = 1;
    if (v > 240) v = 240;
    heatTimeMin = v;
  }
  server.send(200, "text/plain", "SAVED");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // başlangıç röle kapalı
  digitalWrite(RELAY_PIN, RELAY_OFF);
  digitalWrite(LED_PIN, LOW);

  // statik IP
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  lastWifiAttempt = millis();

  // routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/save", handleSave);

  server.begin();
}

// ---------- Loop ----------
void loop() {
  // WiFi reconnect logic
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWifiAttempt >= wifiAttemptInterval) {
      lastWifiAttempt = millis();
      wifiFailCount++;
      WiFi.reconnect();
      Serial.printf("WiFi reconnect attempt %d\n", wifiFailCount);
    }
    if (wifiFailCount >= maxWifiFails) {
      Serial.println("WiFi failed too many times — restarting...");
      delay(3000);
      ESP.restart();
    }
  } else {
    wifiFailCount = 0;
  }

  // Auto-off when duration ended
  if (roleDurum && heatStartTime != 0) {
    unsigned long totalMs = (unsigned long)heatTimeMin * 60000UL;
    unsigned long elapsed = millis() - heatStartTime;
    if (elapsed >= totalMs) {
      setRelay(false);
      Serial.println("Auto-off: time elapsed");
    }
  }

  server.handleClient();
}
