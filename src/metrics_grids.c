#include "internal.h"

static uint64_t sf_grid_members_value(const SfGrid *grid) {
    uint64_t value = UINT64_C(0x3d52b1a9f0e46c87);
    size_t index;
    if (grid->members == NULL) {
        return sf_mix64(value ^ grid->id);
    }
    for (index = 0; index < grid->member_count; ++index) {
        value += sf_mix64(((uint64_t)grid->members[index] << 16) | index);
        value ^= value >> 17;
    }
    return sf_mix64(value);
}

static uint64_t sf_grid_links_value(const SfGrid *grid) {
    uint64_t value = UINT64_C(0x8d9243ee715ac0b6);
    size_t index;
    if (grid->node_links == NULL) {
        return sf_mix64(value ^ grid->label_id);
    }
    for (index = 0; index < grid->member_count; ++index) {
        const SfNode *node = grid->node_links[index];
        if (node != NULL) {
            value ^= sf_mix64(((uint64_t)node->id << 24) | node->kind);
        }
        value = (value << 11) | (value >> 53);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_grids(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xf12a987bc0364d15);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        value += sf_grid_members_value(grid);
        value ^= sf_grid_links_value(grid);
        value ^= sf_hash_text(grid->label, value + grid->id);
        value = sf_mix64(value + grid->flags + index);
    }
    return sf_mix64(value);
}
