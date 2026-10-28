# Prossimo ciclo di sviluppo

Alpha 0.3 aggiunge catalogo giochi, adattatori per profilo, pausa/ripresa, stati di verifica condivisi e salvataggio atomico degli sblocchi simultanei. Il set resta di dieci obiettivi; ID e progressi sono conservati. La base originale ha evidenza sulla configurazione EUR confermata. Per estendere il set servono eventi osservati durante il gioco, non soltanto indirizzi trovati online.

0. **Alpha 0.3 sul PC con Cemu:** ricompilare con `build.ps1` ed eseguire i test. Poi controllare la demo e la notifica sopra Cemu, anche a schermo intero, il jingle, i tasti e la scala DPI. Controllare anche Giochi, pausa prima del caricamento, ripresa senza sblocchi retroattivi e una nuova soglia raggiunta dopo tre campioni sotto soglia. Le prove cloud non sostituiscono la sessione reale.
1. **Stato della partita:** identificare menu iniziale, caricamento, slot selezionato, modalità normale/eroe e sessione giocabile. Questa è una priorità prima di attribuire obiettivi di storia.
2. **Inventario iniziale:** acquisizione di cannocchiale, spada e scudo. Confrontare campioni prima e dopo ogni acquisizione, quindi verificare che i flag rimangano coerenti nei menu, nei cambi di area e dopo un riavvio.
3. **Progressione:** vela, Wind Waker, strumenti dei dungeon e boss, con condizioni che distinguano l'evento appena avvenuto dal caricamento di dati già completati.
4. **Set curato:** aggiungere progressione, esplorazione e sfide. I traguardi numerici dell'alpha servono soprattutto a collaudare il sistema.
5. **Profili ulteriori:** altre regioni/versioni e riavvii di Cemu, con validazione esplicita degli indirizzi e delle firme.

Per le prossime prove bastano il gioco, l'app e i valori/eventi osservati dall'utente. Non servono password, chiavi della console o il trasferimento di ROM al progetto.

Per riprendere senza il contesto della chat, leggi `HANDOFF.md` e le evidenze in
`docs/VALIDATION.md`. Per contributi indipendenti usa `docs/COLLABORATION.md`.
