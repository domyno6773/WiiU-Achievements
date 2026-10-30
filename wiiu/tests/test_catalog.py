#!/usr/bin/env python3
"""Host-side validation of canonical XML -> native Wii U catalog conversion."""
import copy
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "tools"))
import generate_catalog as catalog

ROOT = pathlib.Path(__file__).resolve().parents[2]


class CatalogTests(unittest.TestCase):
    def test_canonical_counts_and_parity(self):
        revision, game, entries = catalog.load_catalog(ROOT)
        self.assertEqual("wwhd-alpha-1", revision)
        self.assertEqual("The Wind Waker HD", game)
        self.assertEqual(110, len(entries))
        self.assertEqual(70, sum(item["executable"] for item in entries))
        self.assertEqual(40, sum(not item["executable"] for item in entries))
        self.assertEqual(53, sum(item["experimental"] for item in entries))
        self.assertEqual({"bronze": 53, "silver": 47, "gold": 10},
                         {rarity: sum(item["rarity"] == rarity for item in entries)
                          for rarity in ("bronze", "silver", "gold")})
        self.assertEqual(len(entries), len(set(item["id"] for item in entries)))
        self.assertEqual(sorted(item["order"] for item in entries),
                         [item["order"] for item in entries])
        self.assertTrue(all(not e["experimental"] for e in entries if not e["executable"]))

    def test_known_threshold_rule(self):
        _, _, entries = catalog.load_catalog(ROOT)
        wallet = next(item for item in entries if item["id"] == "wallet-10")
        self.assertEqual("rupees", wallet["field"])
        self.assertEqual(10, wallet["threshold"])
        self.assertEqual(1, wallet["points"])
        self.assertTrue(wallet["executable"])
        self.assertEqual("bronze", wallet["rarity"])

    def test_planned_cannot_be_executable(self):
        _, _, entries = catalog.load_catalog(ROOT)
        final = next(item for item in entries if item["id"] == "story-ganondorf")
        self.assertFalse(final["executable"])
        self.assertEqual("", final["field"])
        self.assertEqual(0, final["threshold"])
        self.assertEqual("gold", final["rarity"])
        self.assertTrue(final["requirement"])

    def test_presentation_override_and_unicode(self):
        _, _, entries = catalog.load_catalog(ROOT)
        rule = next(item for item in entries if item["id"] == "story-hero-sword")
        self.assertEqual("Ottieni la prima spada dell'eroe.", rule["description"])
        self.assertEqual("Cuore d'avventura", next(item for item in entries if item["id"] == "hearts-6")["title"])
        self.assertEqual("Cuore d'avventura", catalog.ascii_fallback("Cuore d'avventura"))
        self.assertEqual("piu", catalog.ascii_fallback("più"))

    def test_generator_deterministic_and_complete(self):
        first = catalog.generate(ROOT)
        self.assertEqual(first, catalog.generate(ROOT))
        self.assertIn('const unsigned g_catalog_count', first)
        self.assertIn('g_catalog_source_sha256', first)
        self.assertEqual(110, first.count("    {"))

    def test_duplicate_and_missing_editorial_metadata_are_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            for name in catalog.SOURCE_NAMES:
                (root / name).write_bytes((ROOT / name).read_bytes())
            path = root / "achievement-presentation.xml"
            text = path.read_text(encoding="utf-8")
            path.write_text(text.replace('id="wallet-10"', 'id="wallet-100"', 1),
                            encoding="utf-8")
            with self.assertRaises(ValueError):
                catalog.load_catalog(root)

    def test_mismatched_revision_is_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            for name in catalog.SOURCE_NAMES:
                (root / name).write_bytes((ROOT / name).read_bytes())
            path = root / "achievements-planned.xml"
            path.write_text(path.read_text(encoding="utf-8").replace(
                'revision="wwhd-alpha-1"', 'revision="another-version"', 1),
                encoding="utf-8")
            with self.assertRaises(ValueError):
                catalog.load_catalog(root)


if __name__ == "__main__":
    unittest.main()
