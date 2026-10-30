#!/usr/bin/env python3
"""Generate the Wii U catalog from the Windows project's canonical XML files.

No achievements are hand-maintained in this port. All game-specific content,
rarities, ordering and experimental statuses originate upstream.

Example:
    python3 wiiu/tools/generate_catalog.py --root . --out /tmp/catalog_generated.c
"""
import argparse
import hashlib
import json
import pathlib
import unicodedata
import xml.etree.ElementTree as ET

SOURCE_NAMES = (
    "achievements.xml",
    "achievements-story.xml",
    "achievements-planned.xml",
    "achievement-presentation.xml",
)
VALID_RARITIES = {"bronze", "silver", "gold", "platinum"}
DEFAULT_CATEGORY = "Crescita"
DEFAULT_CHAPTER = "Extra"
DEFAULT_ORDER = 10000


def _int(value, name, minimum=0):
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError("Invalid integer for {}: {!r}".format(name, value)) from exc
    if parsed < minimum:
        raise ValueError("{} must be >= {}".format(name, minimum))
    return parsed


def _required(element, attr):
    value = element.get(attr)
    if value is None or not value.strip():
        raise ValueError("Missing {} on {}".format(attr, element.tag))
    return value


def _load_xml(path, expected_tag):
    tree = ET.parse(str(path))
    root = tree.getroot()
    if root.tag != expected_tag:
        raise ValueError("{}: unexpected root {}".format(path.name, root.tag))
    if not root.get("revision"):
        raise ValueError("{}: missing set revision".format(path.name))
    return root


def load_catalog(repo):
    """Return revision, game, entries sorted like the original order metadata.

    Fail closed on duplicate IDs, incomplete editorial metadata, unknown tags,
    invalid values, or mixed revisions. Never turn planned achievements into
    executable rules.
    """
    repo = pathlib.Path(repo)
    files = [repo / name for name in SOURCE_NAMES]
    roots = [_load_xml(path, "achievement-presentation" if i == 3 else "achievement-set")
             for i, path in enumerate(files)]
    revisions = {root.get("revision") for root in roots}
    if len(revisions) != 1:
        raise ValueError("Mixed achievement-set revisions")
    revision = roots[0].get("revision")
    games = {root.get("game") for root in roots[:3]}
    if len(games) != 1 or not next(iter(games)):
        raise ValueError("Mixed or missing game names")
    game = next(iter(games))

    entries = []
    by_id = {}
    for root in roots[:3]:
        for node in root:
            if node.tag not in ("achievement", "planned") or len(node):
                raise ValueError("Unknown or nested achievement element")
            pending = node.tag == "planned"
            if (root is roots[2]) != pending:
                raise ValueError("An executable rule or planned rule is in the wrong file")

            id_ = _required(node, "id")
            if id_ in by_id:
                raise ValueError("Duplicate achievement ID: " + id_)
            title = _required(node, "title")
            description = _required(node, "description")
            points = _int(node.get("points"), "points", 1)
            order = _int(node.get("order", str(DEFAULT_ORDER)), "order")
            category = node.get("category", DEFAULT_CATEGORY)
            chapter = node.get("chapter", DEFAULT_CHAPTER)
            if not category or not chapter:
                raise ValueError("Empty category/chapter on " + id_)
            field = ""
            threshold = 0
            requirement = ""
            experimental = False
            if pending:
                requirement = _required(node, "requirement")
                if node.get("field") or node.get("threshold"):
                    raise ValueError("Planned rule contains execution fields: " + id_)
                allowed = {"id", "title", "description", "category", "requirement",
                           "points", "chapter", "order"}
            else:
                field = _required(node, "field")
                threshold = _int(node.get("threshold"), "threshold", 1)
                if node.get("experimental", "false") not in ("true", "false"):
                    raise ValueError("Invalid experimental flag on " + id_)
                experimental = node.get("experimental") == "true"
                allowed = {"id", "title", "description", "category", "field",
                           "threshold", "points", "chapter", "order", "experimental"}
            if set(node.attrib) - allowed:
                raise ValueError("Unknown XML attributes on " + id_)
            item = {
                "id": id_, "title": title, "description": description,
                "category": category, "chapter": chapter, "field": field,
                "threshold": threshold, "points": points, "order": order,
                "requirement": requirement, "rarity": None,
                "executable": not pending, "experimental": experimental,
            }
            by_id[id_] = item
            entries.append(item)

    seen = set()
    for node in roots[3]:
        if node.tag != "entry" or len(node):
            raise ValueError("Unexpected presentation element")
        id_ = _required(node, "id")
        if id_ not in by_id or id_ in seen:
            raise ValueError("Unknown or duplicate presentation ID: " + id_)
        seen.add(id_)
        rarity = _required(node, "rarity")
        if rarity not in VALID_RARITIES:
            raise ValueError("Invalid rarity on " + id_)
        if rarity == "platinum" and (id_ != "completion-platinum" or not by_id[id_]["requirement"]):
            raise ValueError("Platinum reserved for completion-platinum")
        item = by_id[id_]
        item["rarity"] = rarity
        allowed = {"id", "rarity", "chapter", "order", "title", "description"}
        if set(node.attrib) - allowed:
            raise ValueError("Unexpected presentation metadata on " + id_)
        for field in ("chapter", "title", "description"):
            if field in node.attrib:
                item[field] = _required(node, field)
        if "order" in node.attrib:
            item["order"] = _int(node.get("order"), "order")
    if seen != set(by_id):
        raise ValueError("Presentation metadata missing for: " + ", ".join(sorted(set(by_id) - seen)))

    # Python sort is stable, matching the source file order for equal Order.
    entries.sort(key=lambda item: item["order"])
    return revision, game, entries


