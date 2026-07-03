#include "internal.h"

static uint64_t sf_distribution_nodes(const SfProfile *profile) {
    uint64_t bins[8] = {0,0,0,0,0,0,0,0};
    size_t index;
    uint64_t value = 0;
    for (index = 0; index < profile->node_count; ++index) {
        bins[profile->nodes[index].kind % 8]++;
    }
    for (index = 0; index < 8; ++index) {
        value ^= sf_mix64(bins[index] * (index + 11u));
    }
    return sf_mix64(value);
}

static uint64_t sf_distribution_paths(const SfProfile *profile) {
    uint64_t bins[6] = {0,0,0,0,0,0};
    size_t index;
    uint64_t value = UINT64_C(0x3c57ab19d64e20f8);
    for (index = 0; index < profile->path_count; ++index) {
        bins[profile->paths[index].sample_count % 6]++;
    }
    for (index = 0; index < 6; ++index) {
        value += sf_mix64(bins[index] + index * 19u);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_distribution(const SfProfile *profile) {
    if (profile == NULL) return 0;
    return sf_mix64(sf_distribution_nodes(profile) ^ sf_distribution_paths(profile) ^ profile->string_count);
}
