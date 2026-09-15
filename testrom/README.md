# Testrom

In diesem Ordner ist ein Testrom dass auf ein EPROM gebrannt und in Sockel I auf der Steuereinheit gesteckt werden kann.

Beim Hochfahren startet es einen kurzen Selbsttest der über die serielle Schnittstelle ausgegeben wird.

Danach können über die Fronttasten des Automaten folgende Funktionen gestartet werden:

| Nr | Funktion |
| ---- | ----------------|
| 0 | Reset |
| 1 | Alle Lampen ein |
| 2 | Datum setzen |
| 3 | Musik spielen |
| 4 | Zeit setzen |
| 5 | Alle Lampen aus |
| 6 | Lampentest |
| 7 | Alle Lampen ein |
| 8 | 8256 Test |
| 9 | 8279 Test |
| 10 | RAM Test |
| 11 | RTC Test |
| 12 | Scheiben auslesen |
| 13 | Münzeinwurf aufzeichen |

Die beiden Risikotasten entsprechen links/rechts niedriger/höher und mit der Rückgabetaste kann zurück ins Menü gesprungen werden.