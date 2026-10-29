# Caricamenti automatici — Cemu 2.6 / Wind Waker HD EUR v0

La build `automatic-load-final` può restare collegata durante selezione file,
caricamenti, normali cambi d'area e riavvio di Cemu. Il primo collegamento resta
esplicito. L'app mostra **Attesa automatica** finché lo stato non è utilizzabile,
poi riprende da sola: non servono Pausa/Scollega per i passaggi verificati.
I comandi manuali rimangono disponibili come scelta dell'utente.

Il solo contatore di regressione di inventario non identifica i caricamenti.
Il nuovo lettore osserva lo stato del mondo prima e dopo la finestra dei
progressi, sempre con `host = guestBase + guest` e accesso read-only:

- guest `0x104741E4`, 26 byte: scena corrente, prossima scena e richiesta di
  transizione (`+0x18`, solo 0/1); i nomi hanno 8 byte e terminazione controllata;
- guest `0x10474BDC`, 16 byte: puntatore principale di Link e copia a `+8`,
  controllati per uguaglianza, allineamento e intervallo guest;
- scene `Name` (selezione file) e `sea_T` (sfondo titolo) sono escluse anche se
  contengono un attore Link residuo o dimostrativo.

Derivazione: [layout HD](https://github.com/ZeldaWWHDDecomp/wwhd/blob/main/wwhd_src/include/d/d_com_inf_game.h),
statico `0x1046F0B0` + play `0x12A0` + scena `0x3E94` / player `0x488C`.
Quella fonte riguarda USA v0; l'abilitazione qui si basa anche sulle osservazioni
EUR esatte descritte sotto, non sulla sola compatibilità presunta tra regioni.
Nessuna implementazione della decompilazione è stata incorporata.

Prima della ripresa servono almeno 750 ms di contesto coerente, poi valgono le
normali conferme del motore. Menu, memoria indisponibile, attore sostituito senza
una transizione osservata o campioni incoerenti azzerano le condizioni in corso;
lo stato persistito non viene eliminato. Un salvataggio già avanzato non assegna
retroattivamente i suoi oggetti. La demo non inventa questi campi.

Un normale cambio d'area con richiesta osservata mentre il mondo era valido può
conservare gli obiettivi già armati: le conferme temporanee sono azzerate e
ripartono solo dopo la stabilizzazione. Questo permette di rilevare flag
permanenti aggiornati all'uscita dall'area. Menu/titolo, dati indisponibili,
regressioni permanenti e intervalli di campionamento anomali annullano questa
continuità. Non viene conservata sulla sola base di un nome di scena plausibile.

Se Cemu termina o cambia lo spazio guest, l'app mantiene l'intenzione di
registrare e cerca ogni secondo una sola sessione dello stesso eseguibile e
profilo supportato. Riapre il lettore, controlla di nuovo log e prenotazione,
ricarica i progressi e acquisisce una baseline. **Scollega** annulla anche i
ricollegamenti in attesa e un collegamento asincrono eventualmente in corso.
Altri giochi/versioni non vengono agganciati automaticamente.

Osservazioni reali del 30 ottobre 2026, configurazione esatta:

1. Selezione file: scena `Name`, Link assente. Il caricamento ha mostrato
   richiesta `1`, destinazione `sea`, poi Link presente in due riferimenti.
2. Ripetizione dopo riavvio: stessi segnali e prenotazione/log ricontrollati.
   Lettore e motore nuovi: Menu → Loading → Ready/stabilizzazione → Ready,
   zero sblocchi retroattivi su salvataggio con 15 rupie.
3. Inventario aperto/chiuso dall'utente, poi ingresso in una casa:
   `sea` → Loading → `Omasao` → ripresa automatica, ancora zero sblocchi retroattivi.
4. App completa con audio disattivato e profilo di test isolato: perdita della
   sessione → attesa/ricollegamento → titolo → selezione → partita `sea`,
   ripresa automatica e zero award. Lo sfondo `sea_T` ha evidenziato un attore
   dimostrativo: esclusione esplicita aggiunta e coperta da regressione sintetica.

Le prove hanno letto solo i campi necessari. Nessun dump, log Cemu o salvataggio
è stato salvato nel progetto. I progressi reali non sono stati modificati.

Limiti: polling nominale 100 ms, senza eventi frame-perfect. Morte/continua,
partite nuove/eroe, cambio tra due slot differenti, transizioni di fine gioco e
un playthrough completo non sono ancora verificati live. L'attesa è conservativa
quando manca un segnale: non presume uno stato giocabile dai soli cuori/rupie.
I 53 detector di progressione sperimentali restano tali; queste prove non ne
certificano gli offset né gli eventi di acquisizione.
