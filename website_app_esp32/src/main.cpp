#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>

// Constantes e variáveis
const char* ap_ssid = "ESP32-Access-Point";
const char* ap_password = "12345678";

// Variável que será mostrada no website
float sensor_value = 23.5;  // Exemplo: temperatura ou outro valor

// Cor atual guardada no ESP (memória volátil)
String currentColor = "#007bff";

// Servidor web (tentaremos HTTPS na 443, senão HTTP na 80)
WebServer server(80);
bool https_enabled = false;

// Declarações de funções
void setupAccessPoint();
void setupWebServer();
void handleRoot();
void handleNotFound();
String generateHTML();

// Novas declarações
void handleGetColor();
void handleSetColor();
bool isHexChar(char c);
bool isValidColorHex(const String& s);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Iniciando ESP32 em modo Access Point...");
  
  // Configurar Access Point
  setupAccessPoint();
  
  // Configurar servidor web
  setupWebServer();
  
  Serial.println("Sistema pronto!");
  Serial.print("IP do Access Point: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Acesse o website em: http://" + WiFi.softAPIP().toString());
}

void loop() {
  server.handleClient();
  
  // Simular mudança na variável (opcional - para demonstração)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) { // Atualiza a cada 5 segundos
    sensor_value += random(-10, 11) / 10.0; // Varia ±1.0
    if (sensor_value < 0) sensor_value = 0;
    if (sensor_value > 100) sensor_value = 100;
    lastUpdate = millis();
  }
}

void setupAccessPoint() {
  // Configurar ESP32 como Access Point
  WiFi.mode(WIFI_AP);
  
  bool ap_success = WiFi.softAP(ap_ssid, ap_password);
  
  if (ap_success) {
    Serial.println("Access Point criado com sucesso!");
    Serial.print("SSID: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Erro ao criar Access Point!");
  }
}

void setupWebServer() {
  // Configurar rotas do servidor web
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  // API para cor
  server.on("/api/color", HTTP_GET, handleGetColor);
  server.on("/api/color", HTTP_POST, handleSetColor);
  
  // Iniciar servidor
  server.begin();
  Serial.println("Servidor web iniciado na porta 80");
}

void handleRoot() {
  String html = generateHTML();
  server.send(200, "text/html", html);
}

void handleNotFound() {
  String message = "Página não encontrada\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  
  server.send(404, "text/plain", message);
}

