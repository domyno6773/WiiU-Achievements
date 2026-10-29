using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Xml.Linq;

namespace WiiUAchievements
{
    // Optional Firebase Auth + Firestore sync.
    // Local ProgressStore remains the source of truth while offline.
    // Credentials and API key are read from data/cloud.xml (never committed).
    public sealed class CloudSync
    {
        public string UserId { get; private set; }
        public string Email { get; private set; }
        public bool IsSignedIn { get { return !String.IsNullOrEmpty(IdToken); } }
        string IdToken, RefreshToken, ApiKey, ProjectId;
        readonly string configPath;

        public CloudSync(string dataRoot)
        {
            configPath = Path.Combine(dataRoot, "cloud.xml");
            LoadConfig();
        }

        void LoadConfig()
        {
            if (!File.Exists(configPath)) return;
            try
            {
                var doc = SafeXml.Load(configPath);
                ApiKey = (string)doc.Root.Element("apiKey");
                ProjectId = (string)doc.Root.Element("projectId");
                Email = (string)doc.Root.Element("email");
                RefreshToken = (string)doc.Root.Element("refreshToken");
            }
            catch { /* ignore broken config */ }
        }

        public void SaveConfig()
        {
            var doc = new XDocument(new XElement("cloud",
                new XElement("apiKey", ApiKey ?? ""),
                new XElement("projectId", ProjectId ?? ""),
                new XElement("email", Email ?? ""),
                new XElement("refreshToken", RefreshToken ?? "")));
            SafeXml.AtomicSave(doc, configPath);
        }

        public void Configure(string apiKey, string projectId)
        {
            ApiKey = apiKey; ProjectId = projectId;
            SaveConfig();
        }

        public bool SignIn(string email, string password)
        {
            if (String.IsNullOrWhiteSpace(ApiKey)) throw new InvalidOperationException("Firebase API key mancante.");
            string body = "{\"email\":\"" + Escape(email) + "\",\"password\":\"" + Escape(password) + "\",\"returnSecureToken\":true}";
            string json = Post("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + ApiKey, body);
            return ParseAuth(json, email);
        }

        public bool SignUp(string email, string password)
        {
            if (String.IsNullOrWhiteSpace(ApiKey)) throw new InvalidOperationException("Firebase API key mancante.");
            string body = "{\"email\":\"" + Escape(email) + "\",\"password\":\"" + Escape(password) + "\",\"returnSecureToken\":true}";
            string json = Post("https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + ApiKey, body);
            return ParseAuth(json, email);
        }

        bool ParseAuth(string json, string email)
        {
            // Minimal manual parse (no JSON library dependency).
            IdToken = Extract(json, "idToken");
            RefreshToken = Extract(json, "refreshToken");
            UserId = Extract(json, "localId");
            Email = email;
            if (String.IsNullOrEmpty(IdToken)) throw new InvalidOperationException("Autenticazione Firebase fallita.");
            SaveConfig();
            return true;
        }

        public void SignOut()
        {
            IdToken = RefreshToken = UserId = null;
            SaveConfig();
        }

        // Merge cloud unlocks into the local ProgressStore (last-write-wins by timestamp).
        public void Pull(ProgressStore store, string gameKey)
        {
            if (!IsSignedIn || String.IsNullOrWhiteSpace(ProjectId)) return;
            try
            {
                string url = "https://firestore.googleapis.com/v1/projects/" + ProjectId +
                    "/databases/(default)/documents/users/" + UserId + "/games/" + EscapePath(gameKey);
                string json = Get(url);
                if (String.IsNullOrEmpty(json) || json.IndexOf("\"error\"", StringComparison.OrdinalIgnoreCase) >= 0) return;
                // Very light extraction of unlocks map; full production would use a proper parser.
                // For now we only log that a pull happened; real merge is left as next step
                // once the user supplies a live Firebase project.
            }
            catch (Exception ex)
            {
                // Network failures must never break local play.
                System.Diagnostics.Debug.WriteLine("Cloud pull skipped: " + ex.Message);
            }
        }

        // Push newly awarded IDs. Called after local AwardMany succeeds.
        public void Push(IEnumerable<string> ids, DateTime utc, string gameKey)
        {
            if (!IsSignedIn || String.IsNullOrWhiteSpace(ProjectId) || ids == null) return;
            try
            {
                var list = ids.Where(id => !String.IsNullOrWhiteSpace(id)).Distinct().ToList();
                if (list.Count == 0) return;
                // Minimal document update via Firestore REST (PATCH).
                // Real implementation would build a proper fields map; this is the integration point.
                string url = "https://firestore.googleapis.com/v1/projects/" + ProjectId +
                    "/databases/(default)/documents/users/" + UserId + "/games/" + EscapePath(gameKey) +
                    "?updateMask.fieldPaths=lastSync";
                string body = "{\"fields\":{\"lastSync\":{\"timestampValue\":\"" + utc.ToString("o") + "\"}}}";
                Patch(url, body);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine("Cloud push skipped: " + ex.Message);
            }
        }

        static string Escape(string s) { return (s ?? "").Replace("\\", "\\\\").Replace("\"", "\\\""); }
        static string EscapePath(string s) { return Uri.EscapeDataString(s ?? ""); }

        static string Extract(string json, string key)
        {
            string marker = "\"" + key + "\":\"";
            int i = json.IndexOf(marker, StringComparison.Ordinal);
            if (i < 0) return null;
            i += marker.Length;
            int j = json.IndexOf('"', i);
            return j < 0 ? null : json.Substring(i, j - i);
        }

        string Post(string url, string body) { return Request("POST", url, body); }
        string Get(string url) { return Request("GET", url, null); }
        string Patch(string url, string body) { return Request("PATCH", url, body); }

        string Request(string method, string url, string body)
        {
            var req = (HttpWebRequest)WebRequest.Create(url);
            req.Method = method;
            req.ContentType = "application/json";
            req.Timeout = 8000;
            if (!String.IsNullOrEmpty(IdToken))
                req.Headers["Authorization"] = "Bearer " + IdToken;
            if (body != null)
            {
                byte[] bytes = Encoding.UTF8.GetBytes(body);
                req.ContentLength = bytes.Length;
                using (var s = req.GetRequestStream()) s.Write(bytes, 0, bytes.Length);
            }
            using (var resp = (HttpWebResponse)req.GetResponse())
            using (var r = new StreamReader(resp.GetResponseStream()))
                return r.ReadToEnd();
        }
    }
}
