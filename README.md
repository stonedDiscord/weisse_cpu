# Die Weiße CPU

Dies ist ein Reverse-Engineering Projekt fuer die "Elektronische Steuereinheit" von adp.

Die Platine ist 4087-000101 mit 3 EPROM-Sockeln aus meinem Disc 2000.

IO Ports:
0x50 - 0x51 8279
0x60 - 0x6f 8256
0x70 - 0x71 Walzen etc
0x72        Sound

Im Gegensatz zu den 4040 Platinen wo diese noch etwas anders verteilt sind.
Das ist auch der Grund wieso die ROMs inkompatibel sind.