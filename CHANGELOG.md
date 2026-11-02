# Änderungen

## 0.1.3 — 02.11.2026

- Webstatus wird sekündlich abgefragt: curl-/Shell-Änderungen erscheinen automatisch in Anzeige, Balken und Zellzuständen.
- Laufende Eingaben und fokussierte Bedienelemente werden beim Polling nicht überschrieben.
- Keine parallelen Status-/Schaltanfragen; versteckte Browser-Tabs pausieren das Polling.

## 0.1.2 — 02.11.2026

- Weblayout im Stil des Powermeters: dunkle Karten, mintfarbene Anzeige, Balken und GPIO-Zustandstabelle.
- Mausrad über dB-Eingabe: 1-dB-Schritte mit automatischem Setzen nach 200 ms Pause.
- Slider und ±1-dB-Tasten, Enter/Setzen, Grenzen 0–127 dB.
- Serielle HTTP-Schreibzugriffe mit Zusammenfassung schneller Änderungen und Fehleranzeige.
- GPIO-Zuordnung und WLAN-Verhalten von 0.1.1 unverändert.

## 0.1.1 — 02.11.2026

- GPIO-Zuordnung nach Anwender: GP18/32, GP19/1, GP20/2, GP21/32, GP22/4, GP26/8, GP27/16, GP28/32 dB.
- GP21-AP-Boot-Taste entfernt, damit der 32-dB-Ausgang nicht als Eingang umkonfiguriert wird.
- Boot immer STA; AP weiter per USB `wifi_mode ap`.
- Polarität weiterhin aktiv HIGH; Boot 0 dB.
- Host-Tests und GPIO-Masken geprüft; 0.1.1 ohne vollständigen Zephyr-Build.

## 0.1.0 — 31.10.2026

- Eigenständiger RP2040/Pico-W-Entwurf auf Basis Powermeter 0.1.53.
- WLAN/Settings, AP, USB-CDC und TCP-Konsole übernommen.
- Acht konfigurierbare Dämpferzellen, mehrfach gewichtete 32-dB-Zellen.
- `info`, `att status`, `att set`, Weboberfläche und HTTP-API.
- Gemeinsamer GPIO-Portzugriff, Schaltpause, Fehlerstatus und Host-Tests.
- Zephyr-Build und Hardwareabgleich noch ausstehend.
