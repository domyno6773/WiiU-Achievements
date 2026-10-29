using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace WiiUAchievements
{
    // Session logic. The custom-drawn interface lives in MainForm.Ui.cs.
    public sealed partial class MainForm : Form
    {
        enum StatusKind { Idle, Busy, Live, Demo, Paused, Error }
        sealed class PendingToast { public Achievement Achievement; public string Header; public bool Retry; }

        readonly TextBox cemuPath = new TextBox(), player = new TextBox();
        readonly System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
        readonly Queue<PendingToast> toasts = new Queue<PendingToast>();
        readonly List<string> logLines = new List<string>();
        readonly string root, dataRoot;
        List<Achievement> rules;
        GameDefinition selectedGame = GameCatalog.Default;
        GameProfile activeProfile;
        IGameMemoryReader gameReader;
        readonly Dictionary<string, Art> arts = new Dictionary<string, Art>();
        ProcessMemory memory;
        GameSession session;
        ProgressStore progress;
        AchievementEngine engine;
        GameSnapshot current;
        ulong guestBase;
        DateTime identityCheck;
        bool demo, recording, connecting, paused, soundOn = true, ambientOn = true;
        int engineStateVersion;
        int demoTick;
        ToastForm activeToast;
        string statusTitle = "", statusDetail = "", hint = "";
        StatusKind statusKind;

        public MainForm(bool startDemo, bool autoConnect)
        {
            root = AppDomain.CurrentDomain.BaseDirectory;
            dataRoot = Path.Combine(root, "data");
            rules = selectedGame.LoadRules(root);
            foreach (var a in rules) arts[a.Id] = Art.For(a, rules);
            Text = "Wii U Achievements — " + selectedGame.Title;
            StartPosition = FormStartPosition.CenterScreen;
            AutoScaleMode = AutoScaleMode.None;
            BackColor = Theme.BackBottom; ForeColor = Theme.Ink;
            BuildUi();
            ClientSize = new Size((int)(1120 * scale), (int)(800 * scale));
            MinimumSize = SizeFromClientSize(new Size((int)(900 * scale), (int)(680 * scale)));
            cemuPath.Text = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Cemu_2.6", "Cemu.exe");
            player.Text = "Giocatore 1";
            LoadSettings();
            player.TextChanged += delegate
            {
                if (!FieldsEditable) return;
                progress = null; engine = null; current = null;
                RefreshList();
            };
            timer.Interval = 100;
            timer.Tick += Tick;
            Shown += delegate { if (startDemo) StartDemo(); else if (autoConnect) Connect(); };
            FormClosing += delegate { timer.Stop(); if (memory != null) memory.Dispose(); if (activeToast != null) activeToast.Close(); };
            RefreshList();
            hint = DefaultHint;
            SetStatus("Pronto al collegamento", "Avvia il gioco e carica un salvataggio, poi premi Collega Cemu.", StatusKind.Idle);
        }
        void LoadSettings()
        {
            string path = Path.Combine(dataRoot, "settings.xml");
            if (!File.Exists(path)) return;
            try
            {
                var doc = SafeXml.Load(path);
                cemuPath.Text = (string)doc.Root.Element("cemu") ?? cemuPath.Text; player.Text = (string)doc.Root.Element("player") ?? player.Text;
                soundOn = (string)doc.Root.Element("sound") != "false"; ambientOn = (string)doc.Root.Element("ambient") != "false";
                var savedGame = GameCatalog.Games.FirstOrDefault(g => g.Id == (string)doc.Root.Element("game") && g.Available);
                if (savedGame != null && savedGame != selectedGame) ApplyGameSelection(savedGame);
            }
            catch (Exception ex) { Log("Impostazioni non caricate: " + ex.Message); }
        }
        void SaveSettings()
        {
            SafeXml.AtomicSave(new XDocument(new XElement("settings", new XElement("cemu", cemuPath.Text), new XElement("player", player.Text), new XElement("game", selectedGame.Id),
                new XElement("sound", soundOn ? "true" : "false"), new XElement("ambient", ambientOn ? "true" : "false"))), Path.Combine(dataRoot, "settings.xml"));
        }
        void SetStatus(string title, string subtitle, StatusKind kind)
        {
            statusTitle = title; statusDetail = subtitle; statusKind = kind;
            AccessibleDescription = title + ". " + subtitle;
            Invalidate(); Kick();
        }
        void Log(string message)
        {
            logLines.Add(DateTime.Now.ToString("HH:mm:ss") + "  " + message);
            if (logLines.Count > 500) logLines.RemoveAt(0);
            if (drawerTarget == 0) unreadLogs++;
            Invalidate();
        }
        void SetConnected(bool value)
        {
            player.ReadOnly = value; cemuPath.ReadOnly = value;
            PlaceFields(); Invalidate(); Kick();
        }
        const string DefaultHint = "Prima di caricare un altro salvataggio, metti in pausa o scollega la registrazione.";
        bool FieldsEditable { get { return memory == null && !demo && !connecting; } }
        bool CanConnect { get { return !connecting && memory == null && !demo; } }
        bool CanDisconnect { get { return memory != null || demo; } }

        async void Connect()
        {
            if (connecting || memory != null || demo) return;
            connecting = true; SetConnected(true);
            SetStatus("Collegamento…", "Identificazione della sessione e verifica dello spazio Wii U.", StatusKind.Busy);
            ProcessMemory attached = null;
            try
            {
                if (String.IsNullOrWhiteSpace(player.Text)) throw new InvalidOperationException("Inserisci un nome per il profilo locale.");
                string path = Path.GetFullPath(cemuPath.Text.Trim().Trim('"'));
                cemuPath.Text = path;
                var processes = Process.GetProcessesByName("Cemu").Where(p => SamePath(p, path)).ToList();
                if (processes.Count != 1) throw new InvalidOperationException("Deve essere aperta una sola sessione Cemu dal percorso selezionato.");
                session = GameSession.FromLog(path, processes[0].Id);
                activeProfile = selectedGame.RequireProfile(session);
                gameReader = activeProfile.CreateReader();
                attached = new ProcessMemory(session.Pid);
                var bases = await Task.Run(() => attached.FindGuestBases());
                if (IsDisposed || Disposing) { attached.Dispose(); attached = null; return; }
                if (!bases.Contains(session.GuestBase)) throw new InvalidOperationException("Il log e la memoria del processo non corrispondono. Ricollega dopo l'avvio del gioco.");
                guestBase = session.GuestBase;
                var sample = gameReader.Read(attached, guestBase);
                progress = new ProgressStore(Path.Combine(dataRoot, "progress"), player.Text, session.Fingerprint, selectedGame.SetRevision);
                engine = new AchievementEngine(rules, progress);
                engine.Update(sample);
                SaveSettings();
                memory = attached; attached = null; demo = false; paused = false; recording = true;
                identityCheck = DateTime.MinValue;
                connecting = false;
                SetConnected(true); UpdateStats(sample); RefreshList(); timer.Start();
                SetRecordingStatus();
                hint = DefaultHint;
                Log(selectedGame.Title + " · " + activeProfile.Label + " riconosciuta. " + sample.Rupees + " rupie, " + (sample.Health / 4m) + "/" + (sample.MaxHealth / 4m) + " cuori.");
                if (progress.Warning != null) Log(progress.Warning);
            }
            catch (Exception ex)
            {
                if (attached != null) attached.Dispose();
                engine = null; progress = null; current = null; gameReader = null; activeProfile = null;
                connecting = false;
                SetConnected(false);
                SetStatus("Collegamento non riuscito", ex.Message, StatusKind.Error); Log(ex.Message);
            }
            finally { connecting = false; Invalidate(); }
        }
        static bool SamePath(Process process, string expected)
        {
            try { return String.Equals(Path.GetFullPath(process.MainModule.FileName), expected, StringComparison.OrdinalIgnoreCase); }
            catch { return false; }
        }
        void Disconnect(string reason)
        {
            // Demo unlocks live in a throwaway store and should not linger on screen.
            if (demo) progress = null;
            timer.Stop(); recording = false; demo = false; paused = false;
            if (memory != null) { memory.Dispose(); memory = null; }
            if (engine != null) engine.Reset();
            gameReader = null; activeProfile = null;
            toasts.Clear(); if (activeToast != null) { activeToast.Close(); activeToast = null; }
            current = null; hint = DefaultHint;
            SetConnected(false); SetStatus("Scollegato", reason, StatusKind.Idle); Log(reason);
        }
        void StartDemo()
        {
            if (memory != null || demo || connecting) return;
            try
            {
                // Demo has its own temporary directory, never the real player's progress.
                string temporary = Path.Combine(dataRoot, "demo", Guid.NewGuid().ToString("N"));
                progress = new ProgressStore(temporary, "DEMO", "simulation|" + selectedGame.Id, selectedGame.SetRevision); engine = new AchievementEngine(rules, progress);
                current = null; demo = true; paused = false; recording = true; demoTick = 0;
                SetConnected(true); RefreshList();
                SetRecordingStatus();
                hint = "La demo aumenta le rupie automaticamente. Premi Scollega per tornare al gioco reale.";
                timer.Start(); Log("Demo avviata: nessun collegamento a Cemu.");
            }
            catch (Exception ex) { Log(ex.Message); }
        }
        void SetRecordingStatus()
        {
            if (demo) SetStatus("DEMO · simulazione", "Dati simulati. I progressi della tua partita restano separati.", StatusKind.Demo);
            else SetStatus("Collegato · registrazione attiva", selectedGame.Title + " · " + activeProfile.Label + " · progressi locali", StatusKind.Live);
        }
        void TogglePause()
        {
            if (!CanDisconnect || connecting) return;
            paused = !paused; recording = !paused;
            engine.Reset(); current = null; identityCheck = DateTime.MinValue;
            if (paused)
            {
                timer.Stop();
                toasts.Clear(); if (activeToast != null) { activeToast.Close(); activeToast = null; }
                SetStatus("Registrazione in pausa", "Carica la partita, poi premi Riprendi. Gli obiettivi già soddisfatti non verranno assegnati.", StatusKind.Paused);
                hint = "La pausa ferma gli achievement, non il gioco. Per una nuova partita usa un altro profilo locale.";
            }
            else { timer.Start(); SetRecordingStatus(); hint = DefaultHint; }
            Log(paused ? "Registrazione in pausa: condizioni in corso azzerate." : "Registrazione ripresa con una nuova baseline.");
            RefreshList();
        }
        void SelectGame(GameDefinition game)
        {
            if (!game.Available || (game != selectedGame && !CanConnect)) return;
            if (game != selectedGame)
            {
                ApplyGameSelection(game); TrySaveSettings();
                SetStatus("Pronto al collegamento", "Avvia il gioco selezionato nella configurazione supportata.", StatusKind.Idle);
            }
            CloseLibrary(); RefreshList();
        }
        void ApplyGameSelection(GameDefinition game)
        {
            // Load successfully before changing selection. Existing progress on disk is untouched.
            var loaded = game.LoadRules(root);
            var loadedArt = loaded.ToDictionary(rule => rule.Id, rule => Art.For(rule, loaded));
            ClearTileImages();
            selectedGame = game; rules = loaded; progress = null; engine = null; current = null;
            arts.Clear(); foreach (var pair in loadedArt) arts.Add(pair.Key, pair.Value);
            celebrations.Clear(); filter = categoryFilter = chapterFilter = 0; scroll = scrollTarget = 0; tabX = -1;
            Text = "Wii U Achievements — " + game.Title;
        }
        void Tick(object sender, EventArgs e)
        {
            if (!recording || engine == null) return;
            try
            {
                GameSnapshot sample;
                if (demo)
                {
                    demoTick++;
                    sample = new GameSnapshot { MaxHealth = demoTick < 180 ? 12 : 16, Health = 12, Rupees = Math.Min(1200, (demoTick / 20) * 5), Utc = DateTime.UtcNow };
                }
                else
                {
                    if ((DateTime.UtcNow - identityCheck).TotalSeconds >= 1)
                    {
                        var live = GameSession.FromLog(session.CemuPath, session.Pid);
                        if (live.Fingerprint != session.Fingerprint || live.GuestBase != guestBase) throw new InvalidOperationException("Il gioco o la sessione Cemu sono cambiati. Ricollega l'app.");
                        identityCheck = DateTime.UtcNow;
                    }
                    sample = gameReader.Read(memory, guestBase);
                }
                foreach (var a in engine.Update(sample))
                {
                    Log((demo ? "[DEMO] " : a.Experimental ? "[TEST] " : "") + "Sbloccato: " + a.Title + " (+" + a.Points + ")");
                    toasts.Enqueue(new PendingToast { Achievement = a, Header = demo ? "DEMO · OBIETTIVO SBLOCCATO" : a.Experimental ? "SBLOCCATO · SPERIMENTALE" : "OBIETTIVO SBLOCCATO" });
                    Celebrate(a); RefreshList();
                }
                UpdateStats(sample);
                PumpToasts();
            }
            catch (Exception ex)
            {
                // Invalid samples/disconnections cannot carry an armed condition across a gap.
                Disconnect("Lettura sospesa: " + ex.Message);
            }
        }
        void PumpToasts()
        {
            if (activeToast != null || toasts.Count == 0 || IsDisposed) return;
            var next = toasts.Dequeue();
            ToastForm toast;
            // A popup failure must not stop recording: the unlock is already saved and listed.
            try { toast = new ToastForm(next.Achievement, arts[next.Achievement.Id], next.Header, ToastScreen()); }
            catch (Exception ex) { Log("Notifica non mostrata: " + ex.Message); return; }
            toast.FormClosed += delegate
            {
                if (activeToast == toast) activeToast = null;
                // A rejected layered window falls back to the plain popup for the same unlock.
                if (toast.Failed && !next.Retry && !paused && !IsDisposed && !Disposing)
                { next.Retry = true; var retry = new Queue<PendingToast>(new[] { next }.Concat(toasts)); toasts.Clear(); foreach (var p in retry) toasts.Enqueue(p); }
                if (!IsDisposed && !Disposing && IsHandleCreated) BeginInvoke(new Action(PumpToasts));
            };
            activeToast = toast;
            try { toast.Show(); }
            catch (Exception ex) { activeToast = null; toast.Dispose(); Log("Notifica non mostrata: " + ex.Message); return; }
            if (soundOn && !next.Retry) Chime.Play();
        }
        Screen ToastScreen()
        {
            try
            {
                if (session != null && memory != null)
                {
                    IntPtr window = Process.GetProcessById(session.Pid).MainWindowHandle;
                    if (window != IntPtr.Zero) return Screen.FromHandle(window);
                }
            }
            catch (Exception) { }
            return Screen.FromControl(this);
        }
        void UpdateStats(GameSnapshot s)
        {
            bool changed = current == null || current.Rupees != s.Rupees || current.Health != s.Health || current.MaxHealth != s.MaxHealth || s.ProgressionDiffersFrom(current);
            if (engine != null && engineStateVersion != engine.StateVersion) { changed = true; engineStateVersion = engine.StateVersion; }
            current = s;
            if (changed) { Invalidate(); Kick(); }
        }
        int ActiveCount { get { return rules.Count(a => !a.IsPlanned); } }
        int UnlockedCount { get { return rules.Count(a => !a.IsPlanned && progress != null && progress.Contains(a.Id)); } }
        int EarnedPoints { get { return rules.Where(a => !a.IsPlanned && progress != null && progress.Contains(a.Id)).Sum(a => a.Points); } }
        void RefreshList()
        {
            Invalidate(); Kick();
        }
    }
}
