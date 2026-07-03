#include "internal.h"

static uint64_t sf_density_bounds(const SfProfile *profile) {
    int64_t total = 0;
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        const SfNode *node = &profile->nodes[index];
        int64_t width = (int64_t)node->right - node->left;
        int64_t height = (int64_t)node->bottom - node->top;
        if (width > 0 && height > 0) total += width * height;
    }
    return sf_mix64((uint64_t)total);
}

static uint64_t sf_density_routes(const SfProfile *profile) {
    uint64_t total = 0;
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        total += profile->paths[index].sample_count * (index + 1u);
    }
    return sf_mix64(total);
}

uint64_t sf_metric_density(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    if (profile == NULL) return 0;
    a = sf_density_bounds(profile);
    b = sf_density_routes(profile);
    return sf_mix64(a + b + (profile->grid_count << 12));
}
