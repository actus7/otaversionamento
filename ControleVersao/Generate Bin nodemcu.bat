tools\nodemcu\mkspiffs -p 256 -b 8192 -s 0x0FB000 -c data/ bin\ControleVersao.spiffs.nodemcu.bin   
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 -o bin\ControleVersao.ino.nodemcu.bin