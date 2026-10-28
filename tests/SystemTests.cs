using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Xml.Linq;
using WiiUAchievements;

static class SystemTests
{
    static GameSnapshot Sample(int value, int frame)
    {
        return new GameSnapshot { Rupees = value, MaxHealth = 12, Health = 12,
            Utc = new DateTime(2026, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddMilliseconds(frame * 100) };
    }
    static Achievement Rule(string id, int threshold)
    {
        return new Achievement { Id = id, Title = id, Description = id, Field = "rupees", Threshold = threshold, Points = 1 };
    }
    static bool Rejects(Action action)
    {
        try { action(); return false; }
        catch (ArgumentException) { return true; }
        catch (InvalidDataException) { return true; }
    }
    public static void Run(Action<bool, string> check, string temporary, string rulesFile)
    {
        var rules = new List<Achievement> { Rule("a", 10), Rule("b", 20) };
        var directory = Path.Combine(temporary, "batch");
        var store = new ProgressStore(directory, "player", "game");
        var engine = new AchievementEngine(rules, store);
        check(engine.Status("a").Phase == AchievementPhase.WaitingForBaseline, "Initial engine state is explicit");
        engine.Update(Sample(0, 0)); engine.Update(Sample(0, 1));
        check(engine.Status("a").Phase == AchievementPhase.Stabilizing, "Baseline must settle below the threshold");
        engine.Update(Sample(0, 2));
        check(engine.Status("a").Phase == AchievementPhase.Tracking, "Three below-threshold samples arm the objective");
        int version = engine.StateVersion;
        engine.Update(Sample(0, 3));
        check(engine.StateVersion == version, "Steady samples do not request redundant UI updates");
        engine.Update(Sample(20, 4));
        check(engine.Status("a").Phase == AchievementPhase.Confirming && engine.Status("a").Confirmations == 1, "UI can observe confirmation without mirroring engine logic");
        var status = engine.Status("a");
        engine.Update(Sample(20, 5));
        check(status.Confirmations == 1 && engine.Status("a").Confirmations == 2, "Status snapshots are independent");
        var awards = engine.Update(Sample(20, 6));
        check(awards.Count == 2 && engine.Status("a").Phase == AchievementPhase.Unlocked, "Simultaneous objectives complete together");
        var loaded = new ProgressStore(directory, "player", "game");
        check(loaded.Contains("a") && loaded.Contains("b") && loaded.Get("a").Utc == loaded.Get("b").Utc && Directory.GetFiles(directory, "*.bak").Length == 0,
            "First simultaneous unlock is one atomic save, not partial writes");
        var unlocked = loaded.Get("a"); unlocked.Id = "changed";
        check(loaded.Get("a").Id == "a", "Callers cannot mutate stored unlock records");
        check(!new ProgressStore(directory, "player", "other-game").Contains("a") &&
            !new ProgressStore(directory, "player", "game", "other-revision").Contains("a"), "Game and set revision isolate progress");
        string legacy = Path.Combine(temporary, "legacy"); Directory.CreateDirectory(legacy);
        File.WriteAllText(Path.Combine(legacy, "C44973C3A96CAEB703398F6E.xml"),
            "<progress identity=\"player|EUR-v0|wwhd-alpha-1\"><unlock id=\"wallet-10\" utc=\"2026-10-28T10:00:00Z\" /></progress>");
        check(new ProgressStore(legacy, "player", "EUR-v0", GameCatalog.Default.SetRevision).Contains("wallet-10"),
            "A fixed legacy filename and XML fixture load without migration");

        store = new ProgressStore(Path.Combine(temporary, "low-glitch"), "player", "game"); engine = new AchievementEngine(rules, store);
        engine.Update(Sample(20, 0)); engine.Update(Sample(0, 1));
        for (int i = 2; i < 7; i++) engine.Update(Sample(20, i));
        check(!store.Contains("a") && engine.Status("a").Phase == AchievementPhase.AlreadySatisfied, "A one-sample drop cannot arm an already satisfied save");
        engine.Update(Sample(0, 7)); engine.Update(Sample(0, 8)); engine.Update(Sample(0, 9));
        engine.Update(Sample(20, 10)); engine.Update(Sample(20, 11));
        var invalid = Sample(20, 12); invalid.Health = 13;
        check(Rejects(() => engine.Update(invalid)) && engine.Status("a").Phase == AchievementPhase.WaitingForBaseline,
            "Invalid public snapshots reset pending unlocks");
        for (int i = 13; i < 18; i++) engine.Update(Sample(20, i));
        check(!store.Contains("a"), "Invalid data cannot carry a pending crossing into the next baseline");
        check(Rejects(() => engine.Update(null)) && Rejects(() => engine.Update(new GameSnapshot { Health = 12, MaxHealth = 12 })),
            "Null snapshots and missing timestamps fail closed");
        engine.Update(Sample(0, 20)); engine.Update(Sample(0, 21)); engine.Update(Sample(0, 22)); engine.Update(Sample(20, 23));
        engine.Update(Sample(20, 23)); engine.Update(Sample(20, 24)); engine.Update(Sample(20, 25));
        check(!store.Contains("a") && engine.Status("a").Phase == AchievementPhase.AlreadySatisfied, "Duplicate timestamps reset continuity");
        engine.Update(Sample(0, 26)); engine.Update(Sample(0, 27)); engine.Update(Sample(0, 28)); engine.Update(Sample(20, 29));
        engine.Update(Sample(20, 60));
        check(engine.Status("a").Phase == AchievementPhase.AlreadySatisfied && engine.Status("a").Confirmations == 0,
            "Gap resets the UI state as well as the pending streak");

        string blocked = Path.Combine(temporary, "not-a-directory"); File.WriteAllText(blocked, "blocked");
        store = new ProgressStore(Path.Combine(blocked, "progress"), "player", "game"); engine = new AchievementEngine(rules, store);
        for (int i = 0; i < 3; i++) engine.Update(Sample(0, i));
        engine.Update(Sample(20, 3)); engine.Update(Sample(20, 4)); bool failed = false;
        try { engine.Update(Sample(20, 5)); } catch (IOException) { failed = true; }
        check(failed && !store.Contains("a") && !store.Contains("b") && engine.Status("a").Phase == AchievementPhase.WaitingForBaseline,
            "A failed save publishes no partial award and clears pending state");
        File.Delete(blocked);
        for (int i = 6; i < 9; i++) engine.Update(Sample(20, i));
        check(!store.Contains("a"), "Storage recovery does not award an already satisfied baseline");
        for (int i = 9; i < 12; i++) engine.Update(Sample(0, i));
        engine.Update(Sample(20, 12)); engine.Update(Sample(20, 13));
        check(engine.Update(Sample(20, 14)).Count == 2, "A fresh crossing works after storage recovery");

        var original = XDocument.Load(rulesFile);
        check(AchievementSet.Load(rulesFile).Count == 10, "Existing ten objectives and revision remain loadable");
        string bad = Path.Combine(temporary, "bad-rules.xml");
        Action<XDocument> save = doc => doc.Save(bad);
        var changed = new XDocument(original); changed.Root.Elements().First().SetAttributeValue("threshhold", 20); save(changed);
        check(Rejects(() => AchievementSet.Load(bad)), "Unsupported condition attributes cannot be silently ignored");
        changed = new XDocument(original); changed.Root.Elements().First().SetAttributeValue("threshold", 5001); save(changed);
        check(Rejects(() => AchievementSet.Load(bad)), "Unreachable rupee thresholds are rejected");
        changed = new XDocument(original); changed.Root.Elements().First(x => (string)x.Attribute("field") == "max_health").SetAttributeValue("threshold", 17); save(changed);
        check(Rejects(() => AchievementSet.Load(bad)), "Heart capacity rules require whole attainable hearts");
        changed = new XDocument(original); changed.Root.Add(new XElement("condition")); save(changed);
        check(Rejects(() => AchievementSet.Load(bad)), "Unknown rule elements fail rather than disappearing");
        changed = new XDocument(original); changed.Root.Elements().Skip(1).First().SetAttributeValue("id", (string)changed.Root.Elements().First().Attribute("id")); save(changed);
        check(Rejects(() => AchievementSet.Load(bad)), "Duplicate achievement IDs are rejected");
        check(Rejects(() => AchievementSet.Load(rulesFile, "wrong-revision")), "Each game requires its own expected set revision");

        var heartStore = new ProgressStore(Path.Combine(temporary, "hearts"), "player", "game");
        var heartRule = new Achievement { Id = "heart", Title = "heart", Description = "heart", Field = "max_health", Threshold = 16, Points = 1 };
        var heartEngine = new AchievementEngine(new List<Achievement> { heartRule }, heartStore);
        for (int i = 0; i < 3; i++) heartEngine.Update(Sample(0, i));
        for (int i = 3; i < 6; i++) { var heartSample = Sample(0, i); heartSample.MaxHealth = 16; heartEngine.Update(heartSample); }
        check(heartStore.Contains("heart"), "Heart capacity uses the same stable transition and persistence path");

        var game = GameCatalog.Default;
        var session = new GameSession { CemuVersion = "2.6", TitleId = GameSession.EurTitleId, Version = "v0", RpxHash = "CDC68ACD" };
        check(game.RequireProfile(session).CreateReader() is WindWakerMemoryReader && game.SetRevision == AchievementSet.Revision,
            "Catalog routes the exact supported build to its original reader");
        session.TitleId = "0005000010143500";
        bool unsupported = false;
        try { game.RequireProfile(session); } catch (InvalidOperationException) { unsupported = true; }
        check(unsupported, "A different region cannot use the selected game's memory reader");
        session.TitleId = GameSession.EurTitleId; session.CemuVersion = "2.7";
        check(!game.Profiles[0].Matches(session), "An unsupported Cemu version is rejected before reading memory");
        session.CemuVersion = "2.6"; session.RpxHash = "00000000";
        check(!game.Profiles[0].Matches(session), "An unknown executable hash is rejected before reading memory");
        var future = GameCatalog.Games.First(g => !g.Available);
        unsupported = false;
        try { future.LoadRules(temporary); } catch (InvalidOperationException) { unsupported = true; }
        check(unsupported && future.Profiles.Count == 0, "Unavailable catalog entries cannot load rules or attach");
        check(Rejects(() => new GameDefinition("x", "x", "x", null, null, game.Profiles[0])),
            "Future supported games require a real achievement set");
    }
}
