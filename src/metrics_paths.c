#include "internal.h"

static uint64_t sf_path_delta_value(const SfPath *path) {
    uint64_t value = UINT64_C(0x4e31c0d7ab7528f9);
    size_t index;
    if (path->samples == NULL || path->sample_count == 0) {
        return sf_mix64(value ^ path->id);
    }
    value ^= path->samples[0];
    for (index = 1; index < path->sample_count; ++index) {
        int64_t delta = (int64_t)path->samples[index] - path->samples[index - 1];
        uint64_t magnitude = (uint64_t)(delta < 0 ? -delta : delta);
        value += sf_mix64(magnitude + ((uint64_t)index << 28));
        value ^= (uint64_t)(delta < 0) << (index % 31);
    }
    return sf_mix64(value);
}

static uint64_t sf_path_binding_value(const SfPath *path) {
    uint64_t value = ((uint64_t)path->id << 48) | ((uint64_t)path->node_id << 24) | path->name_id;
    value ^= (uint64_t)path->flags << 13;
    if (path->node != NULL) {
        value += sf_mix64((uint64_t)path->node->id << 32 | path->node->kind);
    }
    return sf_mix64(value);
}

static uint64_t sf_path_payload_value(const SfPath *path) {
    uint64_t value = UINT64_C(0x9a3415c8027ebd6f);
    size_t index;
    if (path->payload_view == NULL) {
        return sf_mix64(value ^ path->payload_view_len);
    }
    for (index = 0; index < path->payload_view_len; ++index) {
        value ^= ((uint64_t)path->payload_view[index] + 1u) * (index + 17u);
        value = sf_mix64(value);
    }
    return value;
}

uint64_t sf_metric_paths(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x0ab1d5ef37c49286);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        value += sf_path_delta_value(path);
        value ^= sf_path_binding_value(path);
        value += sf_path_payload_value(path);
        value = sf_mix64(value + index);
    }
    return sf_mix64(value ^ profile->path_count);
}
