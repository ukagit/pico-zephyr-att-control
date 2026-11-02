# pico-zephyr-att-control — 0.1.3

Erster Firmware-Entwurf für Ulrich / DL2DBG: Weinschel-Engineering-Dämpfer
über GPIO steuern. Zielboard **Raspberry Pi Pico W (2022), RP2040,
2 MiB Flash**, Zephyr-Board `rpi_pico/rp2040/w`.

## Stand und Herkunft

Aus `pico-zephyr-ham-lab-powermeter` 0.1.53 übernommen: WLAN-Verwaltung
inklusive gespeicherter Zugangsdaten, AP-Modus, TCP-Konsole Port 23,
USB-CDC-Baukonfiguration und Build-/Debug-Probe-Skripte.
Neu: Dämpfermodul, zentrales Hardwareprofil, Integer-dB-Kodierung,
Shell und kleine Weboberfläche auf Port 8080.
Keine ADS1115-, Messkurven-, DDS- oder Powermeter-Abhängigkeiten.
OLED und Drehgeber sind in diesem ersten Entwurf noch nicht enthalten;
GP0/GP1 sowie GP13–GP15 bleiben dafür frei.

**Build-Ergebnis 0.1.0 beim Anwender:** Zephyr 4.4.99, Pico W/RP2040 erfolgreich gebaut. Flashen per Debug Probe scheiterte an der CMSIS-DAP-Verbindung.

**Validierung 0.1.1:** Der portable C-Kern wurde mit `-Wall -Wextra -Werror`
übersetzt und auf alle 128 Sollwerte, alle 256 Masken sowie ungültige
Eingaben geprüft. Ein vollständiger Zephyr-Build und Tests am Pico/Weinschel
stehen aus: In der Erstellungsumgebung fehlen west, Zephyr-Tree und SDK.
Das Archiv enthält Quellcode, keine gebaute UF2.

## Hardwareprofil vor Anschluss abgleichen

Annahme ist ein **3200-1 mit 0–127 dB in 1-dB-Schritten** und acht unabhängig
ansteuerbaren Zellen: 1, 2, 4, 8, 16, 32, 32, 32 dB.
Die letzten beiden 32-dB-Zellen bilden die 64-dB-Stufe.
Das ist kein gewöhnliches binäres 8-Bit-Register.
Das genaue Typenschild, eine vorhandene TTL-Treiberplatine und deren
Steckerbelegung sind noch nicht am Benutzergerät bestätigt.

Alle Zuordnungen stehen in `src/att_config.h`:

| Logische Zelle | Dämpfung | Pico GPIO | Physischer Pico-Pin |
|---|---:|---|---:|
| 0 | 1 dB | GP19 | 25 |
| 1 | 2 dB | GP20 | 26 |
| 2 | 4 dB | GP22 | 29 |
| 3 | 8 dB | GP26 | 31 |
| 4 | 16 dB | GP27 | 32 |
| 5 | 32 dB | GP18 | 24 |
| 6 | 32 dB | GP21 | 27 |
| 7 | 32 dB | GP28 | 34 |

**Das sind keine Weinschel-Stecker-Pinnummern.** Jede Leitung muss der
entsprechenden Zelle am tatsächlichen Gerät zugeordnet werden.

Standard ist aktiv HIGH am Pico. Ein invertierender externer Treiber
kann über `ATT_ACTIVE_LOW_MASK` berücksichtigt werden, bitweise je Zelle.
Pico und Treiber benötigen gemeinsame Signalmasse.

### Elektrische Schnittstelle

Laut Weinschel-Datenblatt existieren Standardgeräte mit direkt zugänglichen
12-V-Relaiszellen und Geräte mit zusätzlicher TTL-Platine. Beim Standardgerät
werden die Zellen mit positiver Spannung gegen gemeinsame Masse aktiviert.
Dafür benötigt man eine passende High-Side-Treiberstufe; ein ULN2803 allein
liefert keine positive 12-V-Schaltspannung. Bei tatsächlich anders beschalteten
Spulen richtet sich der Treiber nach deren nachgewiesener Schaltung.

