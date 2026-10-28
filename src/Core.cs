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
        public void Validate()
        {
            new Observation { MaxHealth = MaxHealth, Health = Health, Rupees = Rupees }.Validate();
            if (Utc.Kind != DateTimeKind.Utc || Utc == DateTime.MinValue)
                throw new ArgumentException("Il campione deve avere un timestamp UTC valido.");
        }
        public static int U16(byte[] bytes, int offset) { return (bytes[offset] << 8) | bytes[offset + 1]; }
        public static GameSnapshot Parse(byte[] bytes)
        {
            if (bytes == null || bytes.Length < 6) throw new InvalidDataException("Campione di memoria incompleto.");
            var s = new GameSnapshot { MaxHealth = U16(bytes, 0), Health = U16(bytes, 2), Rupees = U16(bytes, 4), Utc = DateTime.UtcNow };
            s.Validate();
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
            return Load(path, Revision);
        }
        public static List<Achievement> Load(string path, string revision)
        {
            var doc = SafeXml.Load(path);
            if (String.IsNullOrWhiteSpace(revision) || doc.Root == null || doc.Root.Name != "achievement-set" || (string)doc.Root.Attribute("revision") != revision)
                throw new InvalidDataException("Set achievement incompatibile.");
            if (doc.Root.Elements().Any(x => x.Name != "achievement"))
                throw new InvalidDataException("Elemento sconosciuto nel set achievement.");
            var results = new List<Achievement>();
            foreach (var x in doc.Root.Elements("achievement"))
            {
                string[] attributes = { "id", "title", "description", "field", "threshold", "points" };
                if (x.HasElements || x.Attributes().Any(attribute => !attributes.Contains(attribute.Name.ToString())))
                    throw new InvalidDataException("Condizione achievement non supportata.");
                int threshold, points;
                if (!Int32.TryParse((string)x.Attribute("threshold"), NumberStyles.None, CultureInfo.InvariantCulture, out threshold) ||
                    !Int32.TryParse((string)x.Attribute("points"), NumberStyles.None, CultureInfo.InvariantCulture, out points))
                    throw new InvalidDataException("Soglia o punteggio achievement non valido.");
                var a = new Achievement {
                    Id = (string)x.Attribute("id"), Title = (string)x.Attribute("title"),
                    Description = (string)x.Attribute("description"), Field = (string)x.Attribute("field"),
                    Threshold = threshold, Points = points };
                if (String.IsNullOrWhiteSpace(a.Id) || String.IsNullOrWhiteSpace(a.Title) ||
                    String.IsNullOrWhiteSpace(a.Description) || a.Id != a.Id.Trim() || a.Id.Length > 128 ||
                    results.Any(t => t.Id == a.Id) || (a.Field != "rupees" && a.Field != "max_health") || a.Threshold <= 0 || a.Points <= 0 ||
                    (a.Field == "rupees" && a.Threshold > 5000) ||
                    (a.Field == "max_health" && (a.Threshold < 12 || a.Threshold > 80 || a.Threshold % 4 != 0)))
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
            : this(directory, player, gameIdentity, AchievementSet.Revision) { }
        public ProgressStore(string directory, string player, string gameIdentity, string revision)
        {
            if (String.IsNullOrWhiteSpace(player)) throw new ArgumentException("Inserisci un nome giocatore.");
            if (String.IsNullOrWhiteSpace(gameIdentity) || String.IsNullOrWhiteSpace(revision)) throw new ArgumentException("Identità del gioco o revisione mancante.");
            identity = player.Trim() + "|" + gameIdentity + "|" + revision;
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
        public Unlock Get(string id)
        {
            Unlock value;
            return unlocked.TryGetValue(id, out value) ? new Unlock { Id = value.Id, Utc = value.Utc } : null;
        }
        public void Award(string id, DateTime utc)
        {
            AwardMany(new[] { id }, utc);
        }
        public void AwardMany(IEnumerable<string> ids, DateTime utc)
        {
            if (ids == null) throw new ArgumentNullException("ids");
            if (utc.Kind != DateTimeKind.Utc || utc == DateTime.MinValue) throw new ArgumentException("Timestamp sblocco non valido.");
            var requested = ids.Distinct().ToList();
            if (requested.Any(String.IsNullOrWhiteSpace)) throw new ArgumentException("ID sblocco non valido.");
            var additions = requested.Where(id => !Contains(id)).Select(id => new Unlock { Id = id, Utc = utc }).ToList();
            if (additions.Count == 0) return;
            var entries = unlocked.Values.Concat(additions).OrderBy(x => x.Utc).ThenBy(x => x.Id, StringComparer.Ordinal);
            var doc = new XDocument(new XElement("progress", new XAttribute("identity", identity),
                entries.Select(x => new XElement("unlock", new XAttribute("id", x.Id), new XAttribute("utc", x.Utc.ToString("o", CultureInfo.InvariantCulture))))));
            SafeXml.AtomicSave(doc, path); // All awards from a sample commit together, before publishing any.
            foreach (var item in additions) unlocked.Add(item.Id, item);
        }
    }

    public enum AchievementPhase { WaitingForBaseline, Stabilizing, AlreadySatisfied, Tracking, Confirming, Unlocked }
    public sealed class AchievementStatus
    {
        public AchievementPhase Phase { get; internal set; }
        public int Confirmations { get; internal set; }
    }

    public sealed class AchievementEngine
    {
        readonly List<Achievement> achievements;
        readonly ProgressStore store;
        readonly Dictionary<string, bool> armed = new Dictionary<string, bool>();
        readonly Dictionary<string, int> streak = new Dictionary<string, int>();
        readonly Dictionary<string, int> below = new Dictionary<string, int>();
        readonly Dictionary<string, AchievementStatus> states = new Dictionary<string, AchievementStatus>();
        public const int RequiredSamples = 3;
        public int StateVersion { get; private set; }
        DateTime? lastSample;
        public AchievementEngine(List<Achievement> rules, ProgressStore progress)
        {
            if (rules == null || progress == null) throw new ArgumentNullException(rules == null ? "rules" : "progress");
            if (rules.Any(a => a == null || String.IsNullOrWhiteSpace(a.Id)) || rules.Select(a => a.Id).Distinct().Count() != rules.Count)
                throw new ArgumentException("Regole duplicate o non valide.");
            achievements = new List<Achievement>(rules); store = progress; Reset();
        }
        void SetState(string id, AchievementPhase phase, int confirmations)
        {
            AchievementStatus old;
            if (states.TryGetValue(id, out old) && old.Phase == phase && old.Confirmations == confirmations) return;
            states[id] = new AchievementStatus { Phase = phase, Confirmations = confirmations };
            StateVersion++;
        }
        public AchievementStatus Status(string id)
        {
            var state = states[id];
            return new AchievementStatus { Phase = state.Phase, Confirmations = state.Confirmations };
        }
        public void Reset()
        {
            lastSample = null;
            foreach (var a in achievements)
            {
                armed[a.Id] = false; streak[a.Id] = 0; below[a.Id] = 0;
                SetState(a.Id, store.Contains(a.Id) ? AchievementPhase.Unlocked : AchievementPhase.WaitingForBaseline, 0);
            }
        }
        public List<Achievement> Update(GameSnapshot s)
        {
            try { if (s == null) throw new ArgumentNullException("s"); s.Validate(); }
            catch { Reset(); throw; }
            if (lastSample.HasValue && (s.Utc <= lastSample.Value || (s.Utc - lastSample.Value).TotalSeconds > 1)) Reset();
            var awards = new List<Achievement>();
            foreach (var a in achievements)
            {
                if (store.Contains(a.Id)) continue;
                bool met = a.Met(s);
                if (!met)
                {
                    streak[a.Id] = 0;
                    below[a.Id] = Math.Min(RequiredSamples, below[a.Id] + 1);
                    if (below[a.Id] >= RequiredSamples) armed[a.Id] = true;
                    SetState(a.Id, armed[a.Id] ? AchievementPhase.Tracking : AchievementPhase.Stabilizing, 0);
                    continue;
                }
                below[a.Id] = 0;
                if (!armed[a.Id]) { SetState(a.Id, AchievementPhase.AlreadySatisfied, 0); continue; }
                streak[a.Id]++;
                SetState(a.Id, AchievementPhase.Confirming, streak[a.Id]);
                if (streak[a.Id] < RequiredSamples) continue;
                awards.Add(a);
            }
            try { store.AwardMany(awards.Select(a => a.Id), s.Utc); }
            catch { Reset(); throw; }
            foreach (var a in awards) SetState(a.Id, AchievementPhase.Unlocked, 0);
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
