using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Xml;
using System.Xml.Linq;

namespace WiiUAchievements
{
    public sealed class GameSnapshot
    {
        public int Rupees, Health, MaxHealth;
        public DateTime Utc;
        public static int U16(byte[] bytes, int offset) { return (bytes[offset] << 8) | bytes[offset + 1]; }
        public static GameSnapshot Parse(byte[] bytes)
        {
            if (bytes == null || bytes.Length < 6) throw new InvalidDataException("Campione di memoria incompleto.");
            var s = new GameSnapshot { MaxHealth = U16(bytes, 0), Health = U16(bytes, 2), Rupees = U16(bytes, 4), Utc = DateTime.UtcNow };
            new Observation { MaxHealth = s.MaxHealth, Health = s.Health, Rupees = s.Rupees }.Validate();
            return s;
        }
        public int Value(string field)
        {
            switch (field)
            {
                case "rupees": return Rupees;
                case "max_health": return MaxHealth;
                default: throw new InvalidDataException("Campo sconosciuto: " + field);
            }
        }
    }

    public sealed class Achievement
    {
        public string Id, Title, Description, Field;
        public int Threshold, Points;
        public bool Met(GameSnapshot s) { return s.Value(Field) >= Threshold; }
    }
    public static class AchievementSet
    {
        public const string Revision = "wwhd-alpha-1";
        public static List<Achievement> Load(string path)
        {
            var doc = SafeXml.Load(path);
            if (doc.Root == null || doc.Root.Name != "achievement-set" || (string)doc.Root.Attribute("revision") != Revision)
                throw new InvalidDataException("Set achievement incompatibile.");
            var results = new List<Achievement>();
            foreach (var x in doc.Root.Elements("achievement"))
            {
                var a = new Achievement {
                    Id = (string)x.Attribute("id"), Title = (string)x.Attribute("title"),
                    Description = (string)x.Attribute("description"), Field = (string)x.Attribute("field"),
                    Threshold = (int)x.Attribute("threshold"), Points = (int)x.Attribute("points") };
                if (String.IsNullOrWhiteSpace(a.Id) || String.IsNullOrWhiteSpace(a.Title) ||
                    results.Any(t => t.Id == a.Id) || (a.Field != "rupees" && a.Field != "max_health") || a.Threshold <= 0 || a.Points <= 0)
                    throw new InvalidDataException("Definizione achievement non valida.");
                results.Add(a);
            }
            if (results.Count == 0) throw new InvalidDataException("Il set non contiene obiettivi.");
            return results;
        }
    }

