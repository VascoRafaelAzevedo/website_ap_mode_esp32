#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>

// Constantes e variáveis
const char* ap_ssid = "ESP32-Access-Point";
const char* ap_password = "12345678";

// Variável que será mostrada no website
float sensor_value = 23.5;  // Exemplo: temperatura ou outro valor

// Servidor web (tentaremos HTTPS na 443, senão HTTP na 80)
WebServer server(80);
bool https_enabled = false;

// Declarações de funções
void setupAccessPoint();
void setupWebServer();
void handleRoot();
void handleNotFound();
String generateHTML();

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
  html += ".value-display { background: #007bff; color: white; padding: 20px; text-align: center; border-radius: 5px; margin: 20px 0; }";
  html += ".value-number { font-size: 3em; font-weight: bold; margin: 10px 0; }";
  html += ".info { background: #e9ecef; padding: 15px; border-radius: 5px; margin-top: 20px; }";
  html += ".refresh-btn { background: #28a745; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 10px 5px; }";
  html += ".refresh-btn:hover { background: #218838; }";
  html += "</style>";
  html += "<script>";
  html += "function refreshPage() { location.reload(); }";
  html += "setInterval(refreshPage, 10000);"; // Auto refresh a cada 10 segundos
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ ESP32 Monitor</h1>";
  html += "<div class='value-display'>";
  html += "<div>Valor Atual:</div>";
  html += "<div class='value-number'>" + String(sensor_value, 1) + "</div>";
  html += "</div>";
  html += "<button class='refresh-btn' onclick='refreshPage()'>🔄 Atualizar</button>";
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