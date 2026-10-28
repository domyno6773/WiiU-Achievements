using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;

namespace WiiUAchievements
{
    // A catalog entry is not support by itself: available games require a reader
    // and an exact, validated profile. Never guess an adapter for an unknown title.
    public interface IGameMemoryReader
    {
        GameSnapshot Read(ProcessMemory memory, ulong guestBase);
    }

    public sealed class WindWakerMemoryReader : IGameMemoryReader
    {
        public GameSnapshot Read(ProcessMemory memory, ulong guestBase)
        {
            return GameSnapshot.Parse(memory.Read(checked(guestBase + 0x145B7B80U), 6));
        }
    }

    public sealed class GameProfile
    {
        public string Label { get; private set; }
        public string CemuVersion { get; private set; }
        public string TitleId { get; private set; }
        public string TitleVersion { get; private set; }
        public string RpxHash { get; private set; }
        readonly Func<IGameMemoryReader> createReader;

        public GameProfile(string label, string cemuVersion, string titleId, string titleVersion, string rpxHash, Func<IGameMemoryReader> reader)
        {
            if (new[] { label, cemuVersion, titleId, titleVersion, rpxHash }.Any(String.IsNullOrWhiteSpace) || reader == null)
                throw new ArgumentException("Profilo del gioco incompleto.");
            Label = label; CemuVersion = cemuVersion; TitleId = titleId; TitleVersion = titleVersion; RpxHash = rpxHash;
            createReader = reader;
        }
        public bool Matches(GameSession session)
        {
            return session != null && session.CemuVersion == CemuVersion && session.Version == TitleVersion &&
                String.Equals(session.TitleId, TitleId, StringComparison.OrdinalIgnoreCase) &&
                String.Equals(session.RpxHash, RpxHash, StringComparison.OrdinalIgnoreCase);
        }
        public IGameMemoryReader CreateReader() { return createReader(); }
    }

    public sealed class GameDefinition
    {
        public string Id { get; private set; }
        public string Title { get; private set; }
        public string Description { get; private set; }
        public string RulesFile { get; private set; }
        public string SetRevision { get; private set; }
        public ReadOnlyCollection<GameProfile> Profiles { get; private set; }
        public bool Available { get { return Profiles.Count > 0; } }

        public GameDefinition(string id, string title, string description, string rulesFile, string revision, params GameProfile[] profiles)
        {
            if (String.IsNullOrWhiteSpace(id) || String.IsNullOrWhiteSpace(title) || profiles == null || profiles.Any(p => p == null))
                throw new ArgumentException("Voce del catalogo non valida.");
            if (profiles.Length > 0 && (String.IsNullOrWhiteSpace(rulesFile) || String.IsNullOrWhiteSpace(revision)))
                throw new ArgumentException("Un gioco disponibile richiede un set achievement.");
            Id = id; Title = title; Description = description; RulesFile = rulesFile; SetRevision = revision;
            Profiles = Array.AsReadOnly((GameProfile[])profiles.Clone());
        }
        public GameProfile RequireProfile(GameSession session)
        {
            var profile = Profiles.FirstOrDefault(p => p.Matches(session));
            if (profile == null) throw new InvalidOperationException("Configurazione non supportata per " + Title + ". Controlla le versioni nella sezione Giochi.");
            return profile;
        }
        public List<Achievement> LoadRules(string root)
        {
            if (!Available) throw new InvalidOperationException("Questo gioco non è ancora disponibile.");
            return AchievementSet.Load(Path.Combine(root, RulesFile), SetRevision);
        }
    }

    public static class GameCatalog
    {
        public const string WindWakerId = "wind-waker-hd";
        static readonly ReadOnlyCollection<GameDefinition> games = Array.AsReadOnly(new[] {
            new GameDefinition(WindWakerId, "The Wind Waker HD",
                "Set iniziale sperimentale: rupie e capacità di vita.", "achievements.xml", AchievementSet.Revision,
                new GameProfile("EUR v0", "2.6", GameSession.EurTitleId, "v0", "cdc68acd", () => new WindWakerMemoryReader())),
            new GameDefinition("future-games", "Altri giochi", "Non ancora disponibili. Il catalogo si amplierà con i prossimi giochi supportati.", null, null)
        });
        public static ReadOnlyCollection<GameDefinition> Games { get { return games; } }
        public static GameDefinition Default { get { return games.First(g => g.Id == WindWakerId); } }
    }
}
