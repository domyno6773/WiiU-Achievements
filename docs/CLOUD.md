# Cloud opzionale

**Verifica account del 30 ottobre 2026:** trovato il progetto `wiiu-achievements`
(WiiU Achievements), con app web omonima. Authentication email/password è già
abilitato. Il database `(default)` è Firestore Native Standard, regione `nam5`,
free tier. Le regole pubblicate negano attualmente tutte le letture e scritture:
la sincronizzazione continuerà a fallire finché non saranno autorizzati i soli
percorsi personali descritti sotto. Questo controllo non ha modificato regole,
account, dati o fatturazione. Nessun test di trasferimento reale è stato eseguito.

Project ID e Web API key pubblica verificati sono ora precompilati nell'app;
non occorre copiarli dalla console. Non sono credenziali di amministrazione.
Una configurazione locale salvata ha precedenza sui valori predefiniti.

L'app funziona senza account e senza rete. Questa funzione usa Firebase
Authentication email/password e il servizio Firestore REST Native con database
`(default)`. Non crea un backend né modifica un progetto esistente.

Per abilitarla, predisporre un progetto Firebase, abilitare il provider
email/password e un database Firestore compatibile con questa REST API.
Verificare prima edizione e modalità del database; un'istanza MongoDB non è
compatibile. Recuperare Project ID e Web API key dalla configurazione pubblica
Firebase. Non usare chiavi private o credenziali di servizio.

In Impostazioni scegliere **Account cloud…**, inserire Project ID, Web API key,
email e password, poi Accedi o Registrati. La pagina è separata dalle impostazioni
locali. Il profilo locale deve avere lo stesso nome su tutti i PC.

Il backend deve consentire all'utente autenticato di leggere solo
`users/{proprio UID}/profiles/{hash}/unlocks/{id}` e di creare in quel percorso
solo documenti con un unico campo `utc` di tipo timestamp. Non consentire update,
delete, accesso anonimo o accesso ai documenti di altri utenti. Configurare e
verificare queste restrizioni nel progetto prima dell'uso. Nessuna configurazione
di sicurezza server è stata distribuita o verificata da questo repository.

L'hash include profilo locale, fingerprint del gioco e revisione esattamente
come l'identità dei progressi locali. Gli sblocchi vengono caricati e recuperati
in background ogni 30 secondi quando è selezionata una partita reale con un
archivio locale aperto. Il login avvia un tentativo immediato se tale archivio è
presente. La demo è esclusa. Il file locale costituisce la coda persistente:
dopo errori o riavvii gli ID mancanti vengono ritentati. I documenti remoti sono
creati con precondizione di assenza, senza sovrascrivere altri dispositivi.
Il recupero importa solo ID eseguibili del set selezionato, non gli obiettivi
pianificati; conserva i timestamp e non riproduce notifiche di acquisizione.
Lo stato è visibile nella pagina account. Una sola operazione cloud può essere
attiva per volta. Il campionamento non aspetta la rete.

Il refresh token è cifrato con Windows DPAPI per l'utente corrente in
`data/cloud.xml`, senza backup delle credenziali. Non copiare questo file tra PC:
effettuare di nuovo l'accesso. Il logout elimina anche vecchi `.bak`. Le vecchie
configurazioni in chiaro vengono riscritte nel formato protetto; se prive di UID
richiedono un nuovo accesso. Non salvare o condividere `data/`.

Test sintetici verificano parser, rinnovo, recupero, upload, retry, persistenza e
isolamento dei profili. Non sostituiscono un trasferimento reale con le regole del
progetto. I caricamenti del gioco sono gestiti separatamente da
[GameplayGate](AUTOMATIC-LOADS.md); non richiedono rete o login cloud.
