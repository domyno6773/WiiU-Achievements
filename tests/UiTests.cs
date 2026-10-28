using System;
using System.Collections;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;
using WiiUAchievements;

class UiTests
{
    static int checks;
    static readonly BindingFlags Private = BindingFlags.Instance | BindingFlags.NonPublic;
    static object Get(object target, string field) { return target.GetType().GetField(field, Private | BindingFlags.Public).GetValue(target); }
    static void Set(object target, string field, object value) { target.GetType().GetField(field, Private | BindingFlags.Public).SetValue(target, value); }
    static object Call(object target, string method, params object[] args) { return target.GetType().GetMethod(method, Private).Invoke(target, args); }
    static void Check(bool condition, string name)
    {
        if (!condition) throw new Exception("FAIL " + name);
        checks++; Console.WriteLine("PASS " + name);
    }
    static void Settle()
    {
        for (int i = 0; i < 55; i++) { Application.DoEvents(); Thread.Sleep(10); }
    }
    static object Widget(MainForm form, string id) { Call(form, "Arrange"); return ((IDictionary)Get(form, "widgets"))[id]; }
    static void Click(MainForm form, string id)
    {
        object widget = Widget(form, id);
        var r = (RectangleF)Get(widget, "Bounds"); float scale = (float)Get(form, "scale");
        var e = new MouseEventArgs(MouseButtons.Left, 1, (int)((r.X + r.Width / 2) * scale), (int)((r.Y + r.Height / 2) * scale), 0);
        Call(form, "OnMouseDown", e); Call(form, "OnMouseUp", e); Settle();
    }
    static void Key(MainForm form, Keys key)
    {
        object[] args = { new Message(), key };
        form.GetType().GetMethod("ProcessCmdKey", Private).Invoke(form, args); Settle();
    }
    [STAThread]
    static int Main(string[] args)
    {
        try
        {
            string output = args.Length == 0 ? Path.Combine(Path.GetTempPath(), "wiiua-ui-" + Guid.NewGuid().ToString("N")) : Path.GetFullPath(args[0]);
            Directory.CreateDirectory(output);
            Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
            using (var form = new MainForm(false, false))
            {
                // This executable is built in an isolated test directory. It never attaches to Cemu.
                Set(form, "soundOn", false); Set(form, "ambientOn", false);
                form.Show(); Settle();
                form.SavePreview(Path.Combine(output, "main.png"));
                Click(form, "games");
                Check((bool)Get(form, "libraryOpen"), "Mouse opens the Games section");
                Check(!(bool)Get(Widget(form, "game:future-games"), "Enabled"), "Future games show as unavailable and cannot be activated");
                Check(!(bool)Get(Widget(form, "connect"), "Visible"), "Catalog does not expose hidden achievement actions");
                form.SavePreview(Path.Combine(output, "games.png"));
                int children = form.AccessibilityObject.GetChildCount(); bool unavailable = false;
                for (int i = 0; i < children; i++)
                {
                    var item = form.AccessibilityObject.GetChild(i);
                    if (item.Name == "Altri giochi") unavailable = (item.State & AccessibleStates.Unavailable) != 0 && item.Description.Contains("Non disponibile");
                }
                Check(unavailable, "Unavailable state and explanation reach accessibility clients");
                Click(form, "game:future-games");
                Check(((GameDefinition)Get(form, "selectedGame")).Id == GameCatalog.WindWakerId && (bool)Get(form, "libraryOpen"), "Clicking an unavailable card does not change the selected game");
                Key(form, Keys.Tab); Key(form, Keys.Right); Key(form, Keys.Escape);
                Check(!(bool)Get(form, "libraryOpen"), "Escape returns from the Games section");
                Key(form, Keys.G); Click(form, "game:wind-waker-hd");
                Check(!(bool)Get(form, "libraryOpen"), "Selecting the current game opens its objectives");
                Call(form, "StartDemo"); ((System.Windows.Forms.Timer)Get(form, "timer")).Stop();
                for (int i = 0; i < 45; i++) { Call(form, "Tick", null, EventArgs.Empty); Thread.Sleep(2); }
                Check((int)form.GetType().GetProperty("UnlockedCount", Private).GetValue(form, null) == 1, "Demo unlocks the existing objective through the engine");
                Settle(); form.SavePreview(Path.Combine(output, "demo.png"));
                Click(form, "pause");
                Check((bool)Get(form, "paused") && !(bool)Get(form, "recording") && Get(form, "current") == null, "Pause stops recording and clears transient state");
                Check((int)form.GetType().GetProperty("UnlockedCount", Private).GetValue(form, null) == 1, "Pause preserves earned objectives");
                form.SavePreview(Path.Combine(output, "paused.png"));
                Click(form, "games"); Key(form, Keys.B);
                Check((bool)Get(form, "paused") && !(bool)Get(form, "libraryOpen"), "Browsing games does not resume a paused session");
                Key(form, Keys.P); ((System.Windows.Forms.Timer)Get(form, "timer")).Stop();
                Check(!(bool)Get(form, "paused") && (bool)Get(form, "recording"), "P resumes recording with a fresh baseline");
                Call(form, "Tick", null, EventArgs.Empty);
                var engine = (AchievementEngine)Get(form, "engine");
                Check(engine.Status("wallet-10").Phase == AchievementPhase.Unlocked, "Resume retains the already saved unlock");
                Click(form, "games");
                Check((bool)Get(form, "recording"), "Opening the catalog does not stop an active session");
                var syntheticGame = new GameDefinition("test-game", "Synthetic test game", "Test only", "achievements.xml", AchievementSet.Revision, GameCatalog.Default.Profiles[0]);
                Call(form, "SelectGame", syntheticGame);
                Check(((GameDefinition)Get(form, "selectedGame")).Id == GameCatalog.WindWakerId, "Switching to another game is blocked during a session");
                float scale = (float)Get(form, "scale");
                form.ClientSize = new Size((int)(900 * scale), (int)(680 * scale)); Settle();
                form.SavePreview(Path.Combine(output, "games-small.png"));
                var viewport = (RectangleF)Get(form, "libraryGridRect");
                var tile = (RectangleF)Get(Widget(form, "game:wind-waker-hd"), "Bounds");
                Check(viewport.Contains(tile), "Game card fits at the supported minimum window size");
                Key(form, Keys.Escape); form.SavePreview(Path.Combine(output, "main-small.png"));
                Click(form, "settings"); Settle();
                Check(((TextBox)Get(form, "player")).Visible, "Existing settings fields still open");
                form.SavePreview(Path.Combine(output, "settings-small.png"));
                Key(form, Keys.Escape); Key(form, Keys.Y);
                Check(!(bool)Get(form, "recording") && !(bool)Get(form, "demo") && Get(form, "progress") == null, "Disconnect discards demo state without affecting real progress");
                Call(form, "SelectGame", syntheticGame);
                Check(((GameDefinition)Get(form, "selectedGame")).Id == "test-game" && form.Text.Contains("Synthetic test game"), "A future registered game's rules can be selected after disconnect");
                var badGame = new GameDefinition("bad-game", "Missing set", "Test only", "missing.xml", AchievementSet.Revision, GameCatalog.Default.Profiles[0]);
                bool rejected = false;
                try { Call(form, "SelectGame", badGame); } catch (TargetInvocationException ex) { rejected = ex.InnerException is IOException; }
                Check(rejected && ((GameDefinition)Get(form, "selectedGame")).Id == "test-game", "A broken future set leaves the previous selection intact");
                var syntheticProgress = new ProgressStore(Path.Combine(output, "profile-test"), "old-player", "test-game");
                syntheticProgress.Award("wallet-10", DateTime.UtcNow); Set(form, "progress", syntheticProgress);
                ((TextBox)Get(form, "player")).Text = "Synthetic new profile";
                Check(Get(form, "progress") == null, "Editing the local profile clears another profile's displayed unlocks");
                form.Close();
            }
            using (var restored = new MainForm(false, false))
                Check(((GameDefinition)Get(restored, "selectedGame")).Id == GameCatalog.WindWakerId, "Unknown saved catalog IDs safely fall back to the supported default");
            Console.WriteLine("SUCCESS " + checks + " UI checks; previews: " + output); return 0;
        }
        catch (Exception ex) { Console.Error.WriteLine(ex); return 1; }
    }
}
