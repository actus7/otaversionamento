/*******************************************************************************
 * CANAL INTERNET E COISAS                                                     *
 * Controle de Versão - Multiplataforma ESP32 e ESP8266                        *
 * Arquivo Principal                                                           *
 * 04/2020 - Andre Michelon                                                    *
 * andremichelon@internetecoisas.com.br                                        *
 * https://internetecoisas.com.br                                              *
 ******************************************************************************/

/*******************************************************************************
* ARQUIVOS AUXILIARES
*******************************************************************************/
#include "ControleVersaoLib.h"

/*******************************************************************************
* FUNCÕES AUXILIARES
*******************************************************************************/
String softwareStr() {
  // Retorna nome do software
  return String(__FILE__).substring(String(__FILE__).lastIndexOf("\\") + 1);
}

/*******************************************************************************
* TRATAMENTO DAS REQUISÇÕES WEB
*******************************************************************************/
void handleHome() {
  // Home
  File file = SPIFFS.open(F("/Home.htm"), "r");
  if (file && !file.isDirectory()) {
    file.setTimeout(100);
    String s = file.readString();
    file.close();

    // Atualiza conteúdo dinâmico
    // Dispositivo
    s.replace(F("#sw#")             , softwareStr());
    s.replace(F("#hw#")             , platform());
    // Versão atual
    s.replace(F("#curSWVersion#")   , swCurrentVersion());
    s.replace(F("#curSWRelease#")   , dateTimeStr(swCurrentBuild()));
    s.replace(F("#curFSVersion#")   , fsCurrentVersion());
    s.replace(F("#curFSRelease#")   , dateTimeStr(fsCurrentBuild()));
    // Versão nova
    s.replace(F("#newSWVersion#")   , vcs.swVersion);
    s.replace(F("#newSWRelease#")   , dateTimeStr(vcs.swRelease, TIMEZONE));
    s.replace(F("#newSWMandatory#") , vcs.swMandatory ? "Sim" : "Não");
    s.replace(F("#newSWURL#")       , vcs.swURL);
    
    s.replace(F("#newFSVersion#")   , vcs.fsVersion);
    s.replace(F("#newFSRelease#")   , dateTimeStr(vcs.fsRelease, TIMEZONE));
    s.replace(F("#newFSMandatory#") , vcs.fsMandatory ? "Sim" : "Não");
    s.replace(F("#newFSURL#")       , vcs.fsURL);
    // Status
    String st = "";
    if (swCurrentVersion() != vcs.swVersion && fsCurrentVersion() != vcs.fsVersion) {
      // Atualizar SW e FS
      st = F("Atualizar Software e Interface");
    } else if (swCurrentVersion() != vcs.swVersion) {
      // Atualizar SW
      st = F("Atualizar Software");
    } else if (fsCurrentVersion() != vcs.fsVersion) {
      // Atualizar FS
      st = F("Atualizar Interface");
    }
    if (st == "") {
      // Nenuma atualização necessária
      st = F("Versão do Software e Interface estão atualizadas");
    } else {
      st += F("<br><a href=\"vcs\">Clique para Atualizar</b></a>");
    }
    s.replace(F("#status#")         , st);

    // Envia dados
    server.send(200, F("text/html"), s);
    log("Home - Cliente: " + ipStr(server.client().remoteIP()) +
        (server.uri() != "/" ? " [" + server.uri() + "]" : ""));
  } else {
    server.send(500, F("text/plain"), F("Home - ERROR 500"));
    log(F("Home - ERRO lendo arquivo"));
  }
}

void handleVCS() {
  // Manual Version update
  File file = SPIFFS.open(F("/VCS.htm"), "r");
  if (file && !file.isDirectory()) {
    file.setTimeout(100);
    String s = file.readString();
    file.close();

    // Atualiza conteúdo dinâmico
    // Dispositivo
    s.replace(F("#sw#")             , softwareStr());
    s.replace(F("#hw#")             , platform());

    // Status
    byte t = 1;
    String st = "";
    if (swCurrentVersion() != vcs.swVersion) {
      // Atualizar SW
      st = "Software: " + swCurrentVersion() + " &rarr; " + vcs.swVersion + "<br>";
      // Tempo de espera
      #ifdef ESP8266
        t += 90;
      #else
        t += 70;
      #endif
    }
    if (fsCurrentVersion() != vcs.fsVersion) {
      // Atualizar FS
      st += "Interface: " + fsCurrentVersion() + " &rarr; " + vcs.fsVersion + "<br>";
      // Tempo de espera
      #ifdef ESP8266
        t += 90;
      #else
        t += 70;
      #endif
    }
    s.replace(F("#status#")         , st);

    // Tempo de espera
    s.replace(F("#time#")           , String(t));

    // Envia dados
    server.send(200, F("text/html"), s);
    log("VCS - Cliente: " + ipStr(server.client().remoteIP()));
    hold(1000);

    // Efetua atualização
    vcsUpdate();
  } else {
    server.send(500, F("text/plain"), F("VCS - ERROR 500"));
    log(F("VCS - ERRO lendo arquivo"));
  }
}

void handleStream(const String f, const String mime) {
  // Processa requisição de arquivo
  File file = SPIFFS.open("/" + f, "r");
  if (file && !file.isDirectory()) {
    // Define cache para 2 dias
    server.sendHeader(F("Cache-Control"), F("public, max-age=172800"));
    server.streamFile(file, mime);
    file.close();
    log(f + " - Cliente: " + ipStr(server.client().remoteIP()));
  } else {
    server.send(500, F("text/plain"), F("Stream - ERROR 500"));
    log(F("Stream - ERRO lendo arquivo"));
  }
}

void handleLogo() {
  // IeC Logo
  handleStream(F("IeCLogo.png"), F("image/png"));
}

void handleCSS() {
  // CSS
  handleStream(F("css.css"), F("text/css"));
}

/*******************************************************************************
* SETUP
*******************************************************************************/
void setup() {
  #ifdef ESP8266
    // Velocidade para ESP8266
    Serial.begin(74880);
  #else
    // Velocidade para ESP32
    Serial.begin(115200);
  #endif

  log("\nIniciando...");

  // SPIFFS
  if (!SPIFFS.begin()) {
    log(F("SPIFFS ERRO"));
    while (true);
  }

  // Conecta WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  log("Conectando WiFi " + String(WIFI_SSID));
  byte b = 0;
  while(WiFi.status() != WL_CONNECTED) {
    b++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  log("WiFi conectado (" + String(WiFi.RSSI()) + ") IP " + ipStr(WiFi.localIP()));

  // Define NTP
  setSyncProvider(timeNTP);
  setSyncInterval(NTP_INT);
  log(timeStatus() == timeSet ? F("Data/hora atualizada") : F("Falha atualizando Data/hora"));

  // WebServer
  server.on(F("/vcs")         , handleVCS);
  server.on(F("/IeCLogo.png") , handleLogo);
  server.on(F("/css.css")     , handleCSS);
  server.onNotFound(handleHome);
  server.begin();

  // Pronto
  log(F("Pronto"));
}

/*******************************************************************************
* LOOP
*******************************************************************************/
void loop() {
  // WatchDog ----------------------------------------
  yield();

  // Controle de Versão ------------------------------
  vcsCheck();

  // Web ---------------------------------------------
  server.handleClient();
}
