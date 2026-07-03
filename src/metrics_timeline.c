#include "internal.h"

static uint64_t sf_timeline_path(const SfPath *path) {
    uint64_t value = UINT64_C(0x9f35d2e8a17bc640);
    size_t index;
    uint64_t last = 0;
    if (path->samples == NULL) return sf_mix64(value ^ path->id);
    for (index = 0; index < path->sample_count; ++index) {
        uint64_t now = path->samples[index];
        value += sf_mix64(now + last * 3u + index);
        last = now;
    }
    return sf_mix64(value);
}

uint64_t sf_metric_timeline(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x23b4f651d8a09c7e);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->path_count; ++index) {
        value ^= sf_timeline_path(&profile->paths[index]);
        value += sf_mix64(profile->paths[index].cache_sample_count + index);
    }
    return sf_mix64(value);
}
