# Verifica delle modifiche attribuite a Grok — 29 ottobre 2026

**Aggiornamento 30 ottobre:** questo rapporto descrive il commit originale. Le
correzioni successive sono registrate in `VALIDATION.md` e `CLOUD.md`.

La copia locale è stata aggiornata a `8b292ef7c91edbb624975b2a71147b7ce9ed6d84`
con un fast-forward da `4329afacfe09dfb5d4c6749f304edf68baa10e71`.
Il commit introduce **324 righe e ne rimuove 9 in 10 file**, con un solo nuovo
file sorgente, `src/CloudSync.cs` (172 righe).

**Esito: il commit ricevuto non compila.** Le funzioni cloud sono uno scheletro
incompleto e presentano ulteriori difetti, riprodotti in isolamento dopo avere
rimosso esclusivamente una parentesi dalla copia diagnostica. Il controllo dei
caricamenti aggiunge soprattutto un messaggio: la protezione del motore contro
le regressioni esisteva già e copriva anche un singolo campo.

Questo rapporto riguarda tutto il diff tra i due commit, non attribuisce a Grok
il precedente lavoro sugli achievement. L'attribuzione a Grok 4.7 proviene
dall'utente: il commit Git riporta un autore umano e non certifica il modello.
Commit esaminato: [8b292ef su GitHub](https://github.com/hoffo1987/WiiU-Achievements/commit/8b292ef7c91edbb624975b2a71147b7ce9ed6d84).

**Sincronizzazione e tutela dei file locali.** `main` e `origin/main` puntano allo
stesso commit, con zero commit di differenza in entrambe le direzioni.
I due elenchi `ACHIEVEMENTS-DISPONIBILI.txt` e
`docs/ACHIEVEMENTS-DISPONIBILI.txt`, inizialmente non tracciati, sono rimasti
intatti: SHA-256 identico prima e dopo, con prefisso `77CC5C0E`.
Nessun eseguibile esistente è stato sostituito, nessun progresso personale è
stato modificato e nessun accesso a Cemu è stato effettuato. Il codice di
produzione resta quello ricevuto; le sole aggiunte alla copia di lavoro
principale sono la documentazione di questa verifica e i richiami nei documenti
di passaggio. Nessun nuovo commit o push è stato eseguito.

**Inventario completo del commit.**

- `.gitignore` (+3): aggiunge `data/cloud.xml`; `data/` era già ignorata.
- `HANDOFF.md` (+7): annuncia account, cloud e caricamenti automatici; inserisce
  anche un paragrafo Markdown dentro il blocco di comandi PowerShell.
- `NEXT-STEPS.md` (+7): ripete gli annunci e indica come futuro il merge cloud.
- `docs/ARCHITECTURE.md` (+26): descrive lo scheletro cloud e il controllo di
  regressione; sostiene che gli errori di rete non blocchino il gioco.
- `docs/PLAYTHROUGH.md` (+4/-1): rende facoltativo l'uso di Pausa/Scollega prima
  di caricare un salvataggio, pur ammettendo che non tutti i load siano rilevati.
- `docs/VALIDATION.md` (+9/-1): annuncia il controllo ma rinvia i test sintetici;
  spezza il titolo originale lasciando `record and reproduction` come testo.
- `src/CloudSync.cs` (+172): configurazione XML, chiamate REST di registrazione e
  login, logout, GET senza merge e PATCH della sola data `lastSync`.
- `src/Core.cs` (+15): aggiunge `PermanentRegressionCount`, senza cambiare
  `AchievementEngine.Update`, persistenza o decodifica della memoria.
- `src/MainForm.Ui.cs` (+27/-5): aggiunge due campi e tre pulsanti account,
  modifica il posizionamento dei campi e allunga le impostazioni.
- `src/MainForm.cs` (+54/-2): costruisce `CloudSync`, aggiunge il controllo
  prima di `engine.Update`, invia una richiesta per ogni sblocco e definisce i
  tre gestori account; questi ultimi sono fuori dalla classe.

**Problemi confermati, in ordine di intervento.** P1 indica un blocco o un
problema importante prima di usare la funzione; P2 un difetto da correggere.
I problemi UI/cloud descritti sotto sono latenti nel codice ricevuto: per
eseguirli è stato necessario correggere il solo errore di parentesi in una
copia diagnostica, senza correggere il repository principale.

1. **[P1] Il progetto non compila.** `src/MainForm.cs:329` chiude `MainForm`
   prima di `CloudLogin`, `CloudSignUp` e `CloudLogout` (`331–370`); rimane
   inoltre una chiusura finale in eccesso. Il compilatore ufficiale C# 5/x64
   restituisce `CS0116` a riga 331 e `CS1022` a riga 372.
   La build, i test UI e la creazione del pacchetto dal commit originale sono
   quindi bloccati. Non basta che la suite del solo motore passi.

2. **[P1] La password può comparire in chiaro nell'animazione.**
   `src/MainForm.Ui.cs:78–80` imposta la maschera sul controllo Windows, ma
   il nuovo campo viene disegnato anche tramite `DrawField` a riga 933.
   `CloseSheet` nasconde subito il TextBox mentre il pannello rimane visibile;
   il fallback a riga 963 disegna direttamente `w.Box.Text`, ignorando
   `UseSystemPasswordChar`. La riproduzione con `SYNTHETIC_ONLY` conferma che
   attivare/disattivare la maschera non cambia i pixel del fallback, mentre
   sostituire il contenuto con asterischi li cambia. Il testo originale risulta
   leggibile nell'immagine diagnostica. Occorre mascherare anche il disegno
   manuale e gestire il contenuto residuo dopo errori/logout.

3. **[P1] La rete blocca interfaccia e campionamento.**
   `src/MainForm.cs:268` chiama `cloud.Push` direttamente nel timer WinForms;
   login e registrazione fanno lo stesso dai gestori dei pulsanti.
   `src/CloudSync.cs:153–169` usa richieste sincrone con timeout di 8 secondi.
   Con un server esclusivamente locale e un ritardo di 1200 ms, la chiamata ha
   bloccato il thread per **1225–1235 ms** in due esecuzioni, senza consegnare
   alcun tick del timer
   WinForms di prova. Più achievement simultanei provocano richieste separate.
   Il motore azzera la continuità dopo un intervallo superiore a un secondo
   (`src/Core.cs:363`): il rischio di interrompere il monitoraggio deriva
   direttamente dal collegamento dei due percorsi. Non è stata simulata una
   partita reale. Il `catch` impedisce la propagazione di alcuni errori, non
   impedisce il blocco durante l'attesa.

4. **[P1 / funzione incompleta] Gli achievement non vengono sincronizzati.**
   `CloudSync.Push` (`115–128`) filtra gli ID, ma non li inserisce nella richiesta:
   il documento contiene esclusivamente `lastSync`. La maschera PATCH modifica
   soltanto quel campo, coerentemente con la
   [documentazione Firestore](https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents/patch).
   `Pull` (`94–105`) legge un documento e ne scarta il contenuto; il parametro
   `ProgressStore` non viene usato. Non esistono chiamate a `Pull` nell'app.
   Mancano invio dello storico locale, coda offline, retry e riconciliazione.
   La chiave cloud costruita dal chiamante omette inoltre il nome del profilo
   locale, presente invece nell'identità dei progressi su disco: la separazione
   di più profili nello stesso account non è stata progettata.
   Entrare in un account non crea un backup degli sblocchi e un secondo PC non
   può recuperarli. I commenti ammettono che si tratta di uno scheletro: la
   presenza dei pulsanti non dimostra una sincronizzazione funzionante.

5. **[P1] Il nuovo controllo non sostituisce Pausa prima dei caricamenti.**
   `PermanentRegressionCount` (`src/Core.cs:65–78`) conta i cali della tunica e
   dei campi di progressione, escludendo la zuppa. `MainForm.cs:259–262`
   azzera il motore quando il conteggio è almeno tre. Ma il precedente
   `RegressesFrom` (`Core.cs:57–60`), già chiamato da `Update` a riga 363,
   azzerava il motore al primo calo degli stessi campi: il nuovo caso è un
   sottoinsieme della protezione esistente. Non aggiunge uno stato di
   caricamento, identità del salvataggio o verifica che Link sia controllabile.
   Un singolo byte dell'arco che passa da `7` a `0` conta inoltre come tre
   campi distinti, quindi non sono tre segnali indipendenti. Un calo dei cuori
   massimi non entra nel conteggio. Nel fixture che passa da una partita senza
   cannocchiale a un salvataggio che lo possiede, senza alcun calo permanente,
   il nuovo controllo non interviene e il motore assegna l'obiettivo dopo tre
   campioni. È una riproduzione sintetica di un limite già esistente, non un
   nuovo esperimento live. La regressione introdotta è la rassicurazione
   all'utente: `DefaultHint` e `PLAYTHROUGH.md` rendono facoltativa una precauzione
   ancora necessaria. Va mantenuta Pausa/Scollega prima di caricare.

6. **[P2] Il parser di autenticazione rifiuta JSON valido.**
   `src/CloudSync.cs:139–146` cerca esattamente `"idToken":"` e analoghi.
   Basta uno spazio dopo i due punti perché `ParseAuth` fallisca.
   La prova con una risposta sintetica formattata, nello stile degli
   [esempi ufficiali Auth REST](https://firebase.google.com/docs/reference/rest/auth#section-sign-in-email-password),
   fallisce; la versione compatta equivalente passa. Non è stata effettuata
   una richiesta Firebase reale, quindi non si attribuisce un formato a una
   risposta live non osservata. Inoltre `Escape` a riga 136 lascia newline,
   tab e altri caratteri di controllo non codificati. Un parser/serializzatore
   JSON compatibile con .NET Framework può risolvere entrambi senza NuGet.

7. **[P2 / integrazione mancante] Nessun percorso per configurare il cloud.**
   `SignIn` e `SignUp` richiedono `ApiKey`; la configurazione arriva da un file
   privato o da `Configure`, che non è mai chiamato dall'app. Non sono presenti
   interfaccia, esempio di configurazione o procedura completa per API key,
   project ID, provider email/password e regole Firestore. Su un'installazione
   nuova i pulsanti falliscono con `Firebase API key mancante.` prima della rete.
   Non è stato ispezionato alcun eventuale file cloud personale dell'utente.

8. **[P2] Sessione e scadenza dei token non sono gestite.**
   `src/CloudSync.cs:29–38` carica il refresh token, ma nessun metodo lo usa
   per ottenere un nuovo ID token; `expiresIn` non viene letto e `IsSignedIn`
   controlla soltanto che una stringa non sia vuota. La ricostruzione
   dell'oggetto da una configurazione sintetica autenticata risulta subito
   disconnessa, pur avendo caricato il refresh token. Dopo la scadenza di un ID
   token in una sessione aperta lo stato UI può invece continuare a indicare
   connessione e gli errori di invio vengono nascosti. Firebase documenta
   scadenza e un endpoint dedicato al
   [rinnovo](https://firebase.google.com/docs/reference/rest/auth#section-refresh-token).
   Mancano anche verifiche complete di `localId`/refresh token nella risposta.

9. **[P2] Il logout lascia una credenziale riutilizzabile nel backup.**
   `SaveConfig` (`CloudSync.cs:43–50`) salva il refresh token in chiaro usando
   `SafeXml.AtomicSave`; quest'ultimo conserva il file precedente in `.bak`.
   Quando `SignOut` scrive il token vuoto, `cloud.xml.bak` conserva quello
   precedente. La prova con credenziali esclusivamente fittizie conferma che
   il file attivo è vuoto e il backup contiene ancora il vecchio token.
   Ignorare `data/` in Git evita il caricamento nel repository, ma non protegge
   queste copie sul disco. La password non viene salvata nel file XML; il
   refresh token è comunque una credenziale. Servono protezione locale adatta
   e gestione esplicita dei backup all'uscita dall'account.

10. **[P2] Il pacchetto non contiene il nuovo sorgente.**
    L'allowlist di `package.ps1:17–28` non comprende `src/CloudSync.cs`.
    Nella copia con la sola parentesi corretta il pacchetto viene prodotto e il
    manifest verifica **58 file**, ma il sorgente manca. Dopo l'estrazione la
    ricompilazione fallisce con `CS0246` sul tipo `CloudSync`. La build del
    pacchetto usa i sorgenti esterni allo staging, quindi la verifica degli
    hash non rileva questa incompletezza. Il pacchetto diagnostico è stato
    creato con `-SkipTests`, dopo il test separato del motore; non è una release
    e non viene presentato come pacchetto completo o approvato.

11. **[P2] Impostazioni con sovrapposizioni e contenuto fuori finestra.**
    `src/MainForm.Ui.cs:217–223` mette il campo email a `y=470..510` e lascia
    `Prova notifica` a `y=462..502`: si sovrappongono verticalmente per **32 px
    logici**. La sezione account è a y=460 e le etichette Email/Password sono
    disegnate dentro i campi, poi coperte dallo sfondo dei campi stessi.
    La configurazione supportata viene spostata a y=684..760: alla dimensione
    minima supportata, alta 680, il riquadro è interamente fuori finestra.
    Le note sul font si sovrappongono ad altri contenuti anche a 800 px.
    Non esiste scrolling del pannello impostazioni. Geometria e anteprime
    Windows confermano questi difetti.

12. **[P2] Campi cloud bloccati mentre si registra.**
    `src/MainForm.Ui.cs:234–245` estende il ciclo di posizionamento ai campi
    account, ma applica a tutti `box.ReadOnly = !FieldsEditable`.
    `FieldsEditable` era pensato per percorso Cemu e profilo locale: è falso
    durante una connessione o demo. La prova conferma che email e password
    diventano di sola lettura, mentre Registrati/Accedi rimangono abilitati.
    Per inserire le credenziali durante una partita occorrerebbe scollegare la
    registrazione. La Pausa da sola non rende modificabili quei campi.

**Qualità della documentazione e delle prove aggiunte.** Nessun file sotto
`tests/` è cambiato nel commit. Non sono stati aggiunti i controlli di regressione
richiesti da `AGENTS.md` per il nuovo percorso nel motore/UI. La frase “Synthetic
coverage ... can be added later” è un rinvio, non una verifica. Le precedenti
248/45 verifiche riportate nei documenti appartengono al commit antecedente e
non certificano il nuovo cloud. Sono presenti inoltre intestazioni fuori ordine,
il paragrafo cloud dentro il blocco PowerShell di `HANDOFF.md`, il titolo spezzato
in `VALIDATION.md` e otto righe con spazi finali segnalate dal controllo del diff.
La dichiarazione che gli errori di rete non interrompano il gioco è smentita dal
percorso sincrono e dalla prova del timer.

**Cosa rimane preservato nel diff.** Nessuna modifica alle tre liste di regole,
ai 70 obiettivi eseguibili, ai 40 non implementati, a rarità, punti, ID o revisione
`wwhd-alpha-1`. Nessuna modifica a indirizzi/decoder, lettore di memoria,
identificazione di Cemu, font o suoni. Nessuna dipendenza EmuHook/NuGet,
scrittura nella memoria di Cemu, chiave reale o dato personale introdotto dal
diff esaminato. La persistenza locale continua a precedere il tentativo cloud.
Il percorso cloud omette la demo, scelta corretta ma non sufficiente a renderlo
completo. Non emerge dal diff una nuova funzionalità di gioco validata sul campo.

**Verifiche effettivamente eseguite.**

- Build ufficiale del commit originale in una cartella distinta: fallita con
  `CS0116`/`CS1022`. Nessun binario esistente sovrascritto.
- `tests/run.ps1` sul commit originale: **248 controlli superati**, compreso il
  processo Win32 sintetico. Il primo tentativo confinato dal sandbox si era
  arrestato su un'operazione di file temporaneo; il tentativo con l'accesso
  necessario ha completato l'intera suite. Non è un difetto attribuito a Grok.
- Copia diagnostica dei soli file tracciati, senza `data/` o archivi storici,
  sotto `build-check/grok-review-2026-10-29/diagnostic-copy/`. Unica correzione:
  rimozione della parentesi che chiude prematuramente `MainForm`.
- Build e `tests/run-ui.ps1` della copia diagnostica: **45 controlli UI
  superati**; scrolling sintetico mediano 2,22 ms, p95 5,62 ms. Nessun test di
  questa suite esercita davvero il nuovo account.
- Harness locale `build-check/grok-review-2026-10-29/ReviewProbes.cs`:
  **20 osservazioni diagnostiche riprodotte**, comprese JSON, token/backup,
  regressioni, falso passaggio da save più avanzato, geometria, password e rete
  sul solo loopback. Le 20 osservazioni includono controlli di conferma; non
  sono 20 vulnerabilità distinte né test permanenti aggiunti al progetto.
  L'ultima esecuzione completa è salvata in
  `build-check/grok-review-2026-10-29/diagnostic-results.txt`.
- Anteprime diagnostiche Windows ispezionate visivamente; contenuti account e
  password sono tutti fittizi.
- Packaging della copia diagnostica con `-SkipTests`: manifest di 58 file
  verificato, nessuna voce di stato personale; ricompilazione dell'estratto
  fallita perché manca `CloudSync.cs`.
- Verificati uguaglianza HEAD/remoto, preservazione dei due elenchi e assenza di
  modifiche ai sorgenti di produzione nella copia di lavoro principale.

Non sono stati creati account, contattati progetti Firebase, testate regole
server, letti dump/log personali o svolti test live Cemu. Lo stato di un backend
esterno non può essere dedotto da questi file.

**Ordine di correzione proposto.** Ripristinare prima una build compilabile;
correggere mascheramento, blocchi del timer e indicazioni sui caricamenti;
decidere se mantenere l'account nascosto finché non esiste una sincronizzazione
reale oppure completarlo con configurazione, JSON, rinnovo token, coda/merge,
isolamento dei profili e prove. Poi sistemare layout, sorgenti del pacchetto e
verificare la ricompilazione dall'archivio. Queste correzioni non sono state
applicate ai sorgenti dal presente audit, per conservare il commit esaminato.
