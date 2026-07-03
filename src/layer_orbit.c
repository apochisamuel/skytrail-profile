#include "internal.h"

static uint64_t sf_orbit_node_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x1234576fd16b038);
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t width = (int64_t)node->right - node->left;
        int64_t height = (int64_t)node->bottom - node->top;
        uint64_t shape;
        if (width < 0) {
            width = -width;
        }
        if (height < 0) {
            height = -height;
        }
        shape = (uint64_t)width * (uint64_t)(height + 1);
        value ^= sf_mix64(shape + node->id + index);
        value += sf_mix64(((uint64_t)node->kind << 40) | node->flags);
        value = (value << 9) | (value >> 55);
    }
    return sf_mix64(value);
}

static uint64_t sf_orbit_payload_scan(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t node_index;
    for (node_index = 0; node_index < profile->node_count; ++node_index) {
        const SfNode *node = &profile->nodes[node_index];
        size_t byte_index;
        for (byte_index = 0; node->payload != NULL && byte_index < node->payload_len; ++byte_index) {
            uint64_t byte = node->payload[byte_index];
            value += byte * (byte_index + 3u);
            value ^= sf_mix64(byte + node_index + (uint64_t)node->id * 17u);
            if ((byte_index & 3u) == 0) {
                value = (value << 5) | (value >> 59);
            }
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_orbit_path_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9e3779b9126ef0d6);
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        uint64_t start = 0;
        uint64_t end = 0;
        if (path->samples != NULL && path->sample_count > 0) {
            start = path->samples[0];
            end = path->samples[path->sample_count - 1];
        }
        value ^= sf_mix64((start << 32) | end);
        value += sf_mix64(((uint64_t)path->id << 24) | path->node_id);
        value ^= sf_hash_text(path->name, value + index);
    }
    return sf_mix64(value);
}

static uint64_t sf_orbit_weave(const SfProfile *profile, uint64_t left, uint64_t right) {
    uint64_t value = left ^ ((right << 1) | (right >> 63));
    size_t index;
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        value += sf_mix64((uint64_t)grid->id * (grid->member_count + 1u));
        value ^= sf_hash_text(grid->label, value + grid->label_id);
        if (grid->node_links != NULL && grid->member_count > 0) {
            const SfNode *node = grid->node_links[index % grid->member_count];
            if (node != NULL) {
                value ^= sf_mix64((uint64_t)node->id << 16 | node->kind);
            }
        }
    }
    return sf_mix64(value);
}

uint64_t sf_layer_orbit(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_orbit_node_scan(profile);
    b = sf_orbit_payload_scan(profile, a);
    c = sf_orbit_path_scan(profile);
    return sf_orbit_weave(profile, a ^ b, c);
}
