// iBoardbot project
// WifiESP.ino
// Wifi and ESP related functions
// Author: jjrobots 2016

// Read WIFI config from EEPROM
void readWifiConfig()
{
  EEPROM.get(0, WifiConfig);
}

// Write WIFI config to EEPROM
void writeWifiConfig(uint8_t status, char ssid[30], char pass[30], char proxy[30], unsigned int port)
{
  WifiConfig.status = status;  // Status=1 -> configured
  strcpy(WifiConfig.ssid, ssid);
  strcpy(WifiConfig.pass, pass);
  strcpy(WifiConfig.proxy, proxy);
  WifiConfig.port = port;
  EEPROM.put(0, WifiConfig);
}

void WifiConfigurationMode()
{
  delay(5000);
  Serial.println(F("Wifi Configuration mode..."));
  ESPflush();
  Serial1.println(F("AT+RST"));
  ESPwaitFor("ready", 10);

  //Serial1.println(F("AT+CWMODE=1"));   // Station mode
  //ESPwaitFor("OK", 3);
  //Serial1.println(F("AT+CWLAP"));
  //ESPwaitFor("OK",3);
  //Serial1.println(F("AT+CWLAP"));
  //ESPgetAPlist();

  delay(50);
  Serial1.println(F("AT+CWMODE=2"));   // Soft AP mode
  ESPwaitFor("OK", 3);
  Serial1.println(F("AT+CIPSTAMAC?"));
  ESPgetMac(MAC);
  Serial1.println(F("AT+CWSAP=\"JJROBOTS_IBB\",\"87654321\",5,3"));
  ESPwaitFor("OK", 6);
  Serial1.println(F("AT+CIPMUX=1"));
  ESPwaitFor("OK", 3);
  Serial1.println(F("AT+CIPSERVER=1,80"));
  ESPwaitFor("OK", 4);

  Serial.println();
  Serial.println(F("Instructions:"));
  Serial.println(F("->Connect to JJROBOTS_IBB wifi network, password: 87654321"));
  Serial.println(F("->Open page: http://192.168.4.1"));
  Serial.println(F("->Fill SSID and PASSWORD of your Wifi network and press SEND!"));
  // Show web server configuration page until user introduce the configuration
  ESPwebServerConfig();
  delay(500);
  ESPflush();
  WifiConfig.status = 1;
  // Store on EEPROM!
  // Default Host and URL
  EEPROM.put(0, WifiConfig);
  Serial.println(F("  Configured!!"));
  Serial.print(F("SSID:"));
  Serial.println(WifiConfig.ssid);
  Serial.print(F("PASS :"));
  Serial.println(WifiConfig.pass);
  Serial.println(F("HOST : "));
  Serial.println(SERVER_HOST);
  Serial.println(F("URL : "));
  Serial.println(SERVER_URL);
  Serial.print(F("PROXY : "));
  Serial.println(WifiConfig.proxy);
  Serial.print(F("PORT : "));
  Serial.println(WifiConfig.port);


  Serial1.println(F("AT+CWQAP"));
  ESPwaitFor("OK", 5);
  Serial1.println(F("AT+CWMODE=1"));   // Station mode
  ESPwaitFor("OK", 3);
  Serial1.println(F("AT+RST"));
  ESPwaitFor("ready", 10);
  delay(1000);
}

void GetMac()
{
  Serial1.println(F("AT+CIPSTAMAC?"));
  ESPgetMac(MAC);
  Serial.print(F("MAC:"));
  Serial.println(MAC);
}

bool WifiConnect()
{
  Serial1.println(F("AT+CWQAP"));
  ESPwaitFor("OK", 5);
  delay(1000);
  Serial1.println(F("AT+CWMODE=1"));   // Station mode
  ESPwaitFor("OK", 3);
  delay(1500);
  Serial.println(F("Connecting to Wifi network..."));
  Serial1.print(F("AT+CWJAP=\""));
  Serial1.print(WifiConfig.ssid);
  Serial1.print("\",\"");
  Serial1.print(WifiConfig.pass);
  Serial1.println("\"");
  if (ESPwaitFor2("OK", "DISCO", 14) == 1)
    return true;
  else
    return false;
}

