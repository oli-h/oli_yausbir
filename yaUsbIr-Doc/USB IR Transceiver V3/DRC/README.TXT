Installationshinweise:
Die datei DRC.zip in das Verzeichnis c:\Programme\Eagle???\dru entpacken.
Sollen diese CRC-Regeln immer G�ltigkeit haben, dann kopieren sie die Datei zwilling.dru nach default.dru.
Danach den Layouteditor starten und im Menue "Bearbeiten", "Design-Regeln" und "Laden" die entsprechende DRC-Datei (in unserem Falle zwilling.dru) laden und mit "OK" bestaetigen.
Danach im Menuepunkt "Werkzeuge" "Layout pruefen (DRC)" anwaehlen und mit der "Pruefen"-Taste bestaetigen.
Danach werden vorhandene Fehler angezeigt!


Eckdaten in Kurzform:
minimale Leiterbahnbreite/-abstand: >= 0.2 mm
Strichst�rke f�r Kupferschrift >= 0.2 mm
kleinster Bohrdurchmesser: 0.5 mm --> kleinstes L�tauge 0.9mm
Maximale Gr��e �ber Alles: 100mm  x 160mm (bei Einzelplatten incl. 5mm Fr�sabstand!!)
Zu gro�e Leiterplatten werden als Prototyp gefertigt!
Restringe um DK-Bohrungen: >= 0.2 mm
Polygon-Isolierungen: >= 0.3 mm
L�tstopplack-Freistellung 0.1mm

Zus�tzliche Information f�r Konturfr�sen:
-----------------------------------------
Als Fr�skontur wird, wenn nicht anders angegeben, die Leiterplattenumrandung im  Dimension-Layer 20, als Rechteck �ber die maximalen Koordinaten, verwendet. Werden dort zus�tzliche Konturen mit einem 2mm Werkzeug eingef�gt, so k�nnen diese sofort als Fr�skoordinaten �bernommen, und somit kostenfrei gefr�st werden. Alle anderen Fr�serdurchmesser werden ignoriert oder kostenplichtig editiert und m�ssen extra bestellt werden. Au�erdem ist zu beachten, da� alle Einzel-Leiterplatten �ber Stege von min. 1mm Breite zusammengehalten werden m�ssen! Dabei ist es wichtig, da� an der Aussenkontur keine Stege angebracht werden (siehe Zeichnung), damit die Platten von unserem Fertigungsnutzen automatisch getrennt werden. Ist keine Fr�skontur enthalten, so wird nur die Au�enkontur als Rechteck gefr�st! Einzelleiterplatten werden in einem Abstand von mindestens 5mm gesetzt, um beim Fr�sen ein gegenseitiges Besch�digen zu verhindern.
Werden Kreisb�gen gefr�st, muss das Linienende auf CAP = ROUND gesetzt werden
Sollen mehrere gleiche Leiterplatten gefertigt werden, so reicht es aus, die Daten einer Einzel-Leiterplatte zu liefern (ohne Stege), ein Nutzen wird bei uns automatisch kostenfrei generiert.
Um die Leiterplattengr��e genau berechnen zu k�nnen ist es notwendig, da� keinerlei Linien oder Texte au�erhalb der Leiterplattenumrandung positioniert werden!

Siehe auch unter: http://www.leiterplatten-produktion.de/html/haka_technik.htm#fraeskontur

Alle Parameter sind einzusehen unter: http://www.leiterplatten-produktion.de/html/haka_technik.htm