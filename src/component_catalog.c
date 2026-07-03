#include "internal.h"

static uint64_t sf_catalog_node_band(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t horizontal = (int64_t)node->right - node->left;
        int64_t vertical = (int64_t)node->bottom - node->top;
        uint64_t mask = ((uint64_t)(uint16_t)node->left << 48) |
                        ((uint64_t)(uint16_t)node->top << 32) |
                        ((uint64_t)(uint16_t)node->right << 16) |
                        (uint16_t)node->bottom;
        if (horizontal < 0) {
            horizontal = -horizontal;
        }
        if (vertical < 0) {
            vertical = -vertical;
        }
        value ^= sf_mix64(mask + (uint64_t)horizontal * (vertical + 1));
        value += sf_mix64((uint64_t)node->kind * 131u + node->flags + index);
        if (node->title != NULL) {
            value ^= sf_hash_text(node->title, value + node->title_id);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_catalog_path_band(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t path_index;
    for (path_index = 0; path_index < profile->path_count; ++path_index) {
        const SfPath *path = &profile->paths[path_index];
        size_t sample_index;
        uint64_t accumulator = 0;
        for (sample_index = 0; path->samples != NULL && sample_index < path->sample_count; ++sample_index) {
            accumulator += (uint64_t)path->samples[sample_index] * (sample_index + 1u);
            accumulator ^= sf_mix64(accumulator + path->id);
        }
        value += sf_mix64(accumulator ^ ((uint64_t)path->node_id << 24));
        value ^= sf_hash_text(path->name, value + path_index);
        if (path->node != NULL) {
            value += sf_mix64(path->node->id + path->node->payload_len);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_catalog_grid_band(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        uint64_t degree = grid->member_count;
        value ^= sf_mix64((degree << 32) | grid->id);
        value += sf_hash_text(grid->label, value + grid->label_id);
        if (grid->node_links != NULL && degree > 0) {
            const SfNode *first = grid->node_links[0];
            const SfNode *last = grid->node_links[degree - 1];
            if (first != NULL) {
                value ^= sf_mix64(first->id + first->kind);
            }
            if (last != NULL) {
                value += sf_mix64(last->id + last->flags);
            }
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_catalog_reduce(uint64_t a, uint64_t b, uint64_t c, size_t count) {
    uint64_t value = a + (b << 1);
    value ^= (c >> 1) | (c << 63);
    value += sf_mix64(count * UINT64_C(0x94d049bb133111eb));
    value ^= sf_mix64(a ^ b ^ c);
    value = (value << 21) | (value >> 43);
    return sf_mix64(value);
}

uint64_t sf_component_catalog(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_catalog_node_band(profile, UINT64_C(0x61c8864680b583eb));
    b = sf_catalog_path_band(profile, a ^ profile->checksum_value);
    c = sf_catalog_grid_band(profile, b ^ profile->validation_value);
    return sf_catalog_reduce(a, b, c, profile->node_count + profile->path_count + profile->grid_count);
}
