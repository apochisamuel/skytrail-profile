#include "internal.h"

static uint64_t sf_vector_annotation_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x123457b509b040f);
    size_t index;
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *item = &profile->annotations[index];
        uint64_t text = sf_hash_text(item->value, value + item->id);
        uint64_t key = sf_hash_text(item->key, text ^ item->key_id);
        value += sf_mix64(text + key + ((uint64_t)item->node_id << 12));
        if (item->node != NULL) {
            value ^= sf_mix64((uint64_t)item->node->id << 28 | item->node->flags);
        }
        value = (value << 13) | (value >> 51);
    }
    return sf_mix64(value);
}

static uint64_t sf_vector_palette_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9e3779b9f4f99b75);
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        const SfPalette *palette = &profile->palettes[index];
        size_t tone_index;
        uint64_t gradient = 0;
        for (tone_index = 0; palette->tones != NULL && tone_index < palette->tone_count; ++tone_index) {
            uint64_t tone = palette->tones[tone_index];
            gradient += tone * (tone_index + 7u);
            gradient ^= sf_mix64(tone + gradient);
        }
        value ^= sf_mix64(gradient + ((uint64_t)palette->id << 20));
        value += sf_hash_text(palette->name, value ^ palette->node_id);
    }
    return sf_mix64(value);
}

static uint64_t sf_vector_directory_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x7f4a7c159e3779b9);
    size_t index;
    for (index = 0; index < profile->directory_count; ++index) {
        const SfDirectoryEntry *entry = &profile->directory[index];
        uint64_t span = ((uint64_t)entry->offset << 32) | entry->length;
        value ^= sf_mix64(span ^ entry->tag);
        value += sf_mix64(entry->flags + index * 29u);
    }
    return sf_mix64(value);
}

static uint64_t sf_vector_compose(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t value = a ^ b;
    value += (c << 1) | (c >> 63);
    value ^= sf_mix64(a + b + c);
    value += UINT64_C(0x165667b19e3779f9);
    return sf_mix64(value);
}

uint64_t sf_layer_vector(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_vector_annotation_scan(profile);
    b = sf_vector_palette_scan(profile);
    c = sf_vector_directory_scan(profile);
    return sf_vector_compose(a, b, c);
}
