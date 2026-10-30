#ifndef NATIVE_WIIU_NUNITO_H
#define NATIVE_WIIU_NUNITO_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
    uint32_t codepoint;
    uint32_t offset;
    uint16_t width, height;
    int16_t left, top, advance;
} NunitoGlyph;
typedef struct {
    const NunitoGlyph *glyphs;
    unsigned count;
    unsigned size;
} NunitoFace;
enum {
    NUNITO_REGULAR_13,
    NUNITO_BOLD_16,
    NUNITO_EXTRABOLD_25,
    NUNITO_REGULAR_16,
    NUNITO_BOLD_21,
    NUNITO_EXTRABOLD_33
};
extern const NunitoFace nunito_faces[];
extern const uint8_t nunito_pixels[];
extern const size_t nunito_pixels_size;
#endif
