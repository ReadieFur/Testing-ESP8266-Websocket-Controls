#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
int ledUserControl = LED_BUILTIN;
int ledConnection = D1;
int ledNetProcess = D2;

int** StringToDoubleIntArray(std::string inputStr, std::string delimiter, int arr1Len = 0, int arr2Len = 0)
{
    arr1Len += 2;
    arr2Len++;
    inputStr = inputStr.substr(0, inputStr.length() - 2).substr(2);

    std::string tmpStr = inputStr;
    int pos;
    while ((pos = tmpStr.find(delimiter)) != std::string::npos) //While delimiter is found, if found output is 1
    {
        arr1Len++;
        tmpStr.erase(0, pos + delimiter.length()); //Remove section from string
    }
    tmpStr = inputStr; //We now have the number of arrays to make

    int** arr1 = new int* [arr1Len];
    for (int i = 0; i < arr1Len; ++i) { arr1[i] = new int[arr2Len]; } //Create a second array for all rows
    for (int i = 0; i < arr2Len; i++) { if (i == 0) { arr1[0][i] = arr1Len; } else { arr1[0][i] = NULL; } } //Keep a record of the array length

    int si = 1;
    while ((pos = tmpStr.find(delimiter)) != std::string::npos)
    {
        std::string j = tmpStr.substr(0, pos);
        int commaDelim = j.find(",");
        arr1[si][0] = String(j.substr(0, commaDelim).c_str()).toInt();
        arr1[si][1] = String(j.erase(0, commaDelim + 1).c_str()).toInt();

        tmpStr.erase(0, pos + delimiter.length());
        si++;
    }
    std::string j = tmpStr.substr(0, pos);
    int commaDelim = j.find(",");
    arr1[si][0] = String(j.substr(0, commaDelim).c_str()).toInt();
    arr1[si][1] = String(j.erase(0, commaDelim + 1).c_str()).toInt();

    return arr1;
    delete[] arr1; //Free up memory when finished with the array
}

void path(String location)
{
    digitalWrite(ledNetProcess, HIGH);
    if (location == "/")
    {
        //Website minified by my cpp.js file
        String page =
        "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>NODEMCU-V3 LED WEBSOCKET CONTROLS</title><style type='text/css'></style></head><body><header><h3>ARDUINO ESP8266 LED_BUILTIN CONTROLS</h3><h4>OVER HTTP/WS BY KOF.READIE</h4></header><hr><div>LED TOGGLE: <input type='checkbox' id='ledState'></div><hr><div><p>LED SEQUENCE:</p><p>Brightness: 0=on, 1023=off</p><p>Delay: duration of brightness in milliseconds (min=1, max=10000)</p><input type='number' min='0' max='1023' id='seqBright' placeholder='Brightness'><input type='number' min='1' max='10000' id='seqDelay' placeholder='Delay'><input type='button' id='addToSequence' value='Add to sequence'><input type='button' id='clearSequence' value='Clear sequence'><textarea id='sequence' readonly=''>[]</textarea><input type='button' id='sendSequence' value='Submit sequence'></div><hr><div><p>LED SLIDER:</p><input type='range' min='0' max='1023' value='512' id='sldBright'></div><hr><script> var ws; var ledState; window.addEventListener('load', () => { setupWebsocket(); ledState = document.querySelector('#ledState'); brightness = document.querySelector('#brightness'); ledState.addEventListener('click', ledCheckbox); document.querySelector('#addToSequence').addEventListener('click', addToSequence); document.querySelector('#clearSequence').addEventListener('click', clearSequence); document.querySelector('#sendSequence').addEventListener('click', sendSequence); document.querySelector('#sldBright').addEventListener('change', sldBright); document.querySelector('#sldBright').addEventListener('input', sldBrightVisual); }); function setupWebsocket() { ws = new WebSocket(`ws://${window.location.hostname}:81/`); function resetConnection(event) { setTimeout(() => { setupWebsocket(); }, 5000); } ws.onerror = resetConnection; ws.onclose = resetConnection; ws.addEventListener('message', (event) => { console.log(event.data); let message = event.data.split(','); message[0] = parseInt(message[0]); message[1] = parseInt(message[1]); ledState.checked = message[0] == 1023 ? false : true; let sldBright = document.querySelector('#sldBright'); sldBright.style.backgroundColor = `rgba(0, 75, 255, ${((parseInt(sldBright.value = 1023 - message[0]) * 100) / 1023)/100})`; }); } function ledCheckbox() { ledState.checked = !ledState.checked; ws.send(JSON.stringify([[ledState.checked ? 1023 : 0, 0]])); } var sequenceValue = []; function addToSequence() { let seqBright = document.querySelector('#seqBright'); let seqDelay = document.querySelector('#seqDelay'); let seqBrightVal = Math.round(parseInt(seqBright.value)); let seqDelaytVal = Math.round(parseInt(seqDelay.value)); if (seqBrightVal > 1023) { seqBrightVal = 1023; } else if (seqBrightVal < 0) { seqBrightVal = 0; } if (seqDelaytVal > 10000) { seqDelaytVal = 10000; } else if (seqDelaytVal < 1) { seqDelaytVal = 1; } sequenceValue.push([seqBrightVal, seqDelaytVal]); document.querySelector('#sequence').value = JSON.stringify(sequenceValue); seqBright.value = ''; seqDelay.value = ''; } function clearSequence() { sequenceValue = []; document.querySelector('#sequence').value = '[]'; } function sendSequence() { if (sequenceValue.length != 0) { ws.send(JSON.stringify(sequenceValue)); } } function sldBright() { ws.send(JSON.stringify([[1023 - parseInt(document.querySelector('#sldBright').value), 0]])); } function sldBrightVisual() { let sldBright = document.querySelector('#sldBright'); sldBright.style.backgroundColor = `rgba(0, 75, 255, ${((parseInt(sldBright.value) * 100) / 1023)/100})`; } </script><style> #sequence { min-width: 100%; max-width: 100%; min-height: 50px; } #sldBright { -webkit-appearance: none; border: 1px solid black; } </style><footer></footer></body>"
        ;
        server.send(200, "text/html", page);
    }
    else
    {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";
        for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
        server.send(404, "text/plain", message);
    }
    digitalWrite(ledNetProcess, LOW);
}

