#include "internal.h"

static uint64_t sf_palette_wave(const SfPalette *item) {
    uint64_t value = UINT64_C(0x6f2d91ac3b58e407);
    size_t index;
    if (item->tones == NULL || item->tone_count == 0) {
        return sf_mix64(value ^ item->id);
    }
    for (index = 0; index < item->tone_count; ++index) {
        uint64_t tone = item->tones[index];
        value += tone * tone + index * 31u;
        value ^= (tone << (index % 17));
        value = sf_mix64(value);
    }
    return value;
}

static uint64_t sf_palette_binding(const SfPalette *item) {
    uint64_t value = ((uint64_t)item->id << 32) | item->name_id;
    value ^= (uint64_t)item->node_id << 16;
    value += sf_hash_text(item->name, value);
    if (item->node != NULL) {
        value ^= sf_mix64((uint64_t)item->node->kind << 48 | item->node->id);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_palettes(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x8ef5a1c934db6702);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->palette_count; ++index) {
        value ^= sf_palette_wave(&profile->palettes[index]);
        value += sf_palette_binding(&profile->palettes[index]);
        value = sf_mix64(value + index);
    }
    return sf_mix64(value);
}
