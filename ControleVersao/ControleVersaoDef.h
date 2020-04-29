/*******************************************************************************
 * CANAL INTERNET E COISAS                                                     *
 * Controle de Versão - Multiplataforma ESP32 e ESP8266                        *
 * Arquivo de Definições                                                       *
 * 04/2020 - Andre Michelon                                                    *
 * andremichelon@internetecoisas.com.br                                        *
 * https://internetecoisas.com.br                                              *
 ******************************************************************************/

/*******************************************************************************
* BIBLIOTECAS
*******************************************************************************/
#ifdef ESP8266
  // Bibliotecas para ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266httpUpdate.h>
  #include <FS.h>
#else
  // Bibliotecas para ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  #include <HTTPUpdate.h>
  #include <SPIFFS.h>
#endif
#include <TimeLib.h>
#include <ArduinoJson.h>

/*******************************************************************************
* DEFINIÇÕES DE CONSTANTES
*******************************************************************************/
// Wi-Fi
const char*       WIFI_SSID               = "home2.4";
const char*       WIFI_PASSWORD           = "Home31415";

// Intervalo verificação de versão
// Sugestão a cada 6 horas (60 * 60 * 6)
//const int       VCS_CHECK_INT           = 60 * 60 * 6;
const int         VCS_CHECK_INT           = 10; // 10s apenas para teste

// Controle de Versão
#ifdef ARDUINO_ESP32_DEV
  // ESP32
  const char*     VCS_SW_VERSION          = "1.0";
  const char*     VCS_URL                 = "https://dl.dropboxusercontent.com/s/nea9ad5rz9xj6ym/vcs.esp32.txt";
#elif defined ARDUINO_ESP8266_GENERIC
  // ESP8266 Genérico
  const char*     VCS_SW_VERSION          = "1.0";
  const char*     VCS_URL                 = "https://dl.dropboxusercontent.com/s/ai6qdpxgufw9qr5/vcs.generic.txt";
#elif defined ARDUINO_ESP8266_NODEMCU
  // ESP8266 NodeMCU
  const char*     VCS_SW_VERSION          = "1.0";
  const char*     VCS_URL                 = "https://dl.dropboxusercontent.com/s/nx2d4bnhov9vx00/vcs.nodemcu.txt";
#else
  #error Modelo de placa inválido
#endif

// Intervalo NTP (60s * 60m * 6 = 6h)
const int         NTP_INT                 = 60 * 60 * 6;

// Porta Servidor Web
const byte        WEBSERVER_PORT          = 80;

// Fuso Horário
const int8_t      TIMEZONE                = -3;

// Tamanho do Objeto JSON
const   size_t    JSON_SIZE               = JSON_OBJECT_SIZE(8) + 390;

/*******************************************************************************
* ENTIDADES
*******************************************************************************/
// Web Server
#ifdef ESP8266
  // Classe WebServer para ESP8266
  ESP8266WebServer  server(WEBSERVER_PORT);
#else
  // Classe WebServer para ESP32
  WebServer  server(WEBSERVER_PORT);
#endif

/*******************************************************************************
* VARIÁVEIS GLOBAIS
*******************************************************************************/
// Próxima verificação de versão
time_t            vcsNextCheck            = 0;

// Dados do arquivo de versão
struct VCS {
  char    swVersion[14];
  bool    swMandatory;
  time_t  swRelease;
  char    swURL[101];
  char    fsVersion[14];
  bool    fsMandatory;
  time_t  fsRelease;
  char    fsURL[101];
};
VCS vcs;
