#include "internal.h"

static uint64_t sf_node_box_value(const SfNode *node) {
    int64_t width = (int64_t)node->right - node->left;
    int64_t height = (int64_t)node->bottom - node->top;
    uint64_t area;
    if (width < 0) width = -width;
    if (height < 0) height = -height;
    area = (uint64_t)width * (uint64_t)height;
    return sf_mix64(area ^ ((uint64_t)(uint16_t)node->left << 48) ^ ((uint64_t)(uint16_t)node->bottom << 16));
}

static uint64_t sf_node_payload_value(const SfNode *node, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    if (node->payload == NULL) {
        return sf_mix64(value ^ node->id);
    }
    for (index = 0; index < node->payload_len; ++index) {
        value += (uint64_t)node->payload[index] * (index + 1);
        value = (value << 7) | (value >> 57);
        value ^= sf_mix64(value + node->kind + node->flags);
    }
    return sf_mix64(value);
}

static uint64_t sf_node_axis_value(const SfNode *node) {
    int64_t x = (int64_t)node->left + node->right;
    int64_t y = (int64_t)node->top + node->bottom;
    uint64_t value = ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
    value ^= (uint64_t)node->kind << 56;
    value ^= (uint64_t)node->flags << 40;
    return sf_mix64(value);
}

uint64_t sf_metric_nodes(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xd68e213f5c17ab49);
    size_t index;
    if (profile == NULL) {
        return 0;
    }
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        value ^= sf_node_box_value(node);
        value += sf_node_payload_value(node, value ^ node->id);
        value ^= sf_node_axis_value(node);
        value = sf_mix64(value + index + node->id);
    }
    return sf_mix64(value ^ profile->node_count);
}