// ESP functions...
void ESPwait(int timeout_secs)
{
  char c;
  long timer_init;
  uint8_t timeout = 0;

  timer_init = millis();
  while (!timeout) {
    if (((millis() - timer_init) / 1000) > timeout_secs) { // Timeout?
      timeout = 1;
    }
    if (Serial1.available()) {
      c = Serial1.read();
      //Serial.print(c);
    }
  }
}

void ESPflush()
{
  char ch_aux;
  // Serial flush
  Serial1.flush();
  while (Serial1.available() > 0)
    ch_aux = Serial1.read();
}

// Read a new char and rotate buffer (5 char buffer)
uint8_t ESPreadChar(char c[5])
{
  if (Serial1.available()) {
    c[4] = c[3];
    c[3] = c[2];
    c[2] = c[1];
    c[1] = c[0];
    c[0] = Serial1.read();
    Serial.print(c[0]);
    return 1;
  }
  else
    return 0;
}

// Wait for response (max 5 characters)
uint8_t ESPwaitFor(const char *stopstr, int timeout_secs)
{
  char c[5];
  bool found = false;
  long timer_init;

  timer_init = millis();
  while (!found) {
    if (((millis() - timer_init) / 1000) > timeout_secs) { // Timeout?
      Serial.println(F("!Timeout!"));
      return 0;  // timeout
    }
    ESPreadChar(c);
    uint8_t stopstrSize = strlen(stopstr);
    if (stopstrSize > 5)
      stopstrSize = 5;
    found = true;
    for (uint8_t i = 0; i < stopstrSize; i++) {
      if (c[i] != stopstr[stopstrSize - 1 - i]) {
        found = false;
        break;
      }
    }
    if (found) {
      ESPflush();
      Serial.println();
    }
  } // end while (!found)
  //delay(250);
  return 1;
}

// Wait for response (max 5 characters)
uint8_t ESPwaitFor2(const char *stopstr, const char *stopstr2, int timeout_secs)
{
  char c[5];
  uint8_t found = 0;
  long timer_init;
  uint8_t stopstrSize;

  timer_init = millis();
  while (found == 0) {
    if (((millis() - timer_init) / 1000) > timeout_secs) { // Timeout?
      Serial.println(F("!Timeout!"));
      return 0;  // timeout
    }
    ESPreadChar(c);
    stopstrSize = strlen(stopstr);
    if (stopstrSize > 5)
      stopstrSize = 5;
    found = 1;
    for (uint8_t i = 0; i < stopstrSize; i++) {
      if (c[i] != stopstr[stopstrSize - 1 - i]) {
        found = 0;
        break;
      }
    }
    if (found == 0) {
      stopstrSize = strlen(stopstr2);
      if (stopstrSize > 5)
        stopstrSize = 5;
      found = 2;
      for (uint8_t i = 0; i < stopstrSize; i++) {
        if (c[i] != stopstr2[stopstrSize - 1 - i]) {
          found = 0;
          break;
        }
      }
    }
    if (found > 0) {
      delay(20);
      ESPflush();
      Serial.println();
    }
  } // end while (!found)
  //delay(250);
  return found;
}


// getMacAddress from ESP wifi module
uint8_t ESPgetMac(char *MAC)
{
  char c1, c2;
  bool timeout = false;
  long timer_init;
  uint8_t state = 0;
  uint8_t index = 0;

  timer_init = millis();
  while (!timeout) {
    if (((millis() - timer_init) / 1000) > 4) // Timeout 4 seconds
      timeout = true;
    if (Serial1.available()) {
      c2 = c1;
      c1 = Serial1.read();
      Serial.print(c1);
      switch (state) {
        case 0:
          if (c1 == ':')
            state = 1;
          break;
        case 1:
          if (c1 == '\r')
            state = 2;
          else {
            if ((c1 != '"') && (c1 != ':')) {
              if (index < 12)
                MAC[index++] = toupper(c1);  // Uppercase
            }
          }
          break;
        case 2:
          if ((c2 == 'O') && (c1 == 'K')) {
            Serial.println();
            Serial1.flush();
            MAC[12] = '\0';
            return 1;  // Ok
          }
          break;
      } // end switch
    } // Serial_available
  } // while (!timeout)
  Serial.println(F("!Timeout!"));
  Serial1.flush();
  return -1;  // timeout
}

