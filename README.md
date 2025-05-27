# Wetterstation mit Webinterface (Variante 4: RGB Colorpicker)

**Verfasser:** Elyesa Güngür,Cemil Dindar 
**Datum:** 27.05.2025

Datum: 27.05.2025

## 1.Einführung
Es wurde ein Mikrocontroller-Projekt umgesetzt, bei dem eine einfache Wetterstation mit Sensoren gebaut wurde. Ziel war es, Temperatur und Luftfeuchtigkeit zu messen und diese über ein Webinterface darzustellen. Zusätzlich konnten Farben der Status-LED über das Webinterface verändert und gespeichert werden.

## 2.Projeltbeschreibung
Mit Hilfe eines ESP32 Mikrocontrollers wurde eine Wetterstation gebaut. Sie misst Temperatur und Luftfeuchtigkeit mit einem DHT11 Sensor und zeigt die Daten über eine Webseite an. Besonderheit: Die Status-LED kann mit einem grafischen Colorpicker im Webinterface gesteuert werden. Die ausgewählten Farben bleiben auch nach einem Neustart erhalten.

## 3.Theorie
In diesem Projekt wurde ein ESP32 Mikrocontroller eingesetzt. Dieses kleine Gerät kann Daten verarbeiten und sich mit dem WLAN verbinden. Dadurch ist es möglich, Informationen wie Temperatur oder Farben über eine Webseite anzuzeigen oder zu ändern.

Der Sensor, der verwendet wurde, heißt DHT11. Er misst Temperatur und Luftfeuchtigkeit. Diese Werte werden vom Mikrocontroller regelmäßig ausgelesen und weitergegeben. Der DHT11 ist sehr beliebt, weil er einfach zu benutzen ist und für viele kleine Projekte ausreicht.

Damit die Wetterstation auch Farben anzeigen kann, wurde eine RGB-LED eingebaut. Diese LED kann verschiedene Farben leuchten, je nachdem, wie stark jeder der drei Farbkanäle (Rot, Grün, Blau) angesteuert wird. Die Farbe kann man über eine Webseite auswählen – ein sogenannter „Colorpicker“ macht das einfach möglich.

Damit die gewählte Farbe auch nach einem Neustart erhalten bleibt, speichert der ESP32 sie im internen Speicher (EEPROM). Der Code dafür wurde mit der Arduino IDE geschrieben, einer Umgebung, die speziell für Mikrocontroller-Projekte gedacht ist.

## 4.Arbeitsschritte
Zuerst haben wir den ESP32 richtig mit dem Computer verbunden und die Arduino IDE eingerichtet. Dafür mussten wir die passenden Treiber und das Board installieren.

Danach haben wir den DHT11-Sensor angeschlossen. Wir haben getestet, ob er richtig funktioniert und ob er Temperatur und Luftfeuchtigkeit anzeigen kann.

Als Nächstes haben wir den RGB-LED-Streifen angeschlossen. Wir haben ausprobiert, ob die Farben richtig leuchten und ob sich jede Farbe einzeln steuern lässt.

Dann haben wir einen einfachen Webserver auf dem ESP32 eingerichtet. Dieser zeigt eine Webseite mit einem Colorpicker an, mit dem man die LED-Farbe auswählen kann.

Nachdem das funktioniert hat, haben wir den Code erweitert, damit die ausgewählte Farbe im Speicher (EEPROM) gespeichert wird. So bleibt die Farbe auch nach einem Neustart erhalten.

Am Ende haben wir alles getestet und kleinere Fehler verbessert.

