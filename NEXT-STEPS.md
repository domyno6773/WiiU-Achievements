# Prossimo ciclo di sviluppo

La revisione del 2026-10-29 aggiunge rarità esplicite e correzioni di ordine/testo.
Usare `build-check/rarity/`; la memoria e le condizioni restano quelle della
build Playthrough TEST. Il futuro platino sarà un obiettivo separato di
completamento del set; non assegnarlo a una semplice soglia da 25 punti.

**Build attuale: Playthrough TEST.** Sono implementati 53 nuovi rilevamenti
sperimentali, in serie ordinate dalla partenza fino agli strumenti finali: 70
attivi in totale, 40 non implementati. **Tappa / F** filtra il percorso e
**Categoria / C** il tipo. L'utente li verificherà durante la partita normale.
La guida operativa è `docs/PLAYTHROUGH.md`. Le note sotto descrivono la build
precedente e la progressione del lavoro.

Il catalogo ora comprende **110 obiettivi**: **17 attivi** (16 soglie più la tunica verde) e **93 obiettivi di storia ed extra da verificare**, con filtri per categoria e stato. ID originali e revisione `wwhd-alpha-1` sono conservati. I 93 traguardi preparati non assegnano punti o sblocchi finché non hanno una rilevazione verificata. La build `build-check/scroll-fix` corregge lo scorrimento e attiva la tunica dopo verifica live prima/dopo, caricamento e menu. L'utente ha confermato la notifica della tunica e lo sblocco è stato verificato su disco. Rimangono da provare riavvio dell'app per questo obiettivo e riavvio Cemu. Dettagli e fonti: `docs/ACHIEVEMENTS.md`.

0. **Alpha 0.3 sul PC con Cemu:** ricompilare con `build.ps1` ed eseguire i test. Poi controllare la demo e la notifica sopra Cemu, anche a schermo intero, il jingle, i tasti e la scala DPI. Controllare anche Giochi, pausa prima del caricamento, ripresa senza sblocchi retroattivi e una nuova soglia raggiunta dopo tre campioni sotto soglia. Le prove cloud non sostituiscono la sessione reale.
1. **Stato della partita:** identificare menu iniziale, caricamento, slot selezionato, modalità normale/eroe e sessione giocabile. Questa è una priorità prima di attribuire obiettivi di storia.
2. **Inventario iniziale:** acquisizione di cannocchiale, spada e scudo. Confrontare campioni prima e dopo ogni acquisizione, quindi verificare che i flag rimangano coerenti nei menu, nei cambi di area e dopo un riavvio.
3. **Progressione:** vela, Wind Waker, strumenti dei dungeon e boss, con condizioni che distinguano l'evento appena avvenuto dal caricamento di dati già completati.
4. **Attivazione del set curato:** implementare progressivamente i 93 traguardi ancora da verificare, mantenendo gli ID. Aggiungere condizioni su bit/eventi e regressioni solo dopo evidenze live; non tradurre maschere in soglie numeriche. Il catalogo copre storia, strumenti, mare, carte, melodie, collezioni e secondarie HD.
5. **Profili ulteriori:** altre regioni/versioni e riavvii di Cemu, con validazione esplicita degli indirizzi e delle firme.

Per le prossime prove bastano il gioco, l'app e i valori/eventi osservati dall'utente. Non servono password, chiavi della console o il trasferimento di ROM al progetto.

Per riprendere senza il contesto della chat, leggi `HANDOFF.md` e le evidenze in
`docs/VALIDATION.md`. Per contributi indipendenti usa `docs/COLLABORATION.md`.