String ledAnalog = "1023";
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
    digitalWrite(ledNetProcess, HIGH);
    std::string message;
    for (int i = 0; i < length; i++) { message += (char) payload[i]; }
    if (message == "/")
    {
        //String ledAnalog = String(analogRead(ledControl)); //Not working
        Serial.println("WS sending new client (" + webSocket.remoteIP(num).toString() + ") current LED status of: " + ledAnalog + ",0");
        webSocket.sendTXT(num, ledAnalog + ",0"); //Send new client current led status
    }
    else if (type == WStype_TEXT)
    {
        Serial.println("WS recieved: " + String(message.c_str()));
        bool resetLED = false;
        int** arr1 = StringToDoubleIntArray(message, "],[", 0, 2);
        digitalWrite(ledNetProcess, LOW);
        for (int i = 1; i < arr1[0][0]; i++)
        {
            digitalWrite(ledNetProcess, HIGH);
            String arr1Str = String(arr1[i][0]), arr2Str = String(arr1[i][1]);
            Serial.println("Execute: Brightness(" + arr1Str + ") Time(" + arr2Str + ")");
            webSocket.broadcastTXT(arr1Str + "," + arr2Str);
            analogWrite(ledUserControl, arr1[i][0]);
            ledAnalog = arr1Str;
            digitalWrite(ledNetProcess, LOW);
            if (arr1[i][1] == 0) { break; }
            else { delay(arr1[i][1]); resetLED = true; }
        }
        if (resetLED)
        {
            digitalWrite(ledNetProcess, HIGH);
            digitalWrite(ledUserControl, HIGH);
            webSocket.broadcastTXT("1023,0");
            ledAnalog = "1023";
            digitalWrite(ledNetProcess, LOW);
        }
    }
    digitalWrite(ledNetProcess, LOW);
}

void setup()
{
    pinMode(ledUserControl, OUTPUT);
    pinMode(ledConnection, OUTPUT);
    pinMode(ledNetProcess, OUTPUT);
    digitalWrite(ledUserControl, HIGH);
    digitalWrite(ledConnection, LOW);
    digitalWrite(ledNetProcess, LOW);
    WiFi.begin("BT Upstairs Internet", "BTUpstairs"); //Use SoftAP when done
    Serial.begin(115200);
    Serial.println();
    Serial.print("Connecting");
    while(WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(500); } //Wait for connection
    Serial.print("\nLocal IP Address: ");
    Serial.println(WiFi.localIP());
    WiFi.softAP("ESP8266_NodeMCU-V3", "_ESP8266");

    server.on("/", [](){ path("/"); });
    server.onNotFound([](){ path(""); });
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    digitalWrite(ledConnection, HIGH);
}

void loop()
{
    server.handleClient();
    webSocket.loop();
}