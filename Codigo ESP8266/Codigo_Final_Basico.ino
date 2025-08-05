#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <UniversalTelegramBot.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define Sensor 2  // Pin D1 en la placa NodeMCU

OneWire oneWire(Sensor);
DallasTemperature sensors(&oneWire);

//************************************************************************************************************************************************************
// VARIABLES A MODIFICAR:
const char* ssid = "IDENTIFICADOR DE WIFI";                                      // Red wifi
const char* password = "PASS DE WIFI";                                          // Contraseña de la red
const char* Identificador = "/Manantiales";                                     // Identificador por el cual se solicitara la temperatura
const char* Mensaje1 = "ATENCION! La temperatura en Manantiales supero los: ";  // Mensaje al superar el umbral de 30°
const char* Mensaje2 = "Temperatura actual en Manantiales: ";                   // Mensaje que devuelve al identificador
const char* Mensaje3 = "Registro diario: temperatura en Manantiales: ";         // Mensaje diario
const float Umbral = 30.0;                                                      // Umbral de temperatura en grados Celsius
const int horaEnvioDiario = 13;                                                 // Hora de envío diario (formato 24hs)
const int minutoEnvioDiario = 10;                                               // Minuto de envío diario
//************************************************************************************************************************************************************

const char* telegramBotToken = " 123456 "; // Token del Bot de Telegram - borrar 123456 -

WiFiClientSecure client;
UniversalTelegramBot bot(telegramBotToken, client);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800); // -10800 para la zona horaria UTC-3

bool messageSent = false;
bool dailyMessageSent = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando...");
  }
  Serial.println("Conectado a WiFi exitosamente");
  client.setInsecure(); // Permitir conexiones inseguras, necesario para HTTPS en algunos casos
  timeClient.begin(); // Iniciar NTPClient
}

void loop() {
  timeClient.update(); // Actualizar el tiempo

  // Comprobación de actualizaciones del bot
  Serial.println("Comprobando actualizaciones del bot...");
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  if (numNewMessages > 0) {
    Serial.println("Nuevos mensajes recibidos");
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      Serial.println("Mensaje recibido: " + text);

      if (text.equals(Identificador)) {
        float temperatura = getTemperature();
        sendTelegramMessage(String(Mensaje2) + String(temperatura) + " °C", chat_id);
      }
    }
  } else {
    Serial.println("No se han recibido nuevos mensajes");
  }

  // Comprobación de temperatura y envío de mensaje si supera el umbral
  sensors.requestTemperatures();  // Solicita la temperatura al sensor
  float temperatureC = sensors.getTempCByIndex(0);  // Obtiene la temperatura en grados Celsius
  Serial.print("Temperatura: ");
  Serial.print(temperatureC);
  Serial.println(" ºC");

  if (temperatureC > Umbral && !messageSent) {
    sendTelegramMessage(String(Mensaje1) + String(temperatureC) + " ºC", "-1234567899");  //CAMBIAR -1234567899 POR EL IDENTIFICADOR DEL CHAT DONDE QUIERAS RECIBIR EL MENSAJE
    messageSent = true;
  } else if (temperatureC <= Umbral) {
    messageSent = false; // Reiniciar el flag si la temperatura vuelve a bajar por debajo de 30°C
  }

  // Enviar mensaje diario
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  if (currentHour == horaEnvioDiario && currentMinute == minutoEnvioDiario && !dailyMessageSent) {
    float temperatura = getTemperature();
    sendTelegramMessage(String(Mensaje3) + String(temperatura) + " °C", "-1234567899");
    dailyMessageSent = true;
  } else if (currentHour != horaEnvioDiario || currentMinute != minutoEnvioDiario) {
    dailyMessageSent = false; // Reiniciar el flag para el siguiente día
  }

  delay(1000); // Esperar un segundo antes de la siguiente iteración
}

float getTemperature() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  Serial.println("Temperatura medida: " + String(temp));
  return temp;
}

void sendTelegramMessage(String message, String chat_id) {
  Serial.println("Enviando mensaje...");

  // Usar el mismo objeto WiFiClientSecure para conexiones HTTPS
  client.setInsecure(); // Desactivar la verificación de certificado SSL (no recomendado para producción)

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(telegramBotToken) + "/sendMessage";

  Serial.println("Conectando a: " + url);

  http.begin(client, url); // Iniciar la conexión
  http.addHeader("Content-Type", "application/json");

  String requestBody = "{\"chat_id\":\"" + chat_id + "\",\"text\":\"" + message + "\"}";
  Serial.println("Request Body: " + requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Mensaje enviado correctamente");
    Serial.println("Respuesta: " + response);
  } else {
    Serial.print("Error al enviar el mensaje. Código de error HTTP: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
