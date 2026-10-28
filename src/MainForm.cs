using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Media;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace WiiUAchievements
{
    public sealed class MainForm : Form
    {
        readonly Color background = Color.FromArgb(16, 24, 35), surface = Color.FromArgb(25, 37, 51),
            ink = Color.FromArgb(231, 239, 244), muted = Color.FromArgb(157, 178, 192), accent = Color.FromArgb(89, 222, 187);
        readonly TextBox cemuPath = new TextBox(), player = new TextBox();
        readonly Label status = new Label(), detail = new Label(), rupees = new Label(), hearts = new Label(), points = new Label(), count = new Label();
        readonly Label hint = new Label();
        readonly Button connect = new Button(), disconnect = new Button(), demoButton = new Button();
        readonly Button browse = new Button();
        readonly ListView list = new ListView();
        readonly TextBox log = new TextBox();
        readonly CheckBox sound = new CheckBox();
        readonly System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
        readonly Queue<Achievement> toasts = new Queue<Achievement>();
        readonly string root, dataRoot;
        readonly List<Achievement> rules;
        ProcessMemory memory;
        GameSession session;
        ProgressStore progress;
        AchievementEngine engine;
        ulong guestBase;
        uint saveAddress;
        DateTime identityCheck;
        bool demo, recording, connecting;
        int demoTick;
        ToastForm activeToast;
        public MainForm(bool startDemo, bool autoConnect)
        {
            root = AppDomain.CurrentDomain.BaseDirectory;
            dataRoot = Path.Combine(root, "data");
            rules = AchievementSet.Load(Path.Combine(root, "achievements.xml"));
            Text = "Wii U Achievements — The Wind Waker HD";
            StartPosition = FormStartPosition.CenterScreen;
            ClientSize = new Size(1040, 790);
            MinimumSize = new Size(900, 740);
            Font = new Font("Segoe UI", 10);
            ForeColor = ink; BackColor = background;
            AutoScaleMode = AutoScaleMode.Dpi;
            BuildUi();
            LoadSettings();
            timer.Interval = 100;
            timer.Tick += Tick;
            Shown += delegate { if (startDemo) StartDemo(); else if (autoConnect) Connect(); };
            FormClosing += delegate { timer.Stop(); if (memory != null) memory.Dispose(); if (activeToast != null) activeToast.Close(); };
            RefreshList();
            SetStatus("Pronto al collegamento", "Avvia il gioco e carica un salvataggio, poi premi Collega Cemu.", false);
        }
        Label Label(string text, float size, Color color)
        {
            return new Label { Text = text, AutoSize = true, ForeColor = color, Font = new Font("Segoe UI", size), Margin = new Padding(0, 0, 0, 5) };
        }
        void StyleButton(Button button, string text, bool primary)
        {
            button.Text = text; button.AutoSize = true; button.Height = 38;
            button.Padding = new Padding(12, 4, 12, 4); button.Margin = new Padding(0, 0, 10, 0);
            button.FlatStyle = FlatStyle.Flat; button.FlatAppearance.BorderSize = 0;
            button.BackColor = primary ? accent : surface; button.ForeColor = primary ? background : ink;
            button.Cursor = Cursors.Hand;
        }
        void BuildUi()
        {
            var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 7, Padding = new Padding(26, 23, 26, 18) };
            layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 76));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 145));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 65));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 110));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 40));
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 66));
            Controls.Add(layout);

            var header = new FlowLayoutPanel { Dock = DockStyle.Fill, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = Padding.Empty };
            header.Controls.Add(Label("Wii U Achievements", 24, ink));
            header.Controls.Add(Label("THE WIND WAKER HD  /  EUR  /  CEMU 2.6     ·     ALPHA 0.1", 9, accent));
            layout.Controls.Add(header, 0, 0);

            var setup = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3, RowCount = 3, Margin = new Padding(0, 0, 0, 10) };
            setup.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 100));
            setup.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            setup.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 100));
            setup.RowStyles.Add(new RowStyle(SizeType.Absolute, 38));
            setup.RowStyles.Add(new RowStyle(SizeType.Absolute, 38));
            setup.RowStyles.Add(new RowStyle(SizeType.Absolute, 46));
            setup.Controls.Add(Label("Cemu.exe", 10, muted), 0, 0);
            cemuPath.Dock = DockStyle.Fill; cemuPath.BackColor = surface; cemuPath.ForeColor = ink; cemuPath.BorderStyle = BorderStyle.FixedSingle;
            cemuPath.Text = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Cemu_2.6", "Cemu.exe");
            setup.Controls.Add(cemuPath, 1, 0);
            StyleButton(browse, "Sfoglia", false); browse.Dock = DockStyle.Fill;
            browse.Click += delegate { using (var dialog = new OpenFileDialog { Filter = "Cemu|Cemu.exe", Title = "Seleziona Cemu.exe" }) if (dialog.ShowDialog(this) == DialogResult.OK) cemuPath.Text = dialog.FileName; };
            setup.Controls.Add(browse, 2, 0);
            setup.Controls.Add(Label("Profilo locale", 10, muted), 0, 1);
            player.Text = "Giocatore 1"; player.Dock = DockStyle.Fill; player.BackColor = surface; player.ForeColor = ink; player.BorderStyle = BorderStyle.FixedSingle; player.MaxLength = 64;
            setup.Controls.Add(player, 1, 1);
            sound.Text = "Suono"; sound.Checked = true; sound.AutoSize = true; sound.ForeColor = muted;
            setup.Controls.Add(sound, 2, 1);
            var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill, WrapContents = false, Margin = Padding.Empty };
            StyleButton(connect, "Collega Cemu", true); connect.Click += delegate { Connect(); };
            StyleButton(disconnect, "Scollega", false); disconnect.Enabled = false; disconnect.Click += delegate { Disconnect("Sessione scollegata."); };
            StyleButton(demoButton, "Prova demo", false); demoButton.Click += delegate { StartDemo(); };
            buttons.Controls.AddRange(new Control[] { connect, disconnect, demoButton });
            setup.Controls.Add(buttons, 0, 2); setup.SetColumnSpan(buttons, 3);
            layout.Controls.Add(setup, 0, 1);

            var connection = new FlowLayoutPanel { Dock = DockStyle.Fill, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = Padding.Empty };
            status.Font = new Font("Segoe UI Semibold", 12); status.AutoSize = true; status.ForeColor = accent;
            detail.Font = new Font("Segoe UI", 9); detail.AutoSize = true; detail.ForeColor = muted;
            connection.Controls.Add(status); connection.Controls.Add(detail); layout.Controls.Add(connection, 0, 2);

            var stats = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 4, Margin = new Padding(0, 0, 0, 14) };
            for (int i = 0; i < 4; i++) stats.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 25));
            AddStat(stats, "RUPIE", rupees, 0); AddStat(stats, "CUORI", hearts, 1); AddStat(stats, "PUNTI LOCALI", points, 2); AddStat(stats, "OBIETTIVI", count, 3);
            layout.Controls.Add(stats, 0, 3);

            hint.Text = "Set iniziale: rupie e capacità di vita. Gli obiettivi già soddisfatti all'avvio non si sbloccano.";
            hint.Dock = DockStyle.Fill; hint.Font = new Font("Segoe UI", 9); hint.ForeColor = muted;
            layout.Controls.Add(hint, 0, 4);

            list.Dock = DockStyle.Fill; list.View = View.Details; list.FullRowSelect = true; list.MultiSelect = false; list.HideSelection = false;
            list.BackColor = surface; list.ForeColor = ink; list.BorderStyle = BorderStyle.None; list.Font = new Font("Segoe UI", 10); list.ShowItemToolTips = true;
            list.Columns.Add("Obiettivo", 230); list.Columns.Add("Condizione", 470); list.Columns.Add("Punti", 65); list.Columns.Add("Stato", 145);
            list.Resize += delegate { if (list.Width > 600) list.Columns[1].Width = Math.Max(250, list.ClientSize.Width - 460); };
            layout.Controls.Add(list, 0, 5);
            log.Dock = DockStyle.Fill; log.Multiline = true; log.ReadOnly = true; log.ScrollBars = ScrollBars.Vertical;
            log.BackColor = background; log.ForeColor = muted; log.Font = new Font("Consolas", 8); log.BorderStyle = BorderStyle.None;
            log.Margin = new Padding(0, 10, 0, 0); layout.Controls.Add(log, 0, 6);
        }
        void AddStat(TableLayoutPanel panel, string caption, Label value, int index)
        {
            var card = new FlowLayoutPanel { Dock = DockStyle.Fill, BackColor = surface, FlowDirection = FlowDirection.TopDown, WrapContents = false, Padding = new Padding(14, 10, 8, 5), Margin = new Padding(index == 0 ? 0 : 8, 0, 0, 0) };
            card.Controls.Add(Label(caption, 8, muted));
            value.Text = "—"; value.AutoSize = true; value.Font = new Font("Segoe UI Semibold", 24); value.ForeColor = ink;
            card.Controls.Add(value); panel.Controls.Add(card, index, 0);
        }
        void LoadSettings()
        {
            string path = Path.Combine(dataRoot, "settings.xml");
            if (!File.Exists(path)) return;
            try { var doc = SafeXml.Load(path); cemuPath.Text = (string)doc.Root.Element("cemu") ?? cemuPath.Text; player.Text = (string)doc.Root.Element("player") ?? player.Text; }
            catch (Exception ex) { Log("Impostazioni non caricate: " + ex.Message); }
        }
        void SaveSettings()
        {
            SafeXml.AtomicSave(new XDocument(new XElement("settings", new XElement("cemu", cemuPath.Text), new XElement("player", player.Text))), Path.Combine(dataRoot, "settings.xml"));
        }
        void SetStatus(string title, string subtitle, bool live)
        {
            status.Text = title; status.ForeColor = live ? accent : Color.FromArgb(244, 194, 116);
            detail.Text = subtitle;
        }
        void Log(string message)
        {
            log.AppendText(DateTime.Now.ToString("HH:mm:ss") + "  " + message + Environment.NewLine);
        }
        void SetConnected(bool value)
        {
            connect.Enabled = !value; disconnect.Enabled = value; demoButton.Enabled = !value;
            player.Enabled = !value; cemuPath.Enabled = !value;
            browse.Enabled = !value;
        }
        async void Connect()
        {
            if (connecting || memory != null || demo) return;
            connecting = true; connect.Enabled = false;
            SetStatus("Collegamento…", "Identificazione della sessione e verifica dello spazio Wii U.", false);
            ProcessMemory attached = null;
            try
            {
                if (String.IsNullOrWhiteSpace(player.Text)) throw new InvalidOperationException("Inserisci un nome per il profilo locale.");
                string path = Path.GetFullPath(cemuPath.Text.Trim().Trim('"'));
                cemuPath.Text = path;
                var processes = Process.GetProcessesByName("Cemu").Where(p => SamePath(p, path)).ToList();
                if (processes.Count != 1) throw new InvalidOperationException("Deve essere aperta una sola sessione Cemu dal percorso selezionato.");
                session = GameSession.FromLog(path, processes[0].Id);
                if (session.CemuVersion != "2.6" || session.TitleId != GameSession.EurTitleId || session.Version != "v0" || session.RpxHash.ToLowerInvariant() != "cdc68acd")
                    throw new InvalidOperationException("Prima alpha: Cemu 2.6 e Wind Waker HD EUR v0, RPX cdc68acd.");
                attached = new ProcessMemory(session.Pid);
                var bases = await Task.Run(() => attached.FindGuestBases());
                if (!bases.Contains(session.GuestBase)) throw new InvalidOperationException("Il log e la memoria del processo non corrispondono. Ricollega dopo l'avvio del gioco.");
                guestBase = session.GuestBase;
                // Profile confirmed on the user's live EUR v0 session: 0 -> 5 rupees, 12/12 quarters.
                saveAddress = 0x145B7B80;
                var sample = GameSnapshot.Parse(attached.Read(guestBase + saveAddress, 6));
                progress = new ProgressStore(Path.Combine(dataRoot, "progress"), player.Text, session.Fingerprint);
                engine = new AchievementEngine(rules, progress);
                engine.Update(sample);
                SaveSettings();
                memory = attached; attached = null; demo = false; recording = true;
                identityCheck = DateTime.MinValue;
                SetConnected(true); UpdateStats(sample); RefreshList(); timer.Start();
                SetStatus("Collegato · registrazione attiva", "Wind Waker HD EUR v0 · Cemu PID " + session.Pid + " · progressi locali", true);
                hint.Text = "Raccogli rupie e continua l'avventura. Usa un profilo locale diverso per una nuova partita.";
                Log("EUR v0 riconosciuta. " + sample.Rupees + " rupie, " + (sample.Health / 4m) + "/" + (sample.MaxHealth / 4m) + " cuori.");
                if (progress.Warning != null) Log(progress.Warning);
            }
            catch (Exception ex)
            {
                if (attached != null) attached.Dispose();
                SetConnected(false);
                SetStatus("Collegamento non riuscito", ex.Message, false); Log(ex.Message);
            }
            finally { connecting = false; }
        }
        static bool SamePath(Process process, string expected)
        {
            try { return String.Equals(Path.GetFullPath(process.MainModule.FileName), expected, StringComparison.OrdinalIgnoreCase); }
            catch { return false; }
        }
        void Disconnect(string reason)
        {
            timer.Stop(); recording = false; demo = false;
            if (memory != null) { memory.Dispose(); memory = null; }
            if (engine != null) engine.Reset();
            toasts.Clear(); if (activeToast != null) { activeToast.Close(); activeToast = null; }
            SetConnected(false); SetStatus("Scollegato", reason, false); Log(reason);
        }
        void StartDemo()
        {
            if (memory != null || demo) return;
            try
            {
                // Demo has its own temporary directory, never the real player's progress.
                string temporary = Path.Combine(dataRoot, "demo", Guid.NewGuid().ToString("N"));
                progress = new ProgressStore(temporary, "DEMO", "simulation"); engine = new AchievementEngine(rules, progress);
                demo = true; recording = true; demoTick = 0;
                SetConnected(true); RefreshList();
                SetStatus("DEMO · simulazione", "Dati simulati. I progressi della tua partita restano separati.", false);
                hint.Text = "La demo aumenta le rupie automaticamente. Premi Scollega per tornare al gioco reale.";
                timer.Start(); Log("Demo avviata: nessun collegamento a Cemu.");
            }
            catch (Exception ex) { Log(ex.Message); }
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
                        var current = GameSession.FromLog(session.CemuPath, session.Pid);
                        if (current.Fingerprint != session.Fingerprint || current.GuestBase != guestBase) throw new InvalidOperationException("Il gioco o la sessione Cemu sono cambiati. Ricollega l'app.");
                        identityCheck = DateTime.UtcNow;
                    }
                    sample = GameSnapshot.Parse(memory.Read(guestBase + saveAddress, 6));
                }
                foreach (var a in engine.Update(sample))
                {
                    Log((demo ? "[DEMO] " : "") + "Sbloccato: " + a.Title + " (+" + a.Points + ")");
                    toasts.Enqueue(a); RefreshList();
                }
                UpdateStats(sample);
                if (activeToast == null && toasts.Count > 0)
                {
                    activeToast = new ToastForm(toasts.Dequeue(), demo);
                    activeToast.FormClosed += delegate { activeToast = null; };
                    activeToast.Show();
                    if (sound.Checked) SystemSounds.Asterisk.Play();
                }
            }
            catch (Exception ex)
            {
                // Invalid samples/disconnections cannot carry an armed condition across a gap.
                Disconnect("Lettura sospesa: " + ex.Message);
            }
        }
        void UpdateStats(GameSnapshot s)
        {
            rupees.Text = s.Rupees.ToString(); hearts.Text = (s.Health / 4m).ToString("0.##") + " / " + (s.MaxHealth / 4m).ToString("0.##");
            points.Text = rules.Where(a => progress != null && progress.Contains(a.Id)).Sum(a => a.Points).ToString();
            count.Text = rules.Count(a => progress != null && progress.Contains(a.Id)) + " / " + rules.Count;
        }
        void RefreshList()
        {
            list.BeginUpdate(); list.Items.Clear();
            foreach (var a in rules)
            {
                var unlock = progress == null ? null : progress.Get(a.Id);
                var item = new ListViewItem(a.Title); item.ToolTipText = a.Description;
                item.SubItems.Add(a.Description); item.SubItems.Add(a.Points.ToString());
                item.SubItems.Add(unlock == null ? "Da sbloccare" : "Sbloccato " + unlock.Utc.ToLocalTime().ToString("HH:mm"));
                if (unlock != null) item.ForeColor = accent;
                list.Items.Add(item);
            }
            list.EndUpdate();
        }
        // Used only for automated rendering checks of our own UI.
        public void SavePreview(string path)
        {
            using (var bitmap = new Bitmap(Width, Height)) { DrawToBitmap(bitmap, new Rectangle(0, 0, Width, Height)); bitmap.Save(path); }
        }
    }

    public sealed class ToastForm : Form
    {
        readonly System.Windows.Forms.Timer timeout = new System.Windows.Forms.Timer();
        protected override bool ShowWithoutActivation { get { return true; } }
        protected override CreateParams CreateParams
        {
            get { var cp = base.CreateParams; cp.ExStyle |= 0x08000000 | 0x00000080; return cp; }
        }
        public ToastForm(Achievement achievement, bool demo)
        {
            FormBorderStyle = FormBorderStyle.None; ShowInTaskbar = false; TopMost = true;
            BackColor = Color.FromArgb(25, 43, 52); ForeColor = Color.White;
            Size = new Size(390, 110); Padding = new Padding(18, 12, 18, 12);
            var rows = new FlowLayoutPanel { Dock = DockStyle.Fill, FlowDirection = FlowDirection.TopDown, WrapContents = false };
            rows.Controls.Add(new Label { AutoSize = true, Text = demo ? "DEMO · OBIETTIVO SBLOCCATO" : "OBIETTIVO SBLOCCATO", ForeColor = Color.FromArgb(89, 222, 187), Font = new Font("Segoe UI", 8) });
            rows.Controls.Add(new Label { AutoSize = true, Text = achievement.Title, Font = new Font("Segoe UI Semibold", 16) });
            rows.Controls.Add(new Label { AutoSize = true, Text = "+" + achievement.Points + " punti locali", ForeColor = Color.LightGray, Font = new Font("Segoe UI", 10) });
            Controls.Add(rows);
            var area = Screen.PrimaryScreen.WorkingArea;
            StartPosition = FormStartPosition.Manual; Location = new Point(area.Right - Width - 24, area.Bottom - Height - 24);
            timeout.Interval = 4500; timeout.Tick += delegate { Close(); }; timeout.Start();
            FormClosed += delegate { timeout.Dispose(); };
        }
    }
}
