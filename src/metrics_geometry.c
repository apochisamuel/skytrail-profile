#include "internal.h"

static uint64_t sf_geometry_corners(const SfNode *node) {
    int32_t corners[8] = {node->left, node->top, node->right, node->top, node->right, node->bottom, node->left, node->bottom};
    uint64_t value = UINT64_C(0x1a5b9e02c47df638);
    size_t index;
    for (index = 0; index < 8; ++index) {
        value ^= sf_mix64((uint32_t)corners[index] + index * UINT64_C(0x15));
    }
    return sf_mix64(value);
}

static uint64_t sf_geometry_center(const SfNode *node) {
    int64_t x = ((int64_t)node->left + node->right) / 2;
    int64_t y = ((int64_t)node->top + node->bottom) / 2;
    uint64_t value = ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
    return sf_mix64(value ^ node->id);
}

uint64_t sf_metric_geometry(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xf647c30b28d951ae);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        value += sf_geometry_corners(node);
        value ^= sf_geometry_center(node);
        value = sf_mix64(value + node->kind + index);
    }
    return sf_mix64(value);
}
