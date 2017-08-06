windres -O COFF ftdi.latency.rc -o rc.o
g++ -Wall -O2 -DUNICODE -D_UNICODE -s ftdi.latency.cpp rc.o -o ftdi.latency.exe
