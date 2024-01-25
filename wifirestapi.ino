#if (defRestServerEnable)
const String postForms = "<html>\
  <head>\
    <title>ESP8266 Web Server POST handling</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>POST plain text to /postplain/</h1><br>\
    <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
      <input type=\"text\" name=\'{\"hello\": \"world\", \"trash\": \"\' value=\'\"}\'><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    <h1>POST form data to /postform/</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
  </body>\
</html>";
#endif

void handleRoot() {
  debugPrefix(defDebugServer);
  objRestServer.send(200, "text/html", postForms);
  debugServer("served root");
}

void handlePlain() {
  if (objRestServer.method() != HTTP_POST) {
    digitalWrite(led, 1);
    objRestServer.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(led, 0);
  } else {
    digitalWrite(led, 1);
    objRestServer.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    digitalWrite(led, 0);
  }
}

void handleForm() {
  if (objRestServer.method() != HTTP_POST) {
    digitalWrite(led, 1);
    objRestServer.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(led, 0);
  } else {
    digitalWrite(led, 1);
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    objRestServer.send(200, "text/plain", message);
    digitalWrite(led, 0);
  }
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
  objRestServer.send(404, "text/plain", message);
  digitalWrite(led, 0);
}