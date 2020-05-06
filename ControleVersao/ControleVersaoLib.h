/*******************************************************************************
  BIBLIOTECAS E ARQUIVOS AUXILIARES
*******************************************************************************/
#include "ControleVersaoDef.h"

/*******************************************************************************
  FUNÇÕES
*******************************************************************************/
String ipStr(const IPAddress &ip) {
  // Retorna IPAddress como "n.n.n.n"
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

void hold(const unsigned int &ms) {
  // Delay
  unsigned long m = millis();
  while (millis() - m < ms) {
    yield();
  }
}

time_t iso8601DateTime(String s, const int8_t tz = 0) {
  // Retorna ISO8601 como time_t
  TimeElements te;
  te.Year   = s.substring( 0,  4).toInt() - 1970;
  te.Month  = s.substring( 5,  7).toInt();
  te.Day    = s.substring( 8, 10).toInt();
  te.Hour   = s.substring(11, 13).toInt();
  te.Minute = s.substring(14, 16).toInt();
  te.Second = s.substring(17, 19).toInt();
  time_t t  = makeTime(te);
  t -= s.substring(19, 22).toInt() * 3600; // Adjusta para UTC;
  t += tz * 3600;                         // Adjusta para hora local
  return t;
}

String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true) {
  // Retorna time_t como "yyyy-mm-dd hh:mm:ss" ou "dd/mm/yyyy hh:mm:ss"
  if (timeStatus() == timeNotSet) {
    return flBr ? F("00/00/0000 00:00:00") : F("0000-00-00 00:00:00");
  }
  t += tz * 3600; // Adjusta TimeZone
  String s;
  if (flBr) {
    // dd/mm/yyyy hh:mm:ss
    s = "";
    if (day(t) < 10) {
      s += '0';
    }
    s += String(day(t)) + '/';
    if (month(t) < 10) {
      s += '0';
    }
    s += String(month(t)) + '/' + String(year(t)) + ' ';
  } else {
    // yyyy-mm-dd hh:mm:ss
    s = String(year(t)) + '-';
    if (month(t) < 10) {
      s += '0';
    }
    s += String(month(t)) + '-';
    if (day(t) < 10) {
      s += '0';
    }
    s += String(day(t)) + ' ';
  }
  if (hour(t) < 10) {
    s += '0';
  }
  s += String(hour(t)) + ':';
  if (minute(t) < 10) {
    s += '0';
  }
  s += String(minute(t)) + ':';
  if (second(t) < 10) {
    s += '0';
  }
  s += String(second(t));
  return s;
}

void log(const String &s) {
  // Gera log na Serial
  Serial.println(dateTimeStr(now(), TIMEZONE) + " " + s);
}

// Função de Atualização de Data/Hora -------------------
time_t timeNTP() {
  // Retorna time_t do Servidor NTP
  if (WiFi.status() != WL_CONNECTED) {
    // Sem conexão WiFi
    return 0;
  }

  // Servidor NTP
  const char  NTP_SERVER[]    = "pool.ntp.br";

  const byte  NTP_PACKET_SIZE = 48;
  const int   UDP_LOCALPORT   = 2390;
  byte        ntp[NTP_PACKET_SIZE];
  memset(ntp, 0, NTP_PACKET_SIZE);
  ntp[ 0] = 0b11100011;
  ntp[ 1] = 0;
  ntp[ 2] = 6;
  ntp[ 3] = 0xEC;
  ntp[12] = 49;
  ntp[13] = 0x4E;
  ntp[14] = 49;
  ntp[15] = 52;
  WiFiUDP udp;
  udp.begin(UDP_LOCALPORT);
  udp.beginPacket(NTP_SERVER, 123);
  udp.write(ntp, NTP_PACKET_SIZE);
  udp.endPacket();
  hold(2000);
  unsigned long l;
  if (udp.parsePacket()) {
    // Sucesso
    udp.read(ntp, NTP_PACKET_SIZE);
    l = word(ntp[40], ntp[41]) << 16 | word(ntp[42], ntp[43]);
    l -= 2208988800UL;
  } else {
    //Erro
    l = 0;
  }
  // Retorna Data/Hora
  return l;
}

/*******************************************************************************
  FUNÇÕES DE VERSÃO
*******************************************************************************/
String platform() {
  // Obtem a plataforma de hardware
#ifdef ARDUINO_ESP32_DEV
  // ESP32
  return "esp32";
#elif defined ARDUINO_ESP8266_GENERIC
  // ESP8266 Genérico
  return "esp8266";
#elif defined ARDUINO_ESP8266_NODEMCU
  // ESP8266 NodeMCU
  return "nodemcu";
#elif defined ARDUINO_ESP8266_ESP01
  // ESP8285
  return "esp8285";
#else
#error Modelo de placa inválido
#endif
}

String swCurrentVersion() {
  // Versão do Software
  return VCS_SW_VERSION;
}

