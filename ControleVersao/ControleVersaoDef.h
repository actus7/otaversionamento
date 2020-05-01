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
  //#define FORMAT_SPIFFS_IF_FAILED true
#endif
#include <TimeLib.h>
#include <ArduinoJson.h>

/***********************************
* AJUSTES PARA TRABALHO COM O GITHUB
***********************************/
const char* host = "api.github.com";
const int httpsPort = 443;

/*******************************************************************************
* DEFINIÇÕES DE CONSTANTES
*******************************************************************************/
// Wi-Fi
const char*       WIFI_SSID               = "Narnia";
const char*       WIFI_PASSWORD           = "!Wehschmitt2019!";

// Intervalo verificação de versão
// Sugestão a cada 6 horas (60 * 60 * 6)
//const int       VCS_CHECK_INT           = 60 * 60 * 6;
const int         VCS_CHECK_INT           = 60 * 10; // 10 minutos apenas para teste

// Controle de Versão
#ifdef ARDUINO_ESP32_DEV
  // ESP32
  const char*     VCS_SW_VERSION          = "0";
  const char*     VCS_SWURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.ino.esp32.bin?ref=master";
  const char*     VCS_SWDTURL             = "/repos/actus7/otaversionamento/commits/master?path=ControleVersao/bin/ControleVersao.ino.esp32.bin";
  
  const char*     VCS_FSURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.spiffs.esp32.bin?ref=master";
  const char*     VCS_FSDTURL             = "/repos/actus7/otaversionamento/commits/master?path=ControleVersao/bin/ControleVersao.spiffs.esp32.bin";
#elif defined ARDUINO_ESP8266_GENERIC
  // ESP8266 Genérico
  const char*     VCS_SW_VERSION          = "0";
  const char*     VCS_SWURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.ino.generic.bin?ref=master";
  const char*     VCS_FSURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.spiffs.generic.bin?ref=master";
#elif defined ARDUINO_ESP8266_NODEMCU
  // ESP8266 NodeMCU
  const char*     VCS_SW_VERSION          = "0";
  const char*     VCS_SWURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.ino.nodemcu.bin?ref=master";
  const char*     VCS_FSURL               = "/repos/actus7/otaversionamento/contents/ControleVersao/bin/ControleVersao.spiffs.nodemcu.bin?ref=master";
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