def ascii_fallback(value):
    """Temporary monospace-renderer fallback; canonical UTF-8 is preserved."""
    normalized = unicodedata.normalize("NFKD", value)
    return "".join(c for c in normalized if not unicodedata.combining(c)).encode(
        "ascii", "replace").decode("ascii")


def c_literal(value):
    # JSON's ASCII escapes for characters > 127 are not UTF-8 C string literals,
    # so keep actual UTF-8 source bytes. Quote and control escaping is identical.
    return json.dumps(str(value), ensure_ascii=False)


def generate(repo):
    revision, game, entries = load_catalog(repo)
    digest = hashlib.sha256()
    for name in SOURCE_NAMES:
        data = (pathlib.Path(repo) / name).read_bytes()
        digest.update(name.encode("utf-8") + b"\0" + data + b"\0")

    out = [
        "/* AUTO-GENERATED FROM THE FOUR CANONICAL XML FILES. DO NOT EDIT. */",
        '#include "catalog.h"',
        "",
        "const char *const g_catalog_revision = " + c_literal(revision) + ";",
        "const char *const g_catalog_game = " + c_literal(game) + ";",
        "const char *const g_catalog_source_sha256 = " + c_literal(digest.hexdigest()) + ";",
        "const AchievementEntry g_catalog[] = {",
    ]
    for entry in entries:
        parts = [
            c_literal(entry[field]) for field in
            ("id", "title", "description", "category", "chapter",
             "rarity", "field", "requirement")
        ]
        parts.insert(2, c_literal(ascii_fallback(entry["title"])))
        parts.extend(str(entry[field]) for field in ("threshold", "points", "order"))
        parts.extend("1" if entry[field] else "0" for field in ("executable", "experimental"))
        out.append("    {" + ", ".join(parts) + "},")
    out.extend([
        "};",
        "const unsigned g_catalog_count = sizeof(g_catalog) / sizeof(g_catalog[0]);",
        "",
    ])
    return "\n".join(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[2])
    parser.add_argument("--out", type=pathlib.Path, required=True)
    args = parser.parse_args()
    output = generate(args.root)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(output, encoding="utf-8", newline="\n")
    _, _, entries = load_catalog(args.root)
    print("Generated {} achievements ({} executable, {} planned) in {}".format(
        len(entries), sum(a["executable"] for a in entries),
        sum(not a["executable"] for a in entries), args.out))


if __name__ == "__main__":
    main()