// Wait for a response message example: +IPD,2:OK  or +IPD,768:af4aedqead...
// It detects the message using the code1:4009 and detects the end because the connection close
// Return the size of the message (-1: timeout, 2: OK, <=768: data packet)
// Function return: 0 reading message 1: message readed 2: timeout
uint8_t ESPwaitforMessage(uint8_t timeout_secs)
{
  char ch;

  if (((millis() - message_timer_init) / 1000) > timeout_secs) {
    Serial.print(F("!Timeout"));
    message_readed = 2; // timeout
    return 2;
  }
  while (Serial1.available()) {
    ch = Serial1.read();
    mc7 = mc6; mc6 = mc5; mc5 = mc4; mc4 = mc3; mc3 = mc2; mc2 = mc1; mc1 = mc0; mc0 = ch; // Hardcoding this rotate buffer is more efficient
    //Serial.print(" ");
    //Serial.println(ch);
    if ((mc5 == 'C') && (mc4 == 'L') && (mc3 == 'O') && (mc2 == 'S') && (mc1 == 'E') && (mc0 == 'D')) {
      Serial.println(F("CCLOSED!"));
      Serial1.flush();
      message_readed = 1;
      if (message_index_buffer > 0)
        message_size = message_index_buffer;
      else {
        // In case of no packet, we return the last two characters of the message (OK,ER...)
        message_size = 2;
        buffer[0] = mc7;
        buffer[1] = mc6;
      }
      return 1;
    }
    // State machine
    switch (message_status) {
      case 0:
        // Waiting for +IPD,...
        if ((mc4 == '+') && (mc3 == 'I') && (mc2 == 'P') && (mc1 == 'D') && (mc0 == ',')) {
          message_chunked = false;
          message_size = 0;
          message_index = 0;
          message_index_buffer = 0;
          message_status = 1;
        }
        break;
      case 1:
        // Reading message size
        if (ch == ':') {
          Serial.print("SIZE:");
          Serial.println(message_size);
          if (message_size > 1460) {
            Serial.println(F("!PACKET_SIZE_ERROR"));
            return -1;
          }
          message_status = 2;
        }
        else
          message_size = message_size * 10 + int(ch - '0');
        break;
      case 2:
        message_index++;
        if (message_index >= message_size) {
          Serial.println("END");
          message_status = 0;
          break;
        }
        // Detecting packet start 4009 4001 (FA9,FA1)(mc2=0xFA,mc1=0x9F,mc0=0xA1)
        if ((uint8_t(mc2) == 0b11111010) && (uint8_t(mc1) == 0b10011111) && (uint8_t(mc0) == 0b10100001)) {
          Serial.println(F("Packet start!"));
          buffer[0] = mc2;
          buffer[1] = mc1;
          buffer[2] = mc0;
          message_index_buffer = 3;
          message_status = 3;
        }
        break;
      case 3:
        message_index++;
        if (message_index > message_size) {
          Serial.println("END");
          message_status = 0;
          break;
        }
        // Reading packet
        if (message_index_buffer < MAX_PACKET_SIZE) {
          //Serial.print(ch);
          buffer[message_index_buffer] = ch;
          message_index_buffer++;
        }
        else {
          Serial.println(F("Error: message too large!"));
          return 2;  // Error
        }
        break;
    }
  }
  return 0;  // Reading...
}



