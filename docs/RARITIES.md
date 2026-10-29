# Rarità e revisione editoriale — 2026-10-29

La rarità è ora assegnata esplicitamente a tutti i **110 obiettivi**:
**53 bronzo, 47 argento, 10 oro, nessun platino attuale**. Il fatto che un
rilevamento sia sperimentale o non ancora implementato rimane separato.

- **Bronzo:** prime acquisizioni, singoli strumenti e melodie, soglie iniziali e
  attività brevi. Uno strumento obbligatorio non diventa raro solo perché arriva
  tardi nella storia.
- **Argento:** completamenti di dungeon e tappe sostanziali, potenziamenti
  facoltativi, raccolte intermedie e missioni/minigiochi con un impegno maggiore.
- **Oro:** grandi completamenti, raccolte integrali e traguardi conclusivi.
- **Platino:** riservato al futuro trofeo conclusivo del set completo. Nessun
  obiettivo attuale lo assegna. La vittoria su Ganondorf è prevista come oro;
  il futuro platino sarà separato e richiederà una definizione completa del set.

I dieci ori previsti sono: 20 cuori, Triforza completa, Molgera con pieno potere
della Spada Suprema, Ganondorf, 49 settori della carta nautica, tutti i 46 tesori
sommersi delle carte, fondo del labirinto, 44 frammenti di cuore, 46 carte del
tesoro e Galleria completa. Alcuni sono ancora non attivi: la rarità assegnata
non ne abilita il rilevamento.

Il portafoglio da 5000 rupie è argento, mentre i 20 cuori sono oro, anche se
entrambi mantengono i 25 punti precedenti. **I punti non determinano più la
medaglia.** Non è stato ricalcolato il punteggio degli sblocchi esistenti.

## Correzioni apportate

La torre che emerge precede ora le attività della Torre degli Dei; i risvegli dei
saggi precedono le relative tappe dei templi; Puppet Ganon precede Ganondorf.
Le soglie dei cuori e delle rupie seguono l'ordine numerico crescente. Gli extra
sono raggruppati per categoria; l'ordine resta indicativo e non vincola il motore.

Le descrizioni di spada, borse, vela, macchina a colori, Spada Suprema, armatura
magica e Melodia del Ciclone sono state allineate al dato effettivamente osservato:
possesso o apprendimento. Non promettono il controllo di un pagamento, risultato
di minigioco o singolo filmato quando il decoder verifica solo l'acquisizione.

## Contratto tecnico

`achievement-presentation.xml` contiene una voce per ogni ID, con `rarity` e le
eventuali correzioni di ordine/testo. Il caricatore verifica revisione, copertura
completa, duplicati, ID sconosciuti e attributi ammessi. Il file non può modificare
punti, soglie, campi di memoria o stato sperimentale. Le rarità ammesse sono
`bronze`, `silver`, `gold`, `platinum`; quest'ultima è riservata all'ID futuro
`completion-platinum`, ancora non implementato e assente dal catalogo attuale.

Il file delle 16 soglie conserva il formato leggibile dalle vecchie build.
ID, revisione `wwhd-alpha-1`, condizioni e progressi rimangono invariati.
Schede, dettagli, accessibilità e notifiche usano tutti la stessa rarità; le
schede mostrano anche il nome sotto la medaglia. La build di revisione è in
`build-check/rarity/`, separata dall'eseguibile che potrebbe essere aperto.
