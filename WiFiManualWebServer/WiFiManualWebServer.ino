/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

// Variabili per SSID e password
char ssid[32];
const char *password = "123456789";

WiFiServer server(80);

IPAddress myIP;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Ottieni il MAC address
  String macAddress = WiFi.softAPmacAddress();
  macAddress.replace(":", ""); // Rimuovi i due punti dal MAC address

  // Crea il nome della Wi-Fi con il MAC address
  sprintf(ssid, "ESPap_%s", macAddress.c_str());

  Serial.println();
  Serial.print("Configuring access point: ");
  Serial.println(ssid);

  // Configura l'AP con SSID e password
  WiFi.softAP(ssid, password);

  myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Avvia il server
  server.begin();
  Serial.println(F("Server started"));

  // Stampa l'indirizzo IP
  Serial.println(myIP);
}


void loop() {
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client) { return; }
  Serial.println(F("new client"));

  client.setTimeout(5000);  // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = 0;
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = 1;
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }


  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now "));
  client.print((val) ? F("high") : F("low"));
  client.print(F("<br><br>Click <a href='"));
  client.print(F("/gpio/1'>here</a> to switch LED GPIO on, or <a href='"));
  client.print(F("/gpio/0'>here</a> to switch LED GPIO off.</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}
