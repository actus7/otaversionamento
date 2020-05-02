tools\generic\mkspiffs -p 256 -b 8192 -s 0x0FB000 -c data/ bin\ControleVersao.spiffs.generic.bin          
arduino-cli compile --fqbn esp8266:esp8266:generic -o bin\ControleVersao.ino.generic.bin