// This function sends an http GET request to an url
// it uses the configuration stored on WifiConfig global variable
uint8_t ESPsendHTTP(char *url)
{
  char cmd_get[160];
  char cmd_send[15];
  char strSize[6];
  char strPort[6];

  //Serial.print("Free RAM:");
  //Serial.println(freeRam());
  // Open TCP connection on port 80
  strcpy(cmd_get, "AT+CIPSTART=\"TCP\",\"");
  if ((WifiConfig.port > 0) && (WifiConfig.port < 65000) && (strlen(WifiConfig.proxy) > 0)) { // Connection with proxy?
    strcat(cmd_get, WifiConfig.proxy);
    strcat(cmd_get, "\",");
    sprintf(strPort, "%d", WifiConfig.port);
    strcat(cmd_get, strPort);
  }
  else {  // Standard HTTP connection (direct to host on port 80)
    strcat(cmd_get, SERVER_HOST);
    strcat(cmd_get, "\",80");
  }
  Serial1.println(cmd_get);
  if (ESPwaitFor("OK", 5))
  {
    strcpy(cmd_get, "GET ");
    strcat(cmd_get, url);
    strcat(cmd_get, " HTTP/1.1\r\nHost:");
    strcat(cmd_get, SERVER_HOST);
    strcat(cmd_get, "\r\nConnection: close\r\n\r\n");
    sprintf(strSize, "%d", strlen(cmd_get));
    strcpy(cmd_send, "AT+CIPSEND=");
    strcat(cmd_send, strSize);
    Serial1.println(cmd_send);
    ESPwaitFor(">", 3);
    Serial1.println(cmd_get);
    //Serial.print("SEND:");
    //Serial.println(cmd_get);
    ESPwaitFor("OK", 5);
    return 1;
  }
  else {
    digitalWrite(13,LOW);
    Serial.println(F("Connection error"));
    Serial1.println("AT+CIFSR");
    ESPwaitFor("OK", 5);
    Serial1.println("AT+CIPCLOSE");
    ESPwaitFor("OK", 5);
    delay(4000);  // delay on error...
    digitalWrite(13,HIGH);
    return 0;
  }
}


// Extract parameter from GET string  ?param1=xxx&param2=yyy
// return value: 0 while extracting, 1: Extracted OK, 2: Error
// First time: param should be initialized ('\0') externally
uint8_t ESPwebServerExtractParam(char *param, char ch)
{
  static uint8_t index;

  if ((ch == '\n') || (ch == ' ') || (ch == '&')) { // End of param?
    index = 0;
    return 1;
  }

  if ((index == 0) && (ch == '=')) { // end of param name?
    index = 1;  // increase index to know that we are reading the parameter
  }
  else if (index > 0) {    // extracting param...
    param[index - 1] = ch;
    param[index] = '\0';
    index++;
    if (index >= WIFICONFIG_MAXLEN) {
      Serial.println("Error!:Param too large");
      index = 0;
      return 2;   // Error: param too large!
    }
  }
  return 0;
}


