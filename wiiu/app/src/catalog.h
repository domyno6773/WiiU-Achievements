#ifndef WIIU_ACHIEVEMENTS_CATALOG_H
#define WIIU_ACHIEVEMENTS_CATALOG_H

/* Generated catalog is display-only. It cannot grant an achievement.
 * The canonical XML and Windows rule engine remain authoritative. */
typedef struct AchievementEntry {
    const char *id;
    const char *title;             /* Original UTF-8 title */
    const char *ascii_title;       /* Temporary monospace renderer fallback */
    const char *description;       /* Original UTF-8 description */
    const char *category;
    const char *chapter;
    const char *rarity;
    const char *field;
    const char *requirement;
    int threshold;
    int points;
    int order;
    int executable;
    int experimental;
} AchievementEntry;

extern const AchievementEntry g_catalog[];
extern const unsigned g_catalog_count;
extern const char *const g_catalog_revision;
extern const char *const g_catalog_game;
extern const char *const g_catalog_source_sha256;

#endif
