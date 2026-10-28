# Wii U Achievements — alpha 0.1

Applicazione Windows indipendente, scritta da zero. Primo gioco: **The Legend of Zelda: The Wind Waker HD**, edizione EUR v0, su **Cemu 2.6**.

## Per altre AI o sviluppatori

Inizia da [HANDOFF.md](HANDOFF.md): contiene il contesto completo, i vincoli,
i file da leggere, i comandi e il prossimo lavoro. [AGENTS.md](AGENTS.md) contiene
le istruzioni per gli agenti. La documentazione tecnica in `docs/` è in inglese
per facilitare il passaggio tra strumenti; non serve la chat originale.

Per creare un archivio condivisibile con sorgenti, eseguibile, test, documentazione
e hash dei file, usa `powershell -NoProfile -ExecutionPolicy Bypass -File .\package.ps1`. Il pacchetto
esclude impostazioni personali e progressi. Dopo l'estrazione, verifica i file
con `powershell -NoProfile -ExecutionPolicy Bypass -File .\verify-package.ps1`.

L'app rimane **Windows x64**: trasportabile tra cartelle e PC Windows, con la
configurazione del gioco supportata. Su Linux/macOS un'altra AI può lavorare sui
sorgenti, ma deve lasciare le prove Win32 e sul gioco a un ambiente Windows.

## Avvio

1. Avvia il gioco in Cemu e carica la partita.
2. Apri `WiiUAchievements.exe` dalla stessa cartella di `achievements.xml`.
3. Seleziona il percorso di `Cemu.exe`, scegli un profilo locale e premi **Collega Cemu**.
4. Lascia aperta l'app mentre giochi. Puoi ridurla a icona.

Nella copia locale il percorso della tua installazione è già salvato. Il pacchetto
condivisibile parte senza impostazioni personali: su un altro PC seleziona Cemu.
Il profilo iniziale è **Giocatore 1**. La cartella dell'app deve essere scrivibile.

Non servono Python, AutoHotkey, EmuHook, un account o una connessione Internet. L'eseguibile usa .NET Framework 4.x disponibile su questo PC. La lettura del processo deve avvenire con gli stessi privilegi di Cemu; normalmente basta il doppio clic, senza avvio come amministratore.

## Cosa funziona

- Identificazione di Cemu e del gioco tramite log della sessione, versione, Title ID e hash RPX.
- Individuazione automatica della base di memoria Wii U, anche quando cambia tra gli avvii.
- Lettura in tempo reale di rupie, vita attuale e capacità dei cuori.
- Dieci obiettivi iniziali definiti nel file XML, su rupie e capacità di vita.
- Notifiche desktop con suono opzionale, senza prendere il focus del gioco.
- Punti locali e progressi salvati tra le sessioni.
- Demo con dati e progressi separati da quelli reali.

**È una prima alpha funzionante, non un set completo per Wind Waker HD.** Inventario, storia, dungeon, boss, collezionabili e sfide senza danni sono ancora da sviluppare e validare.

## Configurazione supportata

- Cemu: **2.6**, Windows x64.
- Title ID: **0005000010143600**.
- Versione del titolo: **v0**.
- RPX hash nel log: **cdc68acd**.
- Profilo di memoria: header dei dati di gioco all'indirizzo Wii U **0x145B7B80**.
- Layout: capacità cuori `+0x00` (u16 BE), vita `+0x02` (u16 BE), rupie `+0x04` (u16 BE). La vita usa quarti di cuore.

Il profilo è stato controllato sulla sessione reale dell'utente: **0 → 5 → 15 rupie**, vita **12/12** quarti. Lo sblocco **Primi spiccioli** è stato confermato anche dall'utente durante il gioco. Gli altri traguardi non sono stati tutti raggiunti in gioco durante questa verifica.

