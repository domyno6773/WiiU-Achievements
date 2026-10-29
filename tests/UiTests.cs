using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
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
    static void RenderScroll(MainForm form, Graphics graphics, float offset)
    {
        Set(form, "scroll", offset); Set(form, "scrollTarget", offset);
        graphics.ResetTransform(); graphics.ResetClip();
        Call(form, "Arrange");
        var grid = (RectangleF)Get(form, "gridRect");
        float scale = (float)Get(form, "scale");
        var clip = new RectangleF(0, grid.Y, form.ClientSize.Width / scale, grid.Height);
        graphics.SetClip(new RectangleF(clip.X * scale, clip.Y * scale, clip.Width * scale, clip.Height * scale));
        Call(form, "Render", graphics, clip);
    }
    static long FooterPixels(MainForm form, Bitmap bitmap)
    {
        var tile = (RectangleF)Get(Widget(form, "tile:wallet-10"), "Bounds");
        float scale = (float)Get(form, "scale");
        long sum = 0;
        for (int y = (int)((tile.Y + 79) * scale); y < (int)((tile.Y + 97) * scale); y++)
            for (int x = (int)((tile.X + 102) * scale); x < (int)((tile.Right - 16) * scale); x++)
                sum += bitmap.GetPixel(x, y).ToArgb();
        return sum;
    }
    static void ScrollChecks(MainForm form, string output)
    {
        Set(form, "categoryFilter", 6);
        Set(form, "introStart", -10f); Set(form, "hovered", null); Set(form, "focused", null);
        foreach (var item in ((IDictionary)Get(form, "widgets")).Values) { Set(item, "Hover", 0f); Set(item, "Press", 0f); }
        Call(form, "ClearTileImages");
        using (var bitmap = new Bitmap(form.ClientSize.Width, form.ClientSize.Height))
        using (var graphics = Graphics.FromImage(bitmap))
        {
            RenderScroll(form, graphics, 0);
            int builds = (int)Get(form, "tileImageBuilds");
            Check(builds > 0, "Viewport cards populate the image cache");
            RenderScroll(form, graphics, 1);
            Check((int)Get(form, "tileImageBuilds") == builds, "Scrolling within the same rows reuses card images");
            var walletBounds = (RectangleF)Get(Widget(form, "tile:wallet-10"), "Bounds");
            var walletGrid = (RectangleF)Get(form, "gridRect");
            float walletOffset = walletBounds.Top - walletGrid.Top - 12 + (float)Get(form, "scroll");
            Set(form, "current", new GameSnapshot { Rupees = 1, Health = 12, MaxHealth = 12, Utc = DateTime.UtcNow });
            RenderScroll(form, graphics, walletOffset); long first = FooterPixels(form, bitmap);
            builds = (int)Get(form, "tileImageBuilds");
            Set(form, "current", new GameSnapshot { Rupees = 9, Health = 12, MaxHealth = 12, Utc = DateTime.UtcNow });
            RenderScroll(form, graphics, walletOffset);
            Check(first != FooterPixels(form, bitmap) && (int)Get(form, "tileImageBuilds") == builds,
                "Live progress redraws while static card images remain cached");
            var saved = new ProgressStore(Path.Combine(output, "cache-unlock"), "test", "synthetic");
            saved.Award("wallet-10", DateTime.UtcNow); Set(form, "progress", saved);
            RenderScroll(form, graphics, walletOffset);
            var entry = ((IDictionary)Get(form, "tileImages"))["wallet-10"];
            Check((bool)Get(entry, "Unlocked"), "An unlock replaces the cached locked medal immediately");
            Set(form, "progress", null); Set(form, "current", null);
            Set(form, "categoryFilter", 0); Call(form, "Arrange");
            var grid = (RectangleF)Get(form, "gridRect");
            float end = (float)Get(form, "contentHeight") - grid.Height;
            double[] times = new double[80];
            for (int i = 0; i < times.Length; i++)
            {
                var watch = Stopwatch.StartNew();
                RenderScroll(form, graphics, end * i / (times.Length - 1));
                times[i] = watch.Elapsed.TotalMilliseconds;
            }
            Check(((IDictionary)Get(form, "tileImages")).Count <= 48, "Long scrolling keeps the native bitmap cache bounded");
            var last = Widget(form, "tile:" + ((List<Achievement>)Call(form, "Filtered")).Last().Id);
            Check(((RectangleF)Get(last, "Bounds")).IntersectsWith(grid), "The complete catalog remains reachable by scrolling");
            Array.Sort(times);
            Console.WriteLine("SCROLL 110 cards / 80 grid frames: median={0:F2} ms p95={1:F2} ms", times[40], times[76]);
            Call(form, "ClearTileImages");
            Check(((IDictionary)Get(form, "tileImages")).Count == 0, "Clearing the image cache releases all entries");
            Set(form, "introStart", (float)((Stopwatch)Get(form, "clock")).Elapsed.TotalSeconds - 0.6f);
            RenderScroll(form, graphics, end);
            Check(((IDictionary)Get(form, "tileImages")).Count > 0,
                "Deep scrolling during the intro does not delay cards by their catalog index");
            Set(form, "introStart", -10f);
        }
        Set(form, "scroll", 0f); Set(form, "scrollTarget", 0f);
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
                var curated = (List<Achievement>)Get(form, "rules");
                Check(Art.For(curated.Single(a => a.Id == "wallet-5000"), curated).Tier == Tier.Silver &&
                    Art.For(curated.Single(a => a.Id == "hearts-20"), curated).Tier == Tier.Gold,
                    "Medals use curated rarity independently of equal point values");
                form.SavePreview(Path.Combine(output, "main.png"));
                Check((int)form.GetType().GetProperty("ActiveCount", Private).GetValue(form, null) == 70,
                    "Completion denominator contains only executable objectives");
                Click(form, "tab3");
                var pending = (List<Achievement>)Call(form, "Filtered");
                Check(pending.Count == 40 && pending.All(a => a.IsPlanned),
                    "Implemented series targets no longer appear in the unimplemented catalog");
                string pendingId = pending.First().Id;
                Check(((string)Get(Widget(form, "tile:" + pendingId), "Help")).Contains("Da verificare"),
                    "Pending state and validation requirement reach accessibility clients");
                Check(((string)Call(form, "Describe", Widget(form, "tile:" + pendingId))).Contains("nessuno sblocco automatico"),
                    "Unimplemented story targets explicitly announce that automatic unlocks are unavailable");
                form.SavePreview(Path.Combine(output, "pending.png"));
                Click(form, "tile:" + pendingId);
                Check(((Achievement)Get(form, "detail")).Id == pendingId, "Pending objectives have an inspectable detail card");
                form.SavePreview(Path.Combine(output, "pending-detail.png"));
                Key(form, Keys.Escape); Click(form, "category");
                Check(((List<Achievement>)Call(form, "Filtered")).All(a => a.Category == "Storia"), "Category button filters the pending catalog");
                for (int i = 0; i < AchievementSet.Categories.Count; i++) Key(form, Keys.C);
                Check((int)Get(form, "categoryFilter") == 0, "C cycles all categories and returns to the full catalog");
                Click(form, "tab2");
                Check(((List<Achievement>)Call(form, "Filtered")).All(a => !a.IsPlanned), "Unlockable filter excludes unvalidated objectives");
                Click(form, "category");
                Check(((List<Achievement>)Call(form, "Filtered")).Any(a => a.Id == "story-heros-clothes") &&
                    ((List<Achievement>)Call(form, "Filtered")).Any(a => a.Id == "story-telescope" && a.Experimental),
                    "Story category contains both the confirmed tunic and executable experimental telescope");
                form.SavePreview(Path.Combine(output, "tunic-active.png"));
                Set(form, "categoryFilter", 0);
                Click(form, "tab0");
                Check(((List<Achievement>)Call(form, "Filtered")).First().Id == "story-heros-clothes", "Default ordering begins with the start of the playthrough");
                Click(form, "chapter");
                Check(((List<Achievement>)Call(form, "Filtered")).All(a => a.Chapter == "01 · Outset"), "Chapter filter selects the initial playthrough group");
                Key(form, Keys.F);
                Check(((List<Achievement>)Call(form, "Filtered")).All(a => a.Chapter == "02 · Pirati"), "F advances to the next chronological group");
                Set(form, "chapterFilter", 0);
                ScrollChecks(form, output);
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
                Click(form, "tab3"); form.SavePreview(Path.Combine(output, "pending-live.png"));
                Check((int)form.GetType().GetProperty("UnlockedCount", Private).GetValue(form, null) == 1,
                    "Displaying pending objectives while sampling does not alter unlocks");
                Click(form, "tab0");
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
                Check(((RectangleF)Get(Widget(form, "tab3"), "Bounds")).Right < 900,
                    "All four status tabs fit at the minimum window width");
                Click(form, "tab3"); form.SavePreview(Path.Combine(output, "pending-small.png"));
                Click(form, "tile:" + pendingId); form.SavePreview(Path.Combine(output, "pending-detail-small.png")); Key(form, Keys.Escape);
                Click(form, "tab0");
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
