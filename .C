#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

#define RELAY_PIN  23     // Devkit V1 genelde 23, istersen degistirebilirsin
#define EEPROM_ADDR 0

WebServer server(80);

int relayTime = 1;  // dakika

// ---- HTML SAYFA ----
const char MAIN_page[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>ESP Settings</title>
<style>
    body{font-family:Arial;background:#f2f2f2;margin:0;padding:20px;}
    .box{background:#fff;padding:20px;border-radius:10px;max-width:400px;margin:auto;
    box-shadow:0 0 10px rgba(0,0,0,0.1);}
    label{font-size:16px;display:block;margin-bottom:8px;}
    input{width:100%;padding:10px;font-size:15px;border-radius:6px;border:1px solid #ccc;margin-bottom:20px;}
    button{width:100%;padding:12px;font-size:17px;border:none;border-radius:8px;
           background:#ff8800;color:#fff;font-weight:bold;cursor:pointer;}
    button:hover{background:#ff7700;}
</style>
</head>
<body>
<div class="box">
<h2>ESP Settings</h2>
<form action="/save" method="POST">
<label for="t">Role acik kalma suresi (dk)</label>
<input type="number" id="t" name="t" min="1" max="999" required>
<button type="submit">Kaydet</button>
</form>
</div>
</body>
</html>
)====";

// ---- HTML KAYDEDILDI SAYFASI ----
const char OK_page[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Saved</title>
<style>
body{font-family:Arial;text-align:center;background:#f1f1f1;padding-top:50px;}
a{display:inline-block;margin-top:20px;padding:10px 20px;background:#ff8800;color:#fff;
  text-decoration:none;border-radius:8px;}
a:hover{background:#ff7700;}
</style>
</head>
<body>
<h2>Ayar kaydedildi</h2>
<a href="/">Geri Don</a>
</body>
</html>
)====";


void handleRoot() {
    server.send(200, "text/html", MAIN_page);
}

void handleSave() {
    if (server.hasArg("t")) {
        relayTime = server.arg("t").toInt();
        EEPROM.write(EEPROM_ADDR, relayTime);
        EEPROM.commit();
    }
    server.send(200, "text/html", OK_page);
}

void setup() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    EEPROM.begin(10);
    relayTime = EEPROM.read(EEPROM_ADDR);
    if (relayTime < 1 || relayTime > 200) relayTime = 1;

    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP-Setup", "12345678");

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
}

void loop() {
    server.handleClient();

    // Role tetikleme ornegi:
    // Buraya kendi kosulunu ekleyebilirsin.
    // Ornek: roleTime dakika aktif tut.

    // digitalWrite(RELAY_PIN, HIGH);
    // delay(relayTime * 60UL * 1000UL);
    // digitalWrite(RELAY_PIN, LOW);
}
