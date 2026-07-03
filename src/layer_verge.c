#include "internal.h"

static uint64_t sf_verge_route_window(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x123458b623d6009);
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        size_t sample_index;
        uint64_t minimum = UINT64_MAX;
        uint64_t maximum = 0;
        for (sample_index = 0; path->samples != NULL && sample_index < path->sample_count; ++sample_index) {
            uint64_t point = path->samples[sample_index];
            if (point < minimum) {
                minimum = point;
            }
            if (point > maximum) {
                maximum = point;
            }
        }
        if (minimum != UINT64_MAX) {
            value ^= sf_mix64((maximum - minimum) + ((uint64_t)path->id << 18));
        }
        value += sf_hash_text(path->name, value + path->node_id);
    }
    return sf_mix64(value);
}

static uint64_t sf_verge_node_window(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9e3779b9827c6f4b);
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t cx = (int64_t)node->left + node->right;
        int64_t cy = (int64_t)node->top + node->bottom;
        value += sf_mix64(((uint64_t)(uint32_t)cx << 32) | (uint32_t)cy);
        value ^= sf_hash_text(node->title, value + node->title_id);
        if (node->payload != NULL && node->payload_len > 0) {
            value += sf_mix64(node->payload[node->payload_len / 2] + index);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_verge_membership_window(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x2545f4914f6cdd1d);
    size_t grid_index;
    for (grid_index = 0; grid_index < profile->grid_count; ++grid_index) {
        const SfGrid *grid = &profile->grids[grid_index];
        size_t member_index;
        for (member_index = 0; grid->members != NULL && member_index < grid->member_count; ++member_index) {
            value ^= sf_mix64((uint64_t)grid->members[member_index] * (member_index + 9u));
            value += sf_mix64(grid->id + member_index);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_verge_project(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    uint64_t value = sf_mix64(a + b);
    value ^= sf_mix64(c + d);
    value += (a >> 3) ^ (b << 5);
    value ^= (c >> 11) ^ (d << 7);
    return sf_mix64(value);
}

uint64_t sf_layer_verge(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_verge_route_window(profile);
    b = sf_verge_node_window(profile);
    c = sf_verge_membership_window(profile);
    return sf_verge_project(a, b, c, profile->checksum_value);
}
