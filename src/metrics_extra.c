#include "internal.h"

static uint64_t sf_fold_sequence(uint64_t value, uint64_t item, size_t index) {
    value ^= sf_mix64(item + index * UINT64_C(0x9e3779b185ebca87));
    value = (value << 11) | (value >> 53);
    value += UINT64_C(0x94d049bb133111eb);
    return sf_mix64(value);
}

uint64_t sf_metric_signature(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x216f3a85bc49de70);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->directory_count; ++index) {
        const SfDirectoryEntry *entry = &profile->directory[index];
        value = sf_fold_sequence(value, ((uint64_t)entry->tag << 32) | entry->flags, index);
        value = sf_fold_sequence(value, ((uint64_t)entry->offset << 32) | entry->length, index + 7);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_clusters(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x617d3c8ae45b092f);
    size_t outer;
    size_t inner;
    if (profile == NULL) return 0;
    for (outer = 0; outer < profile->node_count; ++outer) {
        const SfNode *left = &profile->nodes[outer];
        for (inner = outer + 1; inner < profile->node_count; ++inner) {
            const SfNode *right = &profile->nodes[inner];
            int64_t dx = ((int64_t)left->left + left->right) - ((int64_t)right->left + right->right);
            int64_t dy = ((int64_t)left->top + left->bottom) - ((int64_t)right->top + right->bottom);
            uint64_t distance = (uint64_t)(dx < 0 ? -dx : dx) + (uint64_t)(dy < 0 ? -dy : dy);
            value ^= sf_mix64(distance + left->kind + right->kind);
        }
    }
    return sf_mix64(value);
}

uint64_t sf_metric_horizon(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xc2b2ae3d27d4eb4f);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        uint64_t first = 0;
        uint64_t last = 0;
        if (path->samples != NULL && path->sample_count > 0) {
            first = path->samples[0];
            last = path->samples[path->sample_count - 1];
        }
        value += sf_mix64((last >= first ? last - first : first - last) + path->id);
        value ^= sf_hash_text(path->name, value);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_coverage(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x165667b19e3779f9);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        value += sf_mix64(grid->member_count * (index + 1u));
        if (grid->node_links != NULL) {
            size_t item;
            for (item = 0; item < grid->member_count; ++item) {
                if (grid->node_links[item] != NULL) {
                    value ^= sf_mix64(grid->node_links[item]->id + item);
                }
            }
        }
    }
    return sf_mix64(value);
}

uint64_t sf_metric_coherence(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x27d4eb2f165667c5);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *item = &profile->annotations[index];
        value ^= sf_hash_text(item->key, value + item->id);
        value += sf_hash_text(item->value, value + item->node_id);
        if (item->node != NULL) {
            value ^= sf_mix64((uint64_t)item->node->id << 32 | item->node->kind);
        }
    }
    return sf_mix64(value);
}

uint64_t sf_metric_entropy(const SfProfile *profile) {
    uint64_t counts[16] = {0};
    uint64_t value = UINT64_C(0x3c79ac492ba7b653);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        size_t byte_index;
        for (byte_index = 0; node->payload != NULL && byte_index < node->payload_len; ++byte_index) {
            counts[node->payload[byte_index] & 15u]++;
        }
    }
    for (index = 0; index < 16; ++index) {
        value ^= sf_mix64(counts[index] * counts[index] + index * 73u);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_balance(const SfProfile *profile) {
    int64_t low = 0;
    int64_t high = 0;
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->palette_count; ++index) {
        const SfPalette *palette = &profile->palettes[index];
        size_t tone;
        for (tone = 0; palette->tones != NULL && tone < palette->tone_count; ++tone) {
            if ((tone & 1u) == 0) low += palette->tones[tone];
            else high += palette->tones[tone];
        }
    }
    return sf_mix64((uint64_t)(low > high ? low - high : high - low));
}

uint64_t sf_metric_span(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xa24baed4963ee407);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t width = (int64_t)node->right - node->left;
        int64_t height = (int64_t)node->bottom - node->top;
        value += sf_mix64((uint64_t)(width < 0 ? -width : width));
        value ^= sf_mix64((uint64_t)(height < 0 ? -height : height));
    }
    return sf_mix64(value);
}

uint64_t sf_metric_route_quality(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x6c8e9cf570932bd5);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        size_t sample;
        uint64_t changes = 0;
        for (sample = 1; path->samples != NULL && sample < path->sample_count; ++sample) {
            changes += path->samples[sample] - path->samples[sample - 1];
        }
        value ^= sf_mix64(changes + ((uint64_t)path->id << 18));
    }
    return sf_mix64(value);
}

uint64_t sf_metric_palette_range(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x5bf03635a1b7c9ed);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->palette_count; ++index) {
        const SfPalette *item = &profile->palettes[index];
        uint16_t minimum = UINT16_MAX;
        uint16_t maximum = 0;
        size_t tone;
        for (tone = 0; item->tones != NULL && tone < item->tone_count; ++tone) {
            if (item->tones[tone] < minimum) minimum = item->tones[tone];
            if (item->tones[tone] > maximum) maximum = item->tones[tone];
        }
        if (minimum != UINT16_MAX) value += sf_mix64(maximum - minimum + item->id);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_annotation_shape(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9ddfea08eb382d69);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *item = &profile->annotations[index];
        size_t separators = 0;
        size_t character;
        for (character = 0; item->value != NULL && character < item->value_len; ++character) {
            if (item->value[character] == ':' || item->value[character] == '=') separators++;
        }
        value ^= sf_mix64((uint64_t)separators << 32 | item->value_len);
        value += sf_hash_text(item->key, value + item->id);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_grid_shape(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x7d5c8a0b3e21f496);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        uint64_t pattern = 0;
        size_t member;
        for (member = 0; grid->members != NULL && member < grid->member_count; ++member) {
            pattern ^= sf_mix64((uint64_t)grid->members[member] << (member % 16u));
        }
        value += sf_mix64(pattern + grid->id + grid->label_id);
    }
    return sf_mix64(value);
}
