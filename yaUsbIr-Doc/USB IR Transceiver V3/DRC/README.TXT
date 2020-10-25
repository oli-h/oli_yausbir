Installationshinweise:
Die datei DRC.zip in das Verzeichnis c:\Programme\Eagle???\dru entpacken.
Sollen diese CRC-Regeln immer Gültigkeit haben, dann kopieren sie die Datei zwilling.dru nach default.dru.
Danach den Layouteditor starten und im Menue "Bearbeiten", "Design-Regeln" und "Laden" die entsprechende DRC-Datei (in unserem Falle zwilling.dru) laden und mit "OK" bestaetigen.
Danach im Menuepunkt "Werkzeuge" "Layout pruefen (DRC)" anwaehlen und mit der "Pruefen"-Taste bestaetigen.
Danach werden vorhandene Fehler angezeigt!


Eckdaten in Kurzform:
minimale Leiterbahnbreite/-abstand: >= 0.2 mm
Strichstärke für Kupferschrift >= 0.2 mm
kleinster Bohrdurchmesser: 0.5 mm --> kleinstes Lötauge 0.9mm
Maximale Größe über Alles: 100mm  x 160mm (bei Einzelplatten incl. 5mm Fräsabstand!!)
Zu große Leiterplatten werden als Prototyp gefertigt!
Restringe um DK-Bohrungen: >= 0.2 mm
Polygon-Isolierungen: >= 0.3 mm
Lötstopplack-Freistellung 0.1mm

Zusätzliche Information für Konturfräsen:
-----------------------------------------
Als Fräskontur wird, wenn nicht anders angegeben, die Leiterplattenumrandung im  Dimension-Layer 20, als Rechteck über die maximalen Koordinaten, verwendet. Werden dort zusätzliche Konturen mit einem 2mm Werkzeug eingefügt, so können diese sofort als Fräskoordinaten übernommen, und somit kostenfrei gefräst werden. Alle anderen Fräserdurchmesser werden ignoriert oder kostenplichtig editiert und müssen extra bestellt werden. Außerdem ist zu beachten, daß alle Einzel-Leiterplatten über Stege von min. 1mm Breite zusammengehalten werden müssen! Dabei ist es wichtig, daß an der Aussenkontur keine Stege angebracht werden (siehe Zeichnung), damit die Platten von unserem Fertigungsnutzen automatisch getrennt werden. Ist keine Fräskontur enthalten, so wird nur die Außenkontur als Rechteck gefräst! Einzelleiterplatten werden in einem Abstand von mindestens 5mm gesetzt, um beim Fräsen ein gegenseitiges Beschädigen zu verhindern.
Werden Kreisbögen gefräst, muss das Linienende auf CAP = ROUND gesetzt werden
Sollen mehrere gleiche Leiterplatten gefertigt werden, so reicht es aus, die Daten einer Einzel-Leiterplatte zu liefern (ohne Stege), ein Nutzen wird bei uns automatisch kostenfrei generiert.
Um die Leiterplattengröße genau berechnen zu können ist es notwendig, daß keinerlei Linien oder Texte außerhalb der Leiterplattenumrandung positioniert werden!

Siehe auch unter: http://www.leiterplatten-produktion.de/html/haka_technik.htm#fraeskontur

Alle Parameter sind einzusehen unter: http://www.leiterplatten-produktion.de/html/haka_technik.htm