// Mini WEB SERVER to CONFIG the WIFI parameters: SSID, passeword and optionally proxy and port
// If the server receive an url with parameters: decode it and store on EEPROM
// If the server receive any other thing: show the wifi config form page
void ESPwebServerConfig()
{
  char ch;
  uint8_t tcp_ch = 0;
  uint8_t result;
  bool configured = false;
  uint8_t webserver_status = 0;
  char user_param[WIFICONFIG_MAXLEN];

  while (!configured) {
    // Led blink on wifi config...
    if ((millis() / 100) % 2 == 0)
      digitalWrite(13, HIGH);
    else
      digitalWrite(13, LOW);
    while (Serial1.available()) {
      ch = Serial1.read();
      Serial.print(ch);
      // State machine
      switch (webserver_status) {
        case 0:
          // Waiting for +IPD,...
          mc4 = mc3; mc3 = mc2; mc2 = mc1; mc1 = mc0; mc0 = ch; // Hardcoding this rotate buffer is more efficient
          if ((mc4 == '+') && (mc3 == 'I') && (mc2 == 'P') && (mc1 == 'D') && (mc0 == ',')) {
            webserver_status = 1;
          }
          break;
        case 1:
          // Read channel
          tcp_ch = ch - '0';
          //Serial.print("->ch:");
          //Serial.println(tcp_ch);
          webserver_status = 2;
        case 2:
          // Check the page requested (/ or /config)
          // If we found an "?" on the first line then this is the configuration page
          if (ch == '\n')
            webserver_status = 3;
          if (ch == 'v')
            webserver_status = 9;
          if (ch == '?') {
            user_param[0] = '\0';
            webserver_status = 4;
          }
          break;
        case 3:
          // Webserver root => Show configuration page
          Serial.println();
          Serial.println("->Config page");
          delay(20);
          ESPflush();
          ESPconfigWeb(tcp_ch);
          delay(20);
          ESPflush();
          delay(50);
          ESPflush();
          webserver_status = 0;
          break;
        case 9:
          // Webserver root => Show configuration page
          Serial.println();
          Serial.println("->Advanced page");
          delay(20);
          ESPflush();
          ESPconfigWeb_advanced(tcp_ch);
          delay(20);
          ESPflush();
          delay(50);
          ESPflush();
          webserver_status = 0;
          break;
        case 4:
          Serial.println("->P1");
          result = ESPwebServerExtractParam(user_param, ch);
          if (result == 1) { // Ok => extraemos siguiente parametro
            urldecode2(WifiConfig.ssid, user_param);
            user_param[0] = '\0';
            webserver_status = 5;
          }
          if (result == 2) // Error => Show config page again...
            webserver_status = 3;
          break;
        case 5:
          Serial.println("->P2");
          result = ESPwebServerExtractParam(user_param, ch);
          if (result == 1) { // Ok => extraemos siguiente parametro
            urldecode2(WifiConfig.pass, user_param);
            user_param[0] = '\0';
            webserver_status = 6;
          }
          if (result == 2) // Error => Show config page again...
            webserver_status = 3;
          break;
        case 6:
          Serial.println("->P3");
          result = ESPwebServerExtractParam(user_param, ch);
          if (result == 1) { // Ok => extraemos siguiente parametro
            urldecode2(WifiConfig.proxy, user_param);
            Serial.print("->proxy=");
            Serial.println(user_param);
            user_param[0] = '\0';
            webserver_status = 7;
          }
          if (result == 2) // Error => Show config page again...
            webserver_status = 3;
          break;
        case 7:
          Serial.println("->P4:");
          result = ESPwebServerExtractParam(user_param, ch);
          if (result == 1) { // Ok => extraemos siguiente parametro
            if (strlen(user_param) > 0)
              WifiConfig.port = atoi(user_param);
            else
              WifiConfig.port = 0;
            //urldecode2(WifiConfig.pass, user_param);
            Serial.print("->port=");
            Serial.println(user_param);
            user_param[0] = '\0';
            webserver_status = 8;
          }
          if (result == 2) // Error => Show config page again...
            webserver_status = 3;
          break;
        case 8:
          ESPflush();
          delay(50);
          ESPflush();
          ESPconfigWebOK(tcp_ch);   // OK webpage to user
          delay(100);
          ESPflush();
          configured = true;
          break;
        default:
          webserver_status = 3;
      }  // end switch(webserverstatus)
    }  // while serial.available
  } // while !configured
}