La lettura di sei byte non è una fotografia atomica dell'intero gioco. Il motore richiede tre campioni consecutivi validi, a circa 10 Hz. È adatto a questi traguardi persistenti; non garantisce il rilevamento di eventi brevi o di ogni frame.

## Regole e progressi

All'avvio viene acquisita una baseline: un obiettivo già soddisfatto non si sblocca automaticamente caricando una partita avanzata. Il valore deve prima risultare inferiore alla soglia e poi raggiungerla durante il monitoraggio. Ad esempio, **Primi spiccioli** richiede il passaggio da meno di 10 ad almeno 10 rupie nel portafoglio.

Una disconnessione, un campione non valido o una pausa di campionamento superiore a un secondo azzerano le condizioni in corso. Le ricompense già salvate restano disponibili.

I progressi sono in `data/progress/`, separati per nome del profilo, titolo, versione, hash RPX e revisione del set. La scrittura usa un file temporaneo e la sostituzione atomica; il precedente file rimane come backup `.bak`. Se il file è danneggiato, viene conservato prima del recupero dal backup.

Per una nuova partita scegli un altro profilo locale. **Questa alpha non riconosce ancora automaticamente il file di salvataggio selezionato né il passaggio tra salvataggi nello stesso gioco.** Caricare una partita differente durante la registrazione può essere interpretato come un cambiamento di progresso: scollega prima l'app e ricollegala dopo il caricamento. Non è presente una modalità hardcore o una verifica anticheat. I punti sono esclusivamente locali e non vengono inviati a RetroAchievements.

Le notifiche sono finestre desktop: in alcune modalità di schermo intero potrebbero non apparire sopra il gioco. L'obiettivo viene comunque salvato e mostrato nell'app. La prova di notifica è riuscita nella configurazione corrente dell'utente.

## Sviluppo

Il codice è in `src/`. È C# 5 / .NET Framework x64, senza pacchetti di terze parti.

Per ricompilare, chiudi l'app e avvia da PowerShell:

```powershell
.\build.ps1
```

Per compilare senza sovrascrivere l'app aperta, usa
`.\build.ps1 -OutputDirectory .\build-check`. Anche `achievements.xml` viene
copiato accanto all'eseguibile nella cartella di destinazione.

Per i test:

```powershell
.\tests\run.ps1
```

I test coprono byte order, valori impossibili, baseline, transizioni stabili, spike, interruzioni, persistenza, profili distinti, recupero da corruzione e integrazione Win32 con un processo figlio sintetico che riserva 4 GiB. Nessun test scrive nella memoria di Cemu.

Il lettore di produzione apre il processo con `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ`. Non contiene API di scrittura, injection o modifica dei salvataggi. Il rilevamento della base segue la struttura di allocazione documentata dal codice di Cemu. Lo scanner di calibrazione è disponibile nel codice e verificato dai test, ma non è ancora esposto nell'interfaccia: questa prima build usa il profilo EUR confermato.

## Riferimenti tecnici

- [Cemu 2.6 — MMU.cpp](https://github.com/cemu-project/Cemu/blob/v2.6/src/Cafe/HW/MMU/MMU.cpp): riserva contigua di 4 GiB e traduzione degli indirizzi guest.
- [Mappatura pubblica dei dati di Wind Waker HD](https://github.com/jamesst20/ZeldaWindWakerTrainer-for-CEMU/blob/master/ZeldaWindWakerTrainer/GameOffsets.cs): riferimento per gli indirizzi iniziali di cuori e rupie, verificati poi sulla partita reale.

Il programma è un'implementazione originale. Non incorpora EmuHook, altri trainer, codice di gioco o file di Nintendo.

Per trasferire i tuoi progressi, chiudi l'app e copia separatamente
`data/progress/` nella nuova cartella. Mantieni lo stesso nome del profilo locale.
Non serve copiare `data/settings.xml`, che contiene il percorso di Cemu del PC
precedente. Non è ancora stata scelta una licenza di redistribuzione pubblica.
