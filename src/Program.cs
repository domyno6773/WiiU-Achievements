using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace WiiUAchievements
{
    internal static class Program
    {
        [STAThread]
        static int Main(string[] args)
        {
            try
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                string instance;
                using (var sha = SHA256.Create()) instance = BitConverter.ToString(sha.ComputeHash(Encoding.UTF8.GetBytes(AppDomain.CurrentDomain.BaseDirectory))).Replace("-", "").Substring(0, 24);
                bool first;
                using (var mutex = new Mutex(true, "Local\\WiiUAchievements-" + instance, out first))
                {
                if (!first) { MessageBox.Show("Questa copia dell'app è già aperta.", "Wii U Achievements"); return 0; }
                using (var form = new MainForm(args.Contains("--demo"), args.Contains("--connect")))
                {
                    var preview = args.FirstOrDefault(a => a.StartsWith("--preview="));
                    if (preview != null)
                    {
                        var timer = new System.Windows.Forms.Timer { Interval = 1200 };
                        timer.Tick += delegate { timer.Stop(); form.SavePreview(Path.GetFullPath(preview.Substring(10))); form.Close(); timer.Dispose(); };
                        timer.Start();
                    }
                    Application.Run(form);
                }
                }
                return 0;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Wii U Achievements", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }
        }
    }
}
