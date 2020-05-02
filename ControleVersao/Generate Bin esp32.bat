tools\esp32\mkspiffs -p 256 -b 8192 -s 0x0FB000 -c data/ bin\ControleVersao.spiffs.esp32.bin             
arduino-cli compile --fqbn esp32:esp32:esp32 -o bin\ControleVersao.ino.esp32.bin
