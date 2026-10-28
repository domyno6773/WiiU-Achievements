using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.IO;
using System.Media;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace WiiUAchievements
{
    // Per-pixel alpha windows (soft shadow, rounded corners, fades) through UpdateLayeredWindow.
    static class Layered
    {
        [StructLayout(LayoutKind.Sequential)] struct Point32 { public int X, Y; }
        [StructLayout(LayoutKind.Sequential)] struct Size32 { public int Cx, Cy; }
        [StructLayout(LayoutKind.Sequential, Pack = 1)] struct Blend { public byte Op, Flags, Alpha, Format; }
        [DllImport("user32.dll", SetLastError = true)]
        static extern bool UpdateLayeredWindow(IntPtr hwnd, IntPtr dst, ref Point32 position, ref Size32 size, IntPtr src, ref Point32 source, int key, ref Blend blend, int flags);
        [DllImport("user32.dll")] static extern IntPtr GetDC(IntPtr hwnd);
        [DllImport("user32.dll")] static extern int ReleaseDC(IntPtr hwnd, IntPtr dc);
        [DllImport("gdi32.dll")] static extern IntPtr CreateCompatibleDC(IntPtr dc);
        [DllImport("gdi32.dll")] static extern bool DeleteDC(IntPtr dc);
        [DllImport("gdi32.dll")] static extern IntPtr SelectObject(IntPtr dc, IntPtr obj);
        [DllImport("gdi32.dll")] static extern bool DeleteObject(IntPtr obj);

        public static bool Disabled;
        public static bool Available { get { return !Disabled && Environment.OSVersion.Platform == PlatformID.Win32NT; } }

        public static bool Update(IntPtr hwnd, Bitmap bitmap, Point location, byte alpha)
        {
            IntPtr screen = GetDC(IntPtr.Zero), memory = CreateCompatibleDC(screen), hbitmap = IntPtr.Zero, previous = IntPtr.Zero;
            try
            {
                hbitmap = bitmap.GetHbitmap(Color.FromArgb(0));
                previous = SelectObject(memory, hbitmap);
                var size = new Size32 { Cx = bitmap.Width, Cy = bitmap.Height };
                var source = new Point32();
                var position = new Point32 { X = location.X, Y = location.Y };
                var blend = new Blend { Op = 0, Flags = 0, Alpha = alpha, Format = 1 }; // AC_SRC_OVER, AC_SRC_ALPHA
                return UpdateLayeredWindow(hwnd, screen, ref position, ref size, memory, ref source, 0, ref blend, 2); // ULW_ALPHA
            }
            finally
            {
                ReleaseDC(IntPtr.Zero, screen);
                if (hbitmap != IntPtr.Zero) { SelectObject(memory, previous); DeleteObject(hbitmap); }
                DeleteDC(memory);
            }
        }
    }

    // Unlock popup: slides in at the top of the screen that shows Cemu, never takes focus and
    // lets clicks pass through to the game.
    public sealed class ToastForm : Form
    {
        public const float Duration = 5.4f;
        const float Edge = 30, CardW = 460, CardH = 118, InTime = 0.5f, OutTime = 0.42f;
        readonly Achievement achievement;
        readonly Art art;
        readonly string header;
        readonly Rectangle area;
        readonly System.Windows.Forms.Timer frame = new System.Windows.Forms.Timer();
        readonly Stopwatch clock = new Stopwatch();
        readonly bool layered;
        Bitmap fallbackFrame;
        float scale = 1;
        public bool Failed { get; private set; }

        protected override bool ShowWithoutActivation { get { return true; } }
        protected override CreateParams CreateParams
        {
            get
            {
                var cp = base.CreateParams;
                cp.ExStyle |= 0x08000000 | 0x00000080 | 0x00000008; // NOACTIVATE | TOOLWINDOW | TOPMOST
                if (layered) cp.ExStyle |= 0x00080000 | 0x00000020; // LAYERED | TRANSPARENT (click-through)
                return cp;
            }
        }

        public ToastForm(Achievement achievement, Art art, string header, Screen screen)
        {
            this.achievement = achievement; this.art = art; this.header = header;
            layered = Layered.Available;
            area = (screen ?? Screen.PrimaryScreen).WorkingArea;
            FormBorderStyle = FormBorderStyle.None; ShowInTaskbar = false; TopMost = true;
            StartPosition = FormStartPosition.Manual;
            BackColor = Color.FromArgb(246, 249, 251);
            try { using (var g = Graphics.FromHwnd(IntPtr.Zero)) scale = Math.Max(1f, g.DpiX / 96f); } catch (Exception) { scale = 1; }
            if (layered) { Size = new Size(1, 1); Location = new Point(area.Left, area.Top - 400); }
            else
            {
                SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint, true);
                Size = new Size((int)(CardW * scale), (int)(CardH * scale));
                using (var path = Theme.Round(new RectangleF(0, 0, Width, Height), 20 * scale)) Region = new System.Drawing.Region(path);
                Location = new Point(area.Left + (area.Width - Width) / 2, area.Top - Height);
            }
            frame.Interval = 15;
            frame.Tick += delegate { Step(); };
            Shown += delegate { clock.Start(); Step(); frame.Start(); };
            FormClosed += delegate { frame.Stop(); frame.Dispose(); if (fallbackFrame != null) fallbackFrame.Dispose(); };
        }

        void Step()
        {
            float t = (float)clock.Elapsed.TotalSeconds;
            if (t >= Duration) { Close(); return; }
            float slide, alpha;
            Motion(t, out slide, out alpha);
            int w = (int)Math.Ceiling((CardW + Edge * 2) * scale);
            int x = area.Left + (area.Width - w) / 2, top = area.Top + (int)((22 - Edge) * scale);
            int y = top - (int)((1 - slide) * (CardH + Edge + 30) * scale);
            if (layered)
            {
                using (var bitmap = Render(achievement, art, header, t, scale))
                {
                    bool ok = false;
                    try { ok = Layered.Update(Handle, bitmap, new Point(x, y), (byte)(255 * alpha)); }
                    catch (Exception) { ok = false; }
                    if (!ok) { Failed = true; Layered.Disabled = true; frame.Stop(); Close(); }
                }
                return;
            }
            if (fallbackFrame != null) fallbackFrame.Dispose();
            fallbackFrame = Render(achievement, art, header, t, scale);
            Location = new Point(x + (int)(Edge * scale), y + (int)(Edge * scale));
            try { Opacity = Math.Max(0.05, alpha); } catch (Exception) { }
            Invalidate();
        }
        protected override void OnPaint(PaintEventArgs e)
        {
            if (fallbackFrame == null) return;
            e.Graphics.DrawImage(fallbackFrame, -(int)(Edge * scale), -(int)(Edge * scale));
        }

        static void Motion(float t, out float slide, out float alpha)
        {
            if (t < InTime) { slide = Theme.EaseBack(t / InTime); alpha = Theme.EaseOut(t / (InTime * 0.6f)); }
            else if (t > Duration - OutTime) { float u = (t - (Duration - OutTime)) / OutTime; slide = 1 - Theme.EaseIn(u); alpha = 1 - Theme.EaseIn(u); }
            else { slide = 1; alpha = 1; }
        }

        // Renders one frame of the card (with room for its shadow) at time t. Also used by --preview-toast.
        public static Bitmap Render(Achievement a, Art art, string header, float t, float scale)
        {
            var bmp = new Bitmap((int)Math.Ceiling((CardW + Edge * 2) * scale), (int)Math.Ceiling((CardH + Edge * 2) * scale), PixelFormat.Format32bppArgb);
            using (var g = Graphics.FromImage(bmp))
            {
                g.SmoothingMode = SmoothingMode.AntiAlias;
                g.TextRenderingHint = TextRenderingHint.AntiAlias;
                g.PixelOffsetMode = PixelOffsetMode.HighQuality;
                g.ScaleTransform(scale, scale);
                var card = new RectangleF(Edge, Edge, CardW, CardH);
                Theme.Shadow(g, card, 20, 16, 120, 6);
                for (int i = 4; i >= 1; i--)
                    Theme.Stroke(g, Theme.Inflate(card, i * 2.5f, i * 2.5f), 20 + i * 2.5f, Color.FromArgb(22 / i + 6, Theme.Glow), 3);
                Theme.Fill(g, card, 20, Color.White, Color.FromArgb(236, 242, 246));
                using (var path = Theme.Round(card, 20))
                {
                    var state = g.Save();
                    g.SetClip(path, CombineMode.Intersect);
                    using (var b = new LinearGradientBrush(new RectangleF(card.X, card.Y, 150, card.Height), Color.FromArgb(70, Theme.AccentSoft), Color.FromArgb(0, Theme.AccentSoft), 0f))
                        g.FillRectangle(b, card.X, card.Y, 149, card.Height);
                    float shine = (t - 0.62f) / 0.75f;
                    if (shine > 0 && shine < 1)
                    {
                        float sx = card.X - 120 + (card.Width + 240) * Theme.EaseInOut(shine);
                        using (var band = new GraphicsPath())
                        {
                            band.AddPolygon(new[] { new PointF(sx, card.Y), new PointF(sx + 70, card.Y), new PointF(sx + 30, card.Bottom), new PointF(sx - 40, card.Bottom) });
                            using (var b = new LinearGradientBrush(new RectangleF(sx - 40, card.Y, 111, card.Height), Color.FromArgb(0, 255, 255, 255), Color.FromArgb(0, 255, 255, 255), 0f))
                            {
                                var blend = new ColorBlend { Positions = new[] { 0f, 0.5f, 1f }, Colors = new[] { Color.FromArgb(0, 255, 255, 255), Color.FromArgb(150, 255, 255, 255), Color.FromArgb(0, 255, 255, 255) } };
                                b.InterpolationColors = blend;
                                g.FillPath(b, band);
                            }
                        }
                    }
                    float remaining = 1 - Theme.Clamp01((t - InTime) / (Duration - InTime - OutTime));
                    using (var b = new LinearGradientBrush(new RectangleF(card.X, card.Bottom - 4, card.Width, 4), Theme.Glow, Theme.Accent, 0f))
                        g.FillRectangle(b, card.X, card.Bottom - 4, card.Width * remaining, 4);
                    g.Restore(state);
                }
                Theme.Stroke(g, Theme.Inflate(card, -0.5f, -0.5f), 20, Theme.Accent, 2.2f);

                var medalCenter = new PointF(card.X + 62, card.Y + card.Height / 2);
                float rays = Theme.EaseOut((t - 0.2f) / 0.5f);
                Theme.Rays(g, medalCenter, 66, t * 28, Color.FromArgb((int)(150 * rays), 255, 214, 90));
                for (int i = 0; i < 10; i++)
                {
                    float u = (t - 0.28f - i * 0.02f) / 1.0f;
                    if (u <= 0 || u >= 1) continue;
                    double ang = i * Math.PI * 2 / 10 + 0.3;
                    float d = 30 + 44 * Theme.EaseOut(u);
                    var p = new PointF(medalCenter.X + (float)Math.Cos(ang) * d, medalCenter.Y + (float)Math.Sin(ang) * d * 0.8f);
                    Theme.Sparkle(g, p, 6 * (1 - u) + 1.5f, Color.FromArgb((int)(255 * (1 - u)), i % 2 == 0 ? Color.FromArgb(255, 216, 80) : Theme.Glow));
                }
                float pop = (t - 0.1f) / 0.55f;
                float size = 84 * (pop <= 0 ? 0 : 0.25f + 0.75f * Theme.EaseBack(pop));
                if (size > 1)
                {
                    var medal = Theme.Square(medalCenter, size);
                    Theme.Medal(g, medal, art, 0);
                    if (pop >= 1) Theme.StatusBadge(g, Theme.Square(medalCenter, 84), true);
                }

                float reveal = Theme.EaseOut((t - 0.22f) / 0.45f);
                float tx = card.X + 122 + 14 * (1 - reveal), tw = card.Right - 18 - (card.X + 122);
                Theme.Text(g, header, Theme.Font(12, Weight.Heavy), Theme.Alpha(Theme.Accent, reveal), new RectangleF(tx, card.Y + 18, tw, 18));
                Theme.Text(g, a.Title, Theme.Font(22, Weight.Heavy), Theme.Alpha(Theme.Ink, reveal), new RectangleF(tx, card.Y + 38, tw, 32));
                string points = "+" + a.Points + (a.Points == 1 ? " punto" : " punti") + "  ·  " + Art.TierName(art.Tier);
                var star = new RectangleF(tx, card.Y + 77, 16, 16);
                Theme.Star(g, star, Theme.Alpha(Color.FromArgb(255, 226, 110), reveal), Theme.Alpha(Color.FromArgb(222, 150, 20), reveal));
                Theme.Text(g, points, Theme.Font(14, Weight.Bold), Theme.Alpha(Theme.Muted, reveal), new RectangleF(tx + 22, card.Y + 77, tw - 22, 20));
            }
            return bmp;
        }
    }

    // Original three-note bell arpeggio, synthesised once in memory (no external sound file).
    public static class Chime
    {
        static SoundPlayer player;
        public static void Play()
        {
            try
            {
                if (player == null) { player = new SoundPlayer(new MemoryStream(Build())); player.Load(); }
                player.Play();
            }
            catch (Exception) { try { SystemSounds.Asterisk.Play(); } catch (Exception) { } }
        }
        public static byte[] Build()
        {
            const int rate = 44100;
            var samples = new double[(int)(rate * 1.3)];
            Note(samples, rate, 0.00, 1046.50, 0.50); // C6
            Note(samples, rate, 0.085, 1567.98, 0.42); // G6
            Note(samples, rate, 0.17, 2093.00, 0.38); // C7
            Note(samples, rate, 0.17, 2637.02, 0.12); // E7 shimmer
            double peak = 0;
            foreach (var s in samples) peak = Math.Max(peak, Math.Abs(s));
            double gain = peak > 0 ? 0.42 / peak : 0;
            using (var stream = new MemoryStream())
            using (var w = new BinaryWriter(stream))
            {
                int data = samples.Length * 2;
                w.Write(new[] { 'R', 'I', 'F', 'F' }); w.Write(36 + data); w.Write(new[] { 'W', 'A', 'V', 'E' });
                w.Write(new[] { 'f', 'm', 't', ' ' }); w.Write(16); w.Write((short)1); w.Write((short)1);
                w.Write(rate); w.Write(rate * 2); w.Write((short)2); w.Write((short)16);
                w.Write(new[] { 'd', 'a', 't', 'a' }); w.Write(data);
                foreach (var s in samples) w.Write((short)Math.Max(short.MinValue, Math.Min(short.MaxValue, s * gain * short.MaxValue)));
                w.Flush();
                return stream.ToArray();
            }
        }
        static void Note(double[] samples, int rate, double start, double frequency, double amplitude)
        {
            int first = (int)(start * rate);
            for (int i = first; i < samples.Length; i++)
            {
                double t = (i - first) / (double)rate;
                double envelope = (1 - Math.Exp(-t / 0.003)) * Math.Exp(-t / 0.32);
                double tone = Math.Sin(2 * Math.PI * frequency * t)
                    + 0.32 * Math.Sin(2 * Math.PI * frequency * 2 * t) * Math.Exp(-t / 0.12)
                    + 0.12 * Math.Sin(2 * Math.PI * frequency * 3.01 * t) * Math.Exp(-t / 0.06);
                samples[i] += amplitude * envelope * tone;
            }
        }
    }
}
