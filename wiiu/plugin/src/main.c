/*
 * Wii U Achievements — Aroma / WUPS plugin bootstrap.
 *
 * Intentionally inert in milestone 0. No memory reads, game modifications,
 * save writes, overlay calls, or notifications. The hooks below only prove
 * that WUPS can call our lifecycle entry points in a later hardware test.
 */
#include <wups.h>

WUPS_PLUGIN_NAME("Wii U Achievements");
WUPS_PLUGIN_DESCRIPTION("Native achievements companion (prototype, inactive)");
WUPS_PLUGIN_VERSION("v0.0.1");
WUPS_PLUGIN_AUTHOR("Wii U Achievements contributors");
WUPS_PLUGIN_LICENSE("No public license");

INITIALIZE_PLUGIN()
{
    /* Reserved for safe initialization when the integration is ready. */
}

DEINITIALIZE_PLUGIN()
{
    /* Reserved for safe teardown. */
}

ON_APPLICATION_START()
{
    /* Do not monitor an unidentified title. */
}

ON_APPLICATION_ENDS()
{
    /* No resources have been allocated. */
}