// --- NOVAS FUNÇÕES API DE COR ---
bool isHexChar(char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool isValidColorHex(const String& s) {
  if (s.length() != 7 && s.length() != 4) return false; // #RRGGBB ou #RGB
  if (s.charAt(0) != '#') return false;
  for (size_t i = 1; i < s.length(); ++i) {
    if (!isHexChar(s.charAt(i))) return false;
  }
  return true;
}

void handleGetColor() {
  server.send(200, "application/json", String("{\"color\":\"") + currentColor + "\"}");
}

void handleSetColor() {
  String value = server.arg("value");         // espera 'value=#rrggbb'
  if (value.isEmpty() && server.hasArg("plain")) {
    value = server.arg("plain");              // fallback simples
    value.trim();
  }
  if (!isValidColorHex(value)) {
    server.send(400, "application/json", "{\"error\":\"invalid color. use #RRGGBB or #RGB\"}");
    return;
  }
  currentColor = value;
  server.send(200, "application/json", String("{\"color\":\"") + currentColor + "\"}");
}
// --- FIM API COR ---

String generateHTML() {
  String html = "<!DOCTYPE html>";
  html += "<html lang='pt'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 Monitor</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #333; text-align: center; margin-bottom: 30px; }";
  html += ".value-display { background: #007bff; color: white; padding: 20px; text-align: center; border-radius: 5px; margin: 20px 0; transition: background-color 200ms ease, color 200ms ease; }";
  html += ".value-number { font-size: 3em; font-weight: bold; margin: 10px 0; }";
  html += ".info { background: #e9ecef; padding: 15px; border-radius: 5px; margin-top: 20px; }";
  html += ".refresh-btn { background: #28a745; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 10px 5px; }";
  html += ".refresh-btn:hover { background: #218838; }";
  html += ".palette { display: grid; grid-template-columns: repeat(5, 1fr); gap: 8px; margin-top: 10px; }";
  html += ".color-btn { border: none; height: 36px; border-radius: 6px; cursor: pointer; }";
  html += ".color-btn:hover { filter: brightness(0.9); }";
  html += "</style>";
  html += "<script>";
  html += "function refreshPage(){ location.reload(); }";
  html += "setInterval(refreshPage, 10000);";
  html += "document.addEventListener('DOMContentLoaded', function(){";
  html += "  var valueBox = document.querySelector('.value-display');";
  html += "  function pickTextColor(bg){ try{ var c=bg.charAt(0)=='#'?bg.substring(1):bg; if(c.length===3){ c=c[0]+c[0]+c[1]+c[1]+c[2]+c[2]; } var n=parseInt(c,16); var r=(n>>16)&255, g=(n>>8)&255, b=n&255; var lum=(0.299*r+0.587*g+0.114*b)/255; return lum>0.6?'#000':'#fff'; }catch(e){ return '#fff'; } }";
  html += "  function applyColor(color){ if(!valueBox) return; valueBox.style.backgroundColor=color; valueBox.style.color=pickTextColor(color); }";
  html += "  window.setColor = function(color){ applyColor(color); fetch('/api/color', { method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:'value='+encodeURIComponent(color) }).catch(function(){}); };";
  html += "  fetch('/api/color').then(function(r){ return r.json(); }).then(function(d){ if(d && d.color){ applyColor(d.color); } }).catch(function(){});";
  html += "});";
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ ESP32 Monitor</h1>";
  // aplica a cor atual já no HTML para evitar flicker inicial
  html += "<div class='value-display' style='background:" + currentColor + ";'>";
  html += "<div>Valor Atual:</div>";
  html += "<div class='value-number'>" + String(sensor_value, 1) + "</div>";
  html += "</div>";
  html += "<button class='refresh-btn' onclick='refreshPage()'>🔄 Atualizar</button>";

  // Paleta de 20 cores
  html += "<div class='palette'>";
  html += "<button class='color-btn' style='background:#007bff' onclick='setColor(\"#007bff\")' title='#007bff'></button>";
  html += "<button class='color-btn' style='background:#dc3545' onclick='setColor(\"#dc3545\")' title='#dc3545'></button>";
  html += "<button class='color-btn' style='background:#28a745' onclick='setColor(\"#28a745\")' title='#28a745'></button>";
  html += "<button class='color-btn' style='background:#ffc107' onclick='setColor(\"#ffc107\")' title='#ffc107'></button>";
  html += "<button class='color-btn' style='background:#17a2b8' onclick='setColor(\"#17a2b8\")' title='#17a2b8'></button>";
  html += "<button class='color-btn' style='background:#6f42c1' onclick='setColor(\"#6f42c1\")' title='#6f42c1'></button>";
  html += "<button class='color-btn' style='background:#fd7e14' onclick='setColor(\"#fd7e14\")' title='#fd7e14'></button>";
  html += "<button class='color-btn' style='background:#343a40' onclick='setColor(\"#343a40\")' title='#343a40'></button>";
  html += "<button class='color-btn' style='background:#6c757d' onclick='setColor(\"#6c757d\")' title='#6c757d'></button>";
  html += "<button class='color-btn' style='background:#20c997' onclick='setColor(\"#20c997\")' title='#20c997'></button>";
  html += "<button class='color-btn' style='background:#e83e8c' onclick='setColor(\"#e83e8c\")' title='#e83e8c'></button>";
  html += "<button class='color-btn' style='background:#6610f2' onclick='setColor(\"#6610f2\")' title='#6610f2'></button>";
  html += "<button class='color-btn' style='background:#198754' onclick='setColor(\"#198754\")' title='#198754'></button>";
  html += "<button class='color-btn' style='background:#0d6efd' onclick='setColor(\"#0d6efd\")' title='#0d6efd'></button>";
  html += "<button class='color-btn' style='background:#ff6b6b' onclick='setColor(\"#ff6b6b\")' title='#ff6b6b'></button>";
  html += "<button class='color-btn' style='background:#4dabf7' onclick='setColor(\"#4dabf7\")' title='#4dabf7'></button>";
  html += "<button class='color-btn' style='background:#ffcd39' onclick='setColor(\"#ffcd39\")' title='#ffcd39'></button>";
  html += "<button class='color-btn' style='background:#00b894' onclick='setColor(\"#00b894\")' title='#00b894'></button>";
  html += "<button class='color-btn' style='background:#1abc9c' onclick='setColor(\"#1abc9c\")' title='#1abc9c'></button>";
  html += "<button class='color-btn' style='background:#2ecc71' onclick='setColor(\"#2ecc71\")' title='#2ecc71'></button>";
  html += "</div>";

  html += "<div class='info'>";
  html += "<strong>Informações do Sistema:</strong><br>";
  html += "• IP do ESP32: " + WiFi.softAPIP().toString() + "<br>";
  html += "• SSID: " + String(ap_ssid) + "<br>";
  html += "• Última atualização: " + String(millis()/1000) + "s<br>";
  html += "• Free Heap: " + String(ESP.getFreeHeap()) + " bytes";
  html += "</div>";
  html += "</div>";
  html += "</body>";
  html += "</html>";
  return html;
}