time_t swCurrentBuild() {
  // Build do Software
  TimeElements te;
  String s = __DATE__;
  te.Year   = s.substring( 7, 11).toInt() - 1970;
  te.Month  = String("JanFebMarAprMayJunJulAugSetOctNovDec").indexOf(s.substring(0, 3)) / 3 + 1;
  te.Day    = s.substring( 4,  6).toInt();
  s = __TIME__;
  te.Hour   = s.substring( 0,  2).toInt();
  te.Minute = s.substring( 3,  5).toInt();
  te.Second = s.substring( 6,  8).toInt();
  return makeTime(te);
}

String fsCurrentVersion() {
  // Versão do Sistema de Arquivos
  File file = SPIFFS.open(F("/vcsFSInfo.txt"), "r");
  String s;
  if (file && !file.isDirectory()) {
    file.setTimeout(100);
    s = file.readString().substring(20);
    file.close();
  } else {
    s = F("N/D");
  }
  return s;
}

time_t fsCurrentBuild() {
  // Build do Sistema de Arquivos
  File file = SPIFFS.open(F("/vcsFSInfo.txt"), "r");
  if (file && !file.isDirectory()) {
    file.setTimeout(100);
    String s = file.readString().substring(0, 20);
    file.close();
    return iso8601DateTime(s);
  } else {
    return 0;
  }
}

void vcsCheck() {
  // Efetua processo de verificação de versão
  if (WiFi.status() != WL_CONNECTED || vcsNextCheck > now()) {
    // Sem conexão ou aguarda intervalo
    return;
  }

  // Atualiza data/hora da próxima verificação
  vcsNextCheck = now() + VCS_CHECK_INT;

  WiFiClientSecure client;
#ifdef ESP8266
  // ESP8266
  client.setInsecure();
#endif
  Serial.print("Conectando em: ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("Falha na conexao");
    return;
  }

  log(F("Verificando versão SW..."));
  log(VCS_SWURL);
  client.print(String("GET ") + VCS_SWURL + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuscaVersionamentoBinario\r\n" + "Connection: close\r\n\r\n");

  Serial.println("Requisita conexao");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Recebe Cabecalho");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  line = client.readStringUntil('\n');
  if (line.startsWith("{\"name\":\"ControleVersao.ino")) {
    Serial.println("Sucesso na leitura da versão do SW!");

    // Tratamento do arquivo
    const char* json = line.c_str();
    StaticJsonDocument<800> jsonSWVCS;
    deserializeJson(jsonSWVCS, json);

    // Armazena dados na estrutura SWVCS
    strlcpy(vcs.swVersion, jsonSWVCS["sha"]          | "", sizeof(vcs.swVersion));
    vcs.swMandatory =      jsonSWVCS["swMandatory"]  | true;
    strlcpy(vcs.swURL,     jsonSWVCS["download_url"] | "", sizeof(vcs.swURL));

    log(F("Dados recebidos:"));
    serializeJsonPretty(jsonSWVCS, Serial);
    Serial.println();
  } else {
    Serial.println("Falha na leitura da versão do SW");
    return;
  }
  client.stop();

  log(F("Verificando data do SW..."));
  log(VCS_SWDTURL);

  for (int i = 0; i < 500; i++) {
    delay(10);
    if (WiFi.status() == WL_CONNECTED) {
      if (client.connect(host, httpsPort) == true) {

        client.print(String("GET ") + VCS_SWDTURL + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: BuscaVersionamentoBinario\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
          }
        }

        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            Serial.println("Recebe Cabecalho");
            break;
          }
        }

        Serial.println("Buscando dados");
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.startsWith("{\"sha\"")) {
            //Serial.println(line);

            // Tratamento do arquivo
            const char* json = line.c_str();
            StaticJsonDocument<400> jsonFSVCS;
            deserializeJson(jsonFSVCS, json);

            const char* date = jsonFSVCS["commit"]["author"]["date"];
            vcs.swRelease   =      iso8601DateTime(date);

            log(F("Dados recebidos:"));
            serializeJsonPretty(jsonFSVCS, Serial);
            Serial.println();

            break;
          }
        }

        Serial.println();
        Serial.println("closing connection");
        client.stop();
      }
      break;//Encerra o loop FOR()
    }
  }

  log(F("Verificando versão FS..."));
  log(VCS_FSURL);
  for (int i = 0; i < 500; i++) {
    delay(10);
    if (WiFi.status() == WL_CONNECTED) {
      if (client.connect(host, httpsPort) == true) {

        client.print(String("GET ") + VCS_FSURL + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: BuscaVersionamentoBinario\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
          }
        }

        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            Serial.println("Recebe Cabecalho");
            break;
          }
        }

        Serial.println("Buscando dados");
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.startsWith("{\"name\":\"ControleVersao.spiffs")) {
            //Serial.println(line);

            // Tratamento do arquivo
            const char* json = line.c_str();
            StaticJsonDocument<800> jsonFSVCS;
            deserializeJson(jsonFSVCS, json);

            // Armazena dados na estrutura FSVCS
            strlcpy(vcs.fsVersion, jsonFSVCS["sha"]    | "", sizeof(vcs.fsVersion));
            vcs.fsMandatory =      jsonFSVCS["fsMandatory"]  | true;
            strlcpy(vcs.fsURL,     jsonFSVCS["download_url"]        | "", sizeof(vcs.fsURL));

            log(F("Dados recebidos:"));
            serializeJsonPretty(jsonFSVCS, Serial);
            Serial.println();

            break;
          }
        }

        Serial.println();
        Serial.println("closing connection");
        client.stop();
      }
      break;//Encerra o loop FOR()
    }
  }

  log(F("Verificando data do FS..."));
  log(VCS_FSDTURL);
  for (int i = 0; i < 500; i++) {
    delay(10);
    if (WiFi.status() == WL_CONNECTED) {
      if (client.connect(host, httpsPort) == true) {

        client.print(String("GET ") + VCS_FSDTURL + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "User-Agent: BuscaVersionamentoBinario\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
          }
        }

        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            Serial.println("Recebe Cabecalho");
            break;
          }
        }

        Serial.println("Buscando dados");
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.startsWith("{\"sha\"")) {
            //Serial.println(line);

            // Tratamento do arquivo
            const char* json = line.c_str();
            StaticJsonDocument<400> jsonFSVCS;
            deserializeJson(jsonFSVCS, json);

            const char* date = jsonFSVCS["commit"]["author"]["date"];
            vcs.fsRelease   =      iso8601DateTime(date);

            log(F("Dados recebidos:"));
            serializeJsonPretty(jsonFSVCS, Serial);
            Serial.println();

            break;
          }
        }

        Serial.println();
        Serial.println("closing connection");
        client.stop();
      }
      break;//Encerra o loop FOR()
    }
  }
}

