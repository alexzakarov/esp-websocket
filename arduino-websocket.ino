#include "FS.h"
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

// Constants
const char *ssid = "ESP32-AP";
const char *password =  "LetMeInPlz";
const char *msg_toggle_led = "toggleLED";
const char *msg_get_led = "getLEDState";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
const int motorForward = 0;
Servo servo;
// Globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
int mot_state = 0;
int servo_state = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>

<h2>LED Control</h2>

<table>
    <tr>
        <td><button id="toggleButton" onclick="onPress()" disabled>Toggle LED</button></td>
        <td><canvas id="led" width="50" height="50"></canvas></td>
    </tr>
</table>

<div id="output"></div>
      
      
      
<script>

var url = "ws://192.168.4.1:1337/";
var output;
var button;
var canvas;
var context;

// This is called when the page finishes loading
function init() {

    // Assign page elements to variables
    button = document.getElementById("toggleButton");
    output = document.getElementById("output");
    canvas = document.getElementById("led");
    
    // Draw circle in canvas
    context = canvas.getContext("2d");
    context.arc(25, 25, 15, 0, Math.PI * 2, false);
    context.lineWidth = 3;
    context.strokeStyle = "black";
    context.stroke();
    context.fillStyle = "black";1
    context.fill();
    
    // Connect to WebSocket server
    wsConnect(url);
}

// Call this to connect to the WebSocket server
function wsConnect(url) {
    
    // Connect to WebSocket server
    websocket = new WebSocket(url);
    
    // Assign callbacks
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
}

// Called when a WebSocket connection is established with the server
function onOpen(evt) {

    // Log connection state
    console.log("Connected");
    
    // Enable button
    button.disabled = false;
    
    // Get the current state of the LED
    doSend("getLEDState");
}

// Called when the WebSocket connection is closed
function onClose(evt) {

    // Log disconnection state
    console.log("Disconnected");
    
    // Disable button
    button.disabled = true;
    
    // Try to reconnect after a few seconds
    setTimeout(function() { wsConnect(url) }, 2000);
}

// Called when a message is received from the server
function onMessage(evt) {

    // Print out our received message
    console.log("Received: " + evt.data);
    
    // Update circle graphic with LED state
    switch(evt.data) {
        case "0":
            console.log("LED is off");
            context.fillStyle = "black";
            context.fill();
            break;
        case "1":
            console.log("LED is on");
            context.fillStyle = "red";
            context.fill();
            break;
        default:
            break;
    }
}

// Called when a WebSocket error occurs
function onError(evt) {13
    console.log("ERROR: " + evt.data);
}

// Sends a message to the server (and prints it to the console)
function doSend(message) {
    console.log("Sending: " + message);
    websocket.send(message);
}

// Called whenever the HTML button is pressed
function onPress() {
    doSend("toggleLED");
    doSend("getLEDState");
}

// Call the init function as soon as the page loads
window.addEventListener("load", init, false);

</script>

</body>
</html>
)rawliteral";



const char style_css[] PROGMEM = R"rawliteral(
  .body {
    align:center;
  }
)rawliteral";

/***********************************************************
 * Functions
 */

// Callback: receiving any WebSocket message
void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {

  // Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());
      }
      break;

    // Handle text messages from client
    case WStype_TEXT:

      // Print out raw message
      Serial.printf("[%u] Received text: %s\n", client_num, payload);

      // Motor Forward 
      if ( strcmp((char *)payload, "forward") == 0 ) {
        Serial.printf("Toggling Forward to %u\n", 255);
        analogWrite(motorForward, 255);

      } else if ( strcmp((char *)payload, "stop") == 0 ) {
    Serial.printf("Toggling Forward to %u\n", 0);
        analogWrite(motorForward, 0);

      }  else if ( strcmp((char *)payload, "servoLeft") == 0 ) {
        Serial.printf("Toggling Servo to %u\n", 0);
        servo.write(0);

      } else if ( strcmp((char *)payload, "servoRight") == 0 ) {
        Serial.printf("Toggling Servo to %u\n", 180);
        servo.write(180);

      }  else if ( strcmp((char *)payload, "servoNormal") == 0 ) {
        Serial.printf("Toggling Servo to %u\n", 90);
        servo.write(90);

      } else if ( strcmp((char *)payload, "getLEDState") == 0 ) {
        sprintf(msg_buf, "%d", mot_state);
        Serial.printf("Sending to [%u]: %s\n", client_num, msg_buf);
        webSocket.sendTXT(client_num, msg_buf);

      // Message not recognized
      } else {
        Serial.println("[%u] Message not recognized");
      }
      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
    request->send_P(200, "text/html", index_html);
}

// Callback: send style sheet
void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
    request->send_P(200, "text/css", style_css);

}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  request->send(404, "text/plain", "Not found");
}

/***********************************************************
 * Main
 */

void setup(){

  // Init LED and turn off
  pinMode(motorForward, OUTPUT);
  digitalWrite(motorForward, LOW);
  servo.attach(2);
  // Start Serial port
  Serial.begin(115200);


  // Start access point
   if (!WiFi.softAP(ssid, password)) {
    Serial.println("Soft AP creation failed.");
    while(1);
  }
  // Print our IP address
  Serial.println();
  Serial.println("AP running");
  Serial.print("My IP address: ");
  Serial.println(WiFi.softAPIP());

  // On HTTP request for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);
 
  // On HTTP request for style sheet, provide style.css
  server.on("/style.css", HTTP_GET, onCSSRequest);

  // Handle requests for pages that do not exist
  server.onNotFound(onPageNotFound);

  // Start web server
  server.begin();

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  
  // Look for and handle WebSocket data
  webSocket.loop();
}