// Urldecode function from https://gist.github.com/jmsaavedra
// Updated to decode '+' as spaces (typical on form parameters)
void urldecode2(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    }
    else {
      if (*src == '+') {
        *dst++ = ' ';    // whitespaces encoded '+'
        src++;
      }
      else
        *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

// HTML Page to Config Wifi parameters: SSID, password and optional: proxy and port
// We use html static pages with precalculated sizes (memory optimization)
void ESPconfigWeb(uint8_t tcp_ch)
{
  Serial1.print("AT+CIPSEND=");
  Serial1.print(tcp_ch);
  Serial1.println(",573");   // Header length: 84 Content length: 489 = 573
  ESPwaitFor(">", 3);
  Serial1.println(F("HTTP/1.1 200 OK")); //15+2
  Serial1.println(F("Content-Type: text/html")); //23+2
  Serial1.println(F("Connection: close")); //17+2
  Serial1.println(F("Content-Length: 489")); //19+2
  Serial1.println();  //2  => total: 17+25+19+21+2 = 84
  delay(10);
  Serial1.print(F("<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width'>")); //78
  Serial1.print(F("<body><h3>Wifi Configuration Page</h3><form method='get' action='config'>")); //73
  Serial1.print(F("<label>SSID:</label><br><input type='text' name='ssid' size='30'><br>")); //69
  Serial1.print(F("<label>Password:</label><br><input type='password' name='password' size='30'><br>")); //81
  Serial1.print(F("<br><a href='av'>advanced</a><input hidden type='text' name='proxy' value=''><input hidden type='text' name='port' value=''>")); //124
  Serial1.print(F("<br><br><input type='submit' value='SEND!'></form></body></html>")); //60
  // Total=78+73+69+81+124+64= 489
  ESPwaitFor("OK", 5);
  delay(100);
  // Close the connection from server side (safety)
  //Serial.println("->CIPCLOSE");
  Serial1.print("AT+CIPCLOSE=");
  Serial1.println(tcp_ch);
  Serial.print("Free RAM:");
  Serial.println(freeRam());
}

// HTML Page to Config Wifi parameters: SSID, password and optional: proxy and port
// We use html static pages with precalculated sizes (memory optimization)
void ESPconfigWeb_advanced(uint8_t tcp_ch)
{
  Serial1.print("AT+CIPSEND=");
  Serial1.print(tcp_ch);
  Serial1.println(",573");   // Header length: 84 Content length: 489 = 573
  ESPwaitFor(">", 3);
  Serial1.println(F("HTTP/1.1 200 OK")); //15+2
  Serial1.println(F("Content-Type: text/html")); //23+2
  Serial1.println(F("Connection: close")); //17+2
  Serial1.println(F("Content-Length: 489")); //19+2
  Serial1.println();  //2  => total: 17+25+19+21+2 = 84
  delay(10);
  Serial1.print(F("<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width'>")); //78
  Serial1.print(F("<body><h3>Wifi Configuration Page</h3><form method='get' action='config'>")); //73
  Serial1.print(F("<label>SSID:</label><br><input type='text' name='ssid' size='30'><br>")); //69
  Serial1.print(F("<label>Password:</label><br><input type='password' name='password' size='30'><br>")); //81
  Serial1.print(F("<i><h5>OPTIONAL:</h5>Proxy:<input type='text' name='proxy' size='20'>&nbsp;port:<input type='text' name='port' size='6'></i>")); //124
  Serial1.print(F("<br><br><input type='submit' value='SEND!'></form></body></html>")); //60
  // Total=78+73+69+81+124+64= 489
  ESPwaitFor("OK", 5);
  delay(100);
  // Close the connection from server side (safety)
  //Serial.println("->CIPCLOSE");
  Serial1.print("AT+CIPCLOSE=");
  Serial1.println(tcp_ch);
  Serial.print("Free RAM:");
  Serial.println(freeRam());
}


// HTML page showing that Wifi is configured OK
// This page has a button to continue the wizard
void ESPconfigWebOK(uint8_t tcp_ch)
{
  Serial1.print("AT+CIPSEND=");
  Serial1.print(tcp_ch);
  Serial1.println(",388");   // Header length: 84 Content length: 304 = 388
  ESPwaitFor(">", 3);
  Serial1.println(F("HTTP/1.1 200 OK")); //15+2
  Serial1.println(F("Content-Type: text/html")); //23+2
  Serial1.println(F("Connection: close")); //17+2
  Serial1.println(F("Content-Length: 304")); //19+2
  Serial1.println();  //2  => total: 17+25+19+21+2 = 84
  delay(10);

  Serial1.print(F("<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width, user-scalable=no'>")); //96
  Serial1.print(F("<body><h3>ID:&nbsp")); //18
  Serial1.print(MAC); //12
  Serial1.print(F("</h3><h2>Wifi Configured!</h2><button onclick=\"location.href='http:\/\/ibbapp.jjrobots.com/wizard/wizard2.php?ID_IWBB=")); //112
  Serial1.print(MAC); //12
  Serial1.print(F("';\">Go to Test and Registration</button></body></html>")); //54
  // total = 96+18+12+112+12+54 = 304

  //Serial.println("->Wait OK");
  ESPwaitFor("OK", 5);
  //Serial.println("->OK");
  delay(100);
  // Close the connection from server side (safety)
  Serial.println("->CIPCLOSE");
  Serial1.print("AT+CIPCLOSE=");
  Serial1.println(tcp_ch);
  Serial.print("Free RAM:");
  Serial.println(freeRam());
}


