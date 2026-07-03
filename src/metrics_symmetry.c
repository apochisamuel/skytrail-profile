#include "internal.h"

static uint64_t sf_symmetry_nodes(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x6cae291f73bd0458);
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t horizontal = (int64_t)node->left + node->right;
        int64_t vertical = (int64_t)node->top + node->bottom;
        uint64_t difference = (uint64_t)(horizontal > vertical ? horizontal - vertical : vertical - horizontal);
        value ^= sf_mix64(difference + index * 7u);
    }
    return sf_mix64(value);
}

static uint64_t sf_symmetry_palette(const SfProfile *profile) {
    uint64_t value = 0;
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        const SfPalette *palette = &profile->palettes[index];
        if (palette->tones != NULL && palette->tone_count > 1) {
            uint16_t first = palette->tones[0];
            uint16_t last = palette->tones[palette->tone_count - 1];
            value += sf_mix64(first > last ? first - last : last - first);
        }
    }
    return sf_mix64(value);
}

uint64_t sf_metric_symmetry(const SfProfile *profile) {
    if (profile == NULL) return 0;
    return sf_mix64(sf_symmetry_nodes(profile) ^ sf_symmetry_palette(profile));
}