Die zusätzliche klassische TTL-Platine aktiviert die Zelle bei TTL HIGH.
Das verlinkte Herstellerdatenblatt nennt mindestens 2,0 V für HIGH und
höchstens 0,8 V für LOW. Damit sind 3,3-V-Ausgänge prinzipiell geeignet,
sofern genau diese Platine vorliegt und keine unzulässigen Spannungen auf den
Pico zurückwirken. **12 V und 5-V-Ausgänge dürfen nicht an Pico-GPIOs.**
Die 12-V-Versorgung gehört an den Weinschel/Treiber, nicht an die GPIOs.
Ein 3200T/SmartStep ist eine andere Schnittstellenausführung und wird durch
diesen Entwurf nicht automatisch unterstützt.

Quellen (Herstellerunterlagen, gespiegelt):
- https://www.amtest-tm.com/public/files/programmables.pdf
- https://dtsheet.com/doc/1494422/3200-series-ttl-driver-board-instruction-manual
- https://dtsheet.com/doc/1419595/3200-1e

## Schaltverhalten

- Boot setzt alle Zellen inaktiv: **0 dB**, keine Wiederherstellung eines
  alten Dämpfungswertes aus Flash. Vor Initialisierung sind GPIOs hochohmig;
  externe Pull-Widerstände für einen definierten Bootzustand vorsehen.
- `att set` setzt alle zugehörigen GPIO-Bits über einen maskierten Portzugriff.
- Mindestens 200 ms zwischen Änderungen; nach dem GPIO-Schreiben 10 ms
  Beruhigungszeit. Gleichbleibende Werte erzeugen keinen neuen Schaltvorgang.
- Shell und HTTP benutzen dieselbe Mutex-geschützte Steuerung.
- GPIO-Fehler sperren weitere Schreibvorgänge bis Neustart. Bei einem Fehler
  ist die tatsächliche Ausgangsstellung unbekannt; vorheriger Sollwert bleibt
  nur als zuletzt erfolgreich geschriebener Wert sichtbar.
- Keine Kontakt- oder HF-Rückmeldung: Status bedeutet Soll-Schaltstellung.
- Mechanische Umschalttransienten sind trotz gemeinsamen Portzugriffs möglich.
  Beim Umschalten HF ausschalten. 0 dB beim Boot ist kein HF-Schutz.

## Bauen und Flashen

Projekt als neues Geschwisterprojekt in den bestehenden Zephyr-Workspace legen,
z. B. `apps/pico-zephyr-att-control`. Vorhandene Zephyr-4.4.99-Umgebung aktivieren.

```sh
cd apps/pico-zephyr-att-control
tools/test_host.sh
tools/build_flash.sh --pristine --build-only
```

Entspricht:

```sh
west build -b rpi_pico/rp2040/w -S cdc-acm-console . -d build -p always
```

`build/zephyr/zephyr.uf2` per BOOTSEL auf den Pico kopieren oder
`tools/flash_openocd.sh` mit der vorhandenen Debug Probe verwenden.
`tools/build_flash.sh --pristine` baut und flasht per Debug Probe.

## Bedienung

USB-CDC-Konsole oder `telnet IP 23`:

```text
info
att status
att set 0
att set 20
att set 63
att set 127
network_status
wifi_stored
wifi_mode status
```

Zugangsdaten werden mit `wifi cred add -s "SSID" -k 1 -p "PASSWORT"`
für WPA2 dauerhaft gespeichert; anschließend `wifi_stored` verwenden. Übernommen sind deren Settings/NVS-Backend und das
automatische Verbinden gespeicherter Netze. Flash-Settings werden beim
Start geladen; NVS liegt in den letzten 64 KiB des Pico-Flashs.
Die vollständige RP2040-Firmware muss in den übrigen Flash passen.

Boot startet immer im STA-Modus. GP21 ist ein Dämpferausgang und darf
nicht mehr als Boot-Taste gegen GND verwendet werden.

