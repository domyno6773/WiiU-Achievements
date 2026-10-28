# Wii U Achievements — alpha 0.3

Applicazione Windows indipendente, scritta da zero. Primo gioco: **The Legend of Zelda: The Wind Waker HD**, edizione EUR v0, su **Cemu 2.6**.

## Repository e copia locale

La repository di riferimento è [hoffo1987/WiiU-Achievements](https://github.com/hoffo1987/WiiU-Achievements).
Usa un clone Git per proseguire lo sviluppo; gli archivi delle alpha precedenti
sono fotografie storiche e possono contenere sorgenti meno recenti.

Prima di lavorare esegui `git pull --ff-only`. Dopo le modifiche esegui build e
test indicati sotto, controlla `git diff` e `git status`, quindi crea un commit
e invialo con `git push`. Se il pull segnala una divergenza, integra le modifiche
senza sovrascrivere la cronologia con un force push.

`data/` rimane locale: contiene impostazioni e progressi e non viene pubblicata.
L'eseguibile si ricompila con `build.ps1`; gli archivi da distribuire si generano
con `package.ps1`. Conserva il font e la relativa licenza presenti nella repo.

![Finestra principale](docs/screenshots/main.png)

**Novità dell’alpha 0.3:** sezione **Giochi** predisposta per nuovi titoli,
con avviso esplicito per quelli non disponibili; comando **Pausa / Riprendi**;
verifica più robusta delle soglie e salvataggio atomico degli sblocchi simultanei.
Gli obiettivi restano gli stessi dieci e i progressi esistenti mantengono la loro identità.
Grafica, banner, medaglie e suoni dell’alpha 0.2 sono conservati.

Apri **Giochi** dalla barra superiore (oppure premi **G**). Per ora è disponibile
solo Wind Waker HD nella configurazione indicata sotto. La voce **Altri giochi**
è disabilitata: non contiene set pronti. I dettagli per aggiungere un gioco sono
in [docs/GAMES.md](docs/GAMES.md).

**Novità dell'alpha 0.2:** l'interfaccia è stata ridisegnata nello stile dei menu di sistema Wii U, con animazioni:

- banner del gioco con mare in cel-shading animato;
- anello di completamento e medaglie bronzo/argento/oro/platino;
- schede degli obiettivi con barra di avanzamento e cursore di selezione azzurro;
- notifica di sblocco animata, con un jingle originale.

Tutta la grafica è disegnata dal codice, senza file Nintendo. I dettagli tecnici sono in [docs/UI.md](docs/UI.md).

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
3. Apri **Impostazioni** (icona a ingranaggio in alto a destra). Seleziona il percorso di `Cemu.exe` e scegli un profilo locale. Poi chiudi il pannello con **Indietro** e premi **Collega Cemu**.
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
- Notifica di sblocco animata in alto al centro, sullo schermo dove si trova Cemu. Non prende il focus del gioco e lascia passare i clic. Il jingle si può disattivare.
- Interfaccia in stile Wii U. Filtri: tutti, sbloccati, da sbloccare. Facendo clic su un obiettivo si apre la scheda di dettaglio. Il registro attività si apre dall'icona a elenco.
- Punti locali e progressi salvati tra le sessioni.
- Demo con dati e progressi separati da quelli reali.

## Interfaccia e comandi

- Su ogni scheda c'è lo stato dell'obiettivo: sbloccato con data, avanzamento in tempo reale, oppure «Soglia già raggiunta all’avvio». In quest'ultimo caso l'obiettivo non si sblocca finché il valore non rimane sotto la soglia per tre campioni validi e poi la raggiunge per altri tre campioni.
- **Impostazioni:** percorso di Cemu, profilo locale e suono di sblocco. Contiene anche le animazioni del banner e il pulsante **Prova notifica**, utile per controllare che la notifica compaia sopra il gioco nella tua modalità a schermo intero.
- Le animazioni di sfondo si fermano quando la finestra non è attiva, quindi non consumano CPU mentre giochi.

| Tasto | Azione |
| --- | --- |
| Frecce / Tab | Spostano il cursore azzurro tra schede e pulsanti |
| Invio / Spazio | Attivano l'elemento selezionato |
| A | Collega Cemu |
| X | Prova demo |
| Y | Scollega |
| P | Pausa / Riprendi la registrazione |
| G | Apre / chiude la sezione Giochi |
| B / Esc | Chiude il pannello aperto o torna agli obiettivi da Giochi |

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

All'avvio viene acquisita una baseline: un obiettivo già soddisfatto non si sblocca automaticamente caricando una partita avanzata. Il valore deve prima restare inferiore alla soglia per tre campioni consecutivi validi e poi raggiungerla per altri tre campioni durante il monitoraggio. Un singolo valore basso anomalo non basta ad abilitare lo sblocco. Ad esempio, **Primi spiccioli** richiede il passaggio da meno di 10 ad almeno 10 rupie nel portafoglio.

Una disconnessione, un campione non valido o una pausa di campionamento superiore a un secondo azzerano le condizioni in corso. Le ricompense già salvate restano disponibili.

I progressi sono in `data/progress/`, separati per nome del profilo, titolo, versione, hash RPX e revisione del set. Gli obiettivi maturati nello stesso campione vengono salvati insieme, prima di notificare qualsiasi sblocco. La scrittura usa un file temporaneo e la sostituzione atomica; il precedente file rimane come backup `.bak`. Se il file è danneggiato, viene conservato prima del recupero dal backup.

Per una nuova partita scegli un altro profilo locale. **Questa alpha non riconosce ancora automaticamente il file di salvataggio selezionato né il passaggio tra salvataggi nello stesso gioco.** Caricare una partita differente durante la registrazione può essere interpretato come un cambiamento di progresso: metti in pausa prima del caricamento e riprendi dopo essere tornato nel gioco, oppure scollega e ricollega l'app. **Pausa** ferma soltanto la registrazione degli achievement: il gioco continua. Alla ripresa vengono azzerate le condizioni in corso e acquisito un nuovo stato iniziale; gli sblocchi salvati rimangono. La pausa non riconosce automaticamente menu o cambi di salvataggio. Non è presente una modalità hardcore o una verifica anticheat. I punti sono esclusivamente locali e non vengono inviati a RetroAchievements.

Le notifiche sono finestre desktop: in alcune modalità di schermo intero potrebbero non apparire sopra il gioco. L'obiettivo viene comunque salvato e mostrato nell'app. La prova di notifica è riuscita nella configurazione corrente dell'utente.

## Sviluppo

Il codice è in `src/`. È C# 5 / .NET Framework x64, senza pacchetti NuGet. L'interfaccia è in `MainForm.Ui.cs`, `MainForm.Library.cs`, `Theme.cs`, `Banner.cs` e `Notification.cs`. Il font Nunito di `assets/fonts/` viene incorporato nell'eseguibile da `build.ps1`.

L'eseguibile del precedente pacchetto alpha 0.2 è stato compilato con Mono (`mcs`, livello C# 5) su Linux e non è ancora stato provato su Windows. Se non parte o la grafica non è corretta, ricompila con `build.ps1` (vedi sotto). È la via di compilazione ufficiale.

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

La suite ora distingue i test portabili del motore dalle prove native Win32. Su Linux/macOS con Mono usa `bash tests/run.sh`; le prove Win32 vengono saltate esplicitamente. Per esercitare interfaccia, catalogo e pausa in una cartella temporanea su Windows, usa `powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\run-ui.ps1`.

Il workflow GitHub Actions compila e prepara il pacchetto su Windows, esegue la suite completa e i controlli dell’interfaccia senza Cemu. Il suo esito va letto sulla singola esecuzione: non certifica il funzionamento nel gioco.

La prima verifica dell’alpha 0.3 su Windows è riuscita: **50 controlli sul motore e sulle API Win32, 22 sull’interfaccia**, pacchetto con 47 file verificati. L’eseguibile Windows e le anteprime si trovano negli artifact della [verifica su GitHub](https://github.com/hoffo1987/WiiU-Achievements/actions/runs/40764665750). Restano da provare gli sblocchi reali e pausa/caricamento/ripresa su Cemu.

I test coprono byte order, valori impossibili, baseline, transizioni stabili, spike, interruzioni, persistenza, profili distinti, recupero da corruzione e integrazione Win32 con un processo figlio sintetico che riserva 4 GiB. Nessun test scrive nella memoria di Cemu.

Il lettore di produzione apre il processo con `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ`. Non contiene API di scrittura, injection o modifica dei salvataggi. Il rilevamento della base segue la struttura di allocazione documentata dal codice di Cemu. Lo scanner di calibrazione è disponibile nel codice e verificato dai test, ma non è ancora esposto nell'interfaccia: questa prima build usa il profilo EUR confermato.

## Riferimenti tecnici

- [Cemu 2.6 — MMU.cpp](https://github.com/cemu-project/Cemu/blob/v2.6/src/Cafe/HW/MMU/MMU.cpp): riserva contigua di 4 GiB e traduzione degli indirizzi guest.
- [Nunito](https://github.com/googlefonts/nunito): font usato dall'interfaccia, © The Nunito Project Authors, SIL Open Font License 1.1 (`assets/fonts/OFL.txt`).
- [Mappatura pubblica dei dati di Wind Waker HD](https://github.com/jamesst20/ZeldaWindWakerTrainer-for-CEMU/blob/master/ZeldaWindWakerTrainer/GameOffsets.cs): riferimento per gli indirizzi iniziali di cuori e rupie, verificati poi sulla partita reale.

Il programma è un'implementazione originale. Non incorpora EmuHook, altri trainer, codice di gioco o file di Nintendo. Lo stile grafico richiama i menu Wii U, ma icone, medaglie, banner e suono sono creati dal codice.

Per trasferire i tuoi progressi, chiudi l'app e copia separatamente
`data/progress/` nella nuova cartella. Mantieni lo stesso nome del profilo locale.
Non serve copiare `data/settings.xml`, che contiene il percorso di Cemu del PC
precedente. Non è ancora stata scelta una licenza di redistribuzione pubblica.
