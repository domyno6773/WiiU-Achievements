# Cloud opzionale

**Firebase attivato il 30 ottobre 2026:** progetto `wiiu-achievements`
(WiiU Achievements), Authentication email/password e Firestore Native Standard,
database `(default)` in `nam5`, free tier. Le regole personali in
`firestore.rules` sono state pubblicate e rilette dal server. Sono passati 34
controlli sull'emulatore e 9 controlli reali usando il client C# di produzione:
registrazione/accesso, upload, recupero su archivio vuoto, rinnovo dopo riavvio e
isolamento tra utenti. Account e documenti sintetici sono stati eliminati e la
rimozione verificata. L’utente ha poi confermato **Progressi sincronizzati**
nella propria app. Nessuna modifica alla fatturazione.

Per provarlo nell'app:

1. Apri la build corrente e premi **Collega Cemu** per aprire i progressi locali.
2. Premi l'ingranaggio in alto a destra, poi **Account cloud…**, accanto a
   **Prova notifica**. Se manca, apri l'eseguibile aggiornato.
3. Project ID e Web API key pubblica sono già compilati: non modificarli.
4. Inserisci email e password e premi **Registrati** per creare un account
   dell'app, oppure **Accedi** se l'hai già creato. L'account Google usato per la
   console Firebase non crea automaticamente un account email/password nell'app.
5. Attendi **Progressi sincronizzati**: sei già autenticato, non serve accedere
   di nuovo. I pulsanti Accedi/Registrati e i campi sono disabilitati durante la
   sessione; per cambiare account premi **Esci**. Usa lo stesso nome del profilo
   locale sugli altri PC e accedi lì con lo stesso account dell'app.

Il client funziona anche senza account e senza rete. La pagina account è separata
dalle impostazioni locali ed è utilizzabile mentre Cemu resta collegato. Non
condividere password o token. Una configurazione locale salvata ha precedenza sui
valori pubblici predefiniti; la Web API key non è una credenziale amministrativa.

Le regole consentono letture soltanto al proprietario autenticato sotto
`users/{proprio UID}/profiles/{hash}/unlocks/{id}`. L'hash deve contenere 64 cifre
esadecimali minuscole. Si possono creare soltanto i 70 ID eseguibili, con il solo
campo `utc` di tipo timestamp, compreso tra l'epoca Unix e l'ora server più cinque
minuti. Gli sblocchi sono immutabili: update/delete, altri campi, scritture nei
parent, accessi anonimi o tra utenti e gli altri percorsi sono negati. Quando si
aggiungono ID eseguibili, aggiornare regole e test prima del rilascio.

Sono regole di prototipo per progressi privati: rivederle prima di distribuire
ampiamente l'app. La sincronizzazione di un archivio locale non certifica che
l'obiettivo sia stato ottenuto nel gioco e non costituisce un sistema anti-cheat.

L'hash include profilo locale, fingerprint del gioco e revisione esattamente
come l'identità dei progressi locali. Gli sblocchi vengono caricati e recuperati
in background ogni 30 secondi quando è selezionata una partita reale con un
archivio locale aperto. Il login avvia un tentativo immediato se tale archivio è
presente. La demo è esclusa. Il file locale costituisce la coda persistente:
dopo errori o riavvii gli ID mancanti vengono ritentati. I documenti remoti sono
creati con precondizione di assenza, senza sovrascrivere altri dispositivi.
Il recupero importa solo ID eseguibili del set selezionato, non gli obiettivi
pianificati; conserva i timestamp e non riproduce notifiche di acquisizione.
Firestore memorizza i timestamp con precisione al microsecondo, arrotondando per
difetto le frazioni inferiori ([documentazione Firebase](https://firebase.google.com/docs/firestore/manage-data/data-types));
l’archivio locale già esistente non viene riscritto con il valore arrotondato.
Lo stato è visibile nella pagina account. Una sola operazione cloud può essere
attiva per volta. Il campionamento non aspetta la rete.

Il refresh token è cifrato con Windows DPAPI per l'utente corrente in
`data/cloud.xml`, senza backup delle credenziali. Non copiare questo file tra PC:
effettuare di nuovo l'accesso. Il logout elimina anche vecchi `.bak`. Le vecchie
configurazioni in chiaro vengono riscritte nel formato protetto; se prive di UID
richiedono un nuovo accesso. Non salvare o condividere `data/`.

I test del client coprono parser, rinnovo, recupero, upload, retry, persistenza e
isolamento dei profili. Per ripetere le 34 prove delle regole senza usare dati
reali servono Node 20+, npm e Java 21+ compatibile con Firebase Emulator Suite:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tests\test-firestore.ps1 -JavaHome "C:\percorso\jdk"
```

Il runner scarica/usa Firebase CLI, avvia solo Firestore su `127.0.0.1:8089` con
progetto `demo-wiiu-rules` e ferma l'emulatore al termine. Il test rifiuta host non
loopback e progetti non demo. Per pubblicare le regole già verificate, con un
account CLI autorizzato: `npx -y firebase-tools@latest deploy --only firestore:rules --project wiiu-achievements`.
I caricamenti del gioco sono gestiti separatamente da
[GameplayGate](AUTOMATIC-LOADS.md); non richiedono rete o login cloud.