Optional per USB `wifi_mode ap` aufrufen: AP `ATT-DL2DBG`,
offenes Netz, IP `192.168.4.1`, DHCP `.10`–`.11`.
Auch per USB: `wifi_mode ap` oder `wifi_mode ap ATT-DL2DBG meinpasswort`.
Mit `wifi_mode sta` zurück zu gespeicherten WLAN-Zugangsdaten.
AP/STA-Auswahl ist nur RAM; Einstellungen des Dämpfers ebenfalls.
HTTP und TCP-Konsole besitzen keine Authentifizierung und sind für das
vertrauenswürdige Labornetz vorgesehen.

## Web und curl

Browser: `http://IP:8080/` — Zahlenfeld, Setzen, Status.

```sh
url=http://192.168.178.98:8080   # Beispiel-IP, tatsächliche DHCP-IP einsetzen
curl -fsS "$url/api/v1/state"
curl -fsS -X POST "$url/api/v1/att/20"
curl -fsS "$url/api/v1/att"
```

`POST /api/v1/att/<Ganzzahl>` benötigt keinen Body.
Nur POST schaltet; GET liest. Ungültige Werte erhalten HTTP 400,
unbekannte Routen 404, Schaltfehler 503. Keine automatische zyklische
Schaltung oder Sweep-Funktion in 0.1.0.

Beispielstatus:

```json
{"attenuation_db":20,"mask":20,"max_db":127,"ready":true,"gpio_error":0,"feedback":false}
```

## Erster Hardwaretest

1. Ohne angeschlossenen Weinschel bauen, flashen und USB-`info` prüfen.
2. `att set 0/1/2/4/8/16/32/64/96/127` jeweils einzeln ausführen,
   dabei den echten Zahlenwert verwenden, z. B. `att set 64`.
3. Mit Multimeter/Logikanalysator GP18/19/20/21/22/26/27/28 und Polarität prüfen.
4. Typenschild und Treiber/Stecker mit dem Datenblatt abgleichen; Profil anpassen.
5. Erst dann anschließen und ohne HF alle Zellen prüfen.
6. Mit kleinem zulässigem HF-Pegel die Dämpfung kontrollieren.

## Nächste Erweiterungen

OLED an GP0/GP1, Drehgeber an GP14/GP15 mit Taste GP13,
gespeicherte Bedienvoreinstellungen und skriptgesteuerte Messserien.

## Webbedienung ab 0.1.2

Layout nach HamLab-Powermeter: Farben, Karten, große mintfarbene Anzeige,
Balken und Statusbereich. GPIO-Tabelle zeigt die acht logischen Zellzustände.
Mausrad über dem dB-Zahlenfeld: hoch +1 dB, herunter −1 dB, Grenzen 0–127.
Nach 200 ms Pause wird automatisch geschaltet; schnelle Änderungen werden
zusammengefasst, HTTP-Aufträge laufen seriell. Auch Slider und ±1-dB-Tasten
setzen automatisch. Getippte Werte mit Enter oder Setzen übernehmen.
Bei Fehlern Status prüfen und gewünschten Wert erneut setzen.

HTML-Quelle: `web/att.html`; nach Änderungen `python3 tools/embed_web.py`
ausführen. Die eingebettete Seite liegt in `src/att_web.h`. Für das Bauen
ist keine Python-Ausführung zur Seitengenerierung erforderlich.

## Statusaktualisierung ab 0.1.3

Die sichtbare Webseite liest den Status einmal pro Sekunde. Änderungen über
curl, USB oder TCP erscheinen automatisch. Bei fokussiertem Zahlenfeld/Slider
oder unbestätigter Eingabe bleiben die Eingaben erhalten; Istwert, Balken und
GPIO-Tabelle aktualisieren sich weiter. Der Statusknopf synchronisiert auch
den Sollwert ausdrücklich. Während Schaltanfragen wird das Polling übersprungen,
in versteckten Tabs pausiert es. Keine zusätzlichen POSTs durch Polling.