void vcsUpdate() {
  // Efetua o processo de atualizaçao do Sofwtare e Sistema de Arquivos
  WiFiClientSecure client;
#ifdef ESP8266
  // ESP8266
  client.setInsecure();
  ESPhttpUpdate.rebootOnUpdate(false);

  // Callback - Progresso
  ESPhttpUpdate.onProgress([](size_t progresso, size_t total) {
    Serial.print(progresso * 100 / total);
    Serial.print(" ");
  });
#else
  // ESP32
  httpUpdate.rebootOnUpdate(false);

  // Callback - Progresso
  Update.onProgress([](size_t progresso, size_t total) {
    Serial.print(progresso * 100 / total);
    Serial.print(" ");
  });
#endif

  if (fsCurrentVersion() != vcs.fsVersion) {
    // Atualiza Sistema de Arquivos
    log(F("Atualizando Sistema de Arquivos..."));
    log(vcs.fsURL);
    SPIFFS.end();

#ifdef ESP8266
    // ESP8266
    t_httpUpdate_return r = ESPhttpUpdate.updateSpiffs(client, vcs.fsURL);
#else
    // ESP32
    t_httpUpdate_return r = httpUpdate.updateSpiffs(client, vcs.fsURL);
#endif

    // Verifica resultado
    switch (r) {
      case HTTP_UPDATE_FAILED: {
#ifdef ESP8266
          // ESP8266
          String s = ESPhttpUpdate.getLastErrorString();
#else
          // ESP32
          String s = httpUpdate.getLastErrorString();
#endif
          log("Falha: " + s);
        } break;
      case HTTP_UPDATE_NO_UPDATES: {
          log(F("Nenhuma atualização disponível"));
        } break;
      case HTTP_UPDATE_OK: {
          log("Atualizado");
        } break;
    }

    yield();
    SPIFFS.begin();
    hold(500);
  }

  if (swCurrentVersion() != vcs.swVersion) {
    // Atualiza Software
    log(F("Atualizando Software..."));
    log(vcs.swURL);

#ifdef ESP8266
    // ESP8266
    t_httpUpdate_return r = ESPhttpUpdate.update(client, vcs.swURL);
#else
    // ESP32
    t_httpUpdate_return r = httpUpdate.update(client, vcs.swURL);
#endif

    // Verifica resultado
    switch (r) {
      case HTTP_UPDATE_FAILED: {
#ifdef ESP8266
          // ESP8266
          String s = ESPhttpUpdate.getLastErrorString();
#else
          // ESP32
          String s = httpUpdate.getLastErrorString();
#endif
          log("Falha: " + s);
        } break;
      case HTTP_UPDATE_NO_UPDATES:
        log("Nenhuma atualização disponível");
        break;
      case HTTP_UPDATE_OK: {
          log("Atualizado, reiniciando...");
          hold(500);
          ESP.restart();
        } break;
    }
  }
}