    public static class SafeXml
    {
        public static XDocument Load(string path)
        {
            var settings = new XmlReaderSettings { DtdProcessing = DtdProcessing.Prohibit, XmlResolver = null, MaxCharactersInDocument = 1024 * 1024 };
            using (var reader = XmlReader.Create(path, settings)) return XDocument.Load(reader);
        }
        public static void AtomicSave(XDocument doc, string path)
        {
            Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(path)));
            string temp = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
            try
            {
                doc.Save(temp);
                if (File.Exists(path)) File.Replace(temp, path, path + ".bak");
                else File.Move(temp, path);
            }
            finally { if (File.Exists(temp)) File.Delete(temp); }
        }
    }

    public sealed class Unlock
    {
        public string Id;
        public DateTime Utc;
    }
    public sealed class ProgressStore
    {
        readonly string path, identity;
        readonly Dictionary<string, Unlock> unlocked = new Dictionary<string, Unlock>();
        public string Warning { get; private set; }
        public ProgressStore(string directory, string player, string gameIdentity)
        {
            if (String.IsNullOrWhiteSpace(player)) throw new ArgumentException("Inserisci un nome giocatore.");
            identity = player.Trim() + "|" + gameIdentity + "|" + AchievementSet.Revision;
            string key;
            using (var sha = SHA256.Create()) key = BitConverter.ToString(sha.ComputeHash(Encoding.UTF8.GetBytes(identity))).Replace("-", "").Substring(0, 24);
            path = Path.Combine(directory, key + ".xml");
            if (!File.Exists(path)) return;
            try { Load(path); }
            catch (Exception ex)
            {
                if (!(ex is XmlException || ex is InvalidDataException || ex is FormatException || ex is ArgumentException)) throw;
                unlocked.Clear();
                // Do not silently overwrite corrupt progress with an empty document.
                string damaged = path + ".damaged-" + DateTime.UtcNow.ToString("yyyyMMddHHmmssfff");
                File.Copy(path, damaged, false);
                if (File.Exists(path + ".bak"))
                {
                    try { Load(path + ".bak"); Warning = "Progressi recuperati dal backup; originale danneggiato conservato."; }
                    catch { unlocked.Clear(); throw new InvalidDataException("Progressi e backup non leggibili. Copie conservate: " + damaged); }
                }
                else throw new InvalidDataException("Progressi non leggibili. Originale conservato: " + damaged);
            }
        }
        void Load(string file)
        {
            var doc = SafeXml.Load(file);
            if (doc.Root == null || doc.Root.Name != "progress" || (string)doc.Root.Attribute("identity") != identity)
                throw new InvalidDataException("Identità dei progressi non valida.");
            var temp = new Dictionary<string, Unlock>();
            foreach (var x in doc.Root.Elements("unlock"))
            {
                string id = (string)x.Attribute("id");
                if (String.IsNullOrWhiteSpace(id) || temp.ContainsKey(id)) throw new InvalidDataException("Sblocco non valido.");
                temp.Add(id, new Unlock { Id = id, Utc = DateTime.Parse((string)x.Attribute("utc"), CultureInfo.InvariantCulture, DateTimeStyles.RoundtripKind).ToUniversalTime() });
            }
            unlocked.Clear();
            foreach (var item in temp) unlocked.Add(item.Key, item.Value);
        }
        public bool Contains(string id) { return unlocked.ContainsKey(id); }
        public Unlock Get(string id) { Unlock value; return unlocked.TryGetValue(id, out value) ? value : null; }
        public void Award(string id, DateTime utc)
        {
            if (Contains(id)) return;
            var entries = unlocked.Values.Concat(new[] { new Unlock { Id = id, Utc = utc } }).OrderBy(x => x.Utc);
            var doc = new XDocument(new XElement("progress", new XAttribute("identity", identity),
                entries.Select(x => new XElement("unlock", new XAttribute("id", x.Id), new XAttribute("utc", x.Utc.ToString("o", CultureInfo.InvariantCulture))))));
            SafeXml.AtomicSave(doc, path); // Commit on disk before publishing an unlock.
            unlocked.Add(id, new Unlock { Id = id, Utc = utc });
        }
    }

    public sealed class AchievementEngine
    {
        readonly List<Achievement> achievements;
        readonly ProgressStore store;
        readonly Dictionary<string, bool> armed = new Dictionary<string, bool>();
        readonly Dictionary<string, int> streak = new Dictionary<string, int>();
        DateTime? lastSample;
        public AchievementEngine(List<Achievement> rules, ProgressStore progress) { achievements = rules; store = progress; Reset(); }
        public void Reset()
        {
            lastSample = null;
            foreach (var a in achievements) { armed[a.Id] = false; streak[a.Id] = 0; }
        }
        public List<Achievement> Update(GameSnapshot s)
        {
            if (lastSample.HasValue && (s.Utc <= lastSample.Value || (s.Utc - lastSample.Value).TotalSeconds > 1)) Reset();
            var awards = new List<Achievement>();
            foreach (var a in achievements)
            {
                if (store.Contains(a.Id)) continue;
                bool met = a.Met(s);
                // First sample only establishes a baseline. Existing save progress is not awarded.
                if (!lastSample.HasValue) { armed[a.Id] = !met; streak[a.Id] = 0; continue; }
                if (!met) { armed[a.Id] = true; streak[a.Id] = 0; continue; }
                if (!armed[a.Id]) continue;
                if (++streak[a.Id] < 3) continue;
                store.Award(a.Id, s.Utc);
                awards.Add(a);
            }
            lastSample = s.Utc;
            return awards;
        }
    }

    public static class GameIdentity
    {
        public static bool IsWindWaker(string title)
        {
            if (String.IsNullOrWhiteSpace(title)) return false;
            return title.IndexOf("wind waker", StringComparison.OrdinalIgnoreCase) >= 0 &&
                title.IndexOf("HD", StringComparison.OrdinalIgnoreCase) >= 0;
        }
    }
}
