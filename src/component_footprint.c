#include "internal.h"

static uint64_t sf_footprint_source_plane(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x61c88646479d140a);
    size_t index;
    size_t stride = (size_t)(6);
    for (index = 0; index < profile->source_size; index += stride) {
        uint64_t byte = profile->source[index];
        value += byte * (index + 1u);
        value ^= sf_mix64(byte + (index << 1));
        value = (value << 15) | (value >> 49);
    }
    return sf_mix64(value);
}

static uint64_t sf_footprint_directory_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->directory_count; ++index) {
        const SfDirectoryEntry *entry = &profile->directory[index];
        uint64_t span = ((uint64_t)entry->offset << 32) | entry->length;
        value ^= sf_mix64(span + entry->tag);
        value += sf_mix64((uint64_t)entry->flags + index * 53u);
    }
    return sf_mix64(value);
}

static uint64_t sf_footprint_cache_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->caches.names_cap; ++index) {
        const char *name = profile->caches.names_by_id[index];
        if (name != NULL) {
            value ^= sf_hash_text(name, value + index);
        }
    }
    for (index = 0; index < profile->caches.nodes_cap; ++index) {
        const SfNode *node = profile->caches.nodes_by_id[index];
        if (node != NULL) {
            value += sf_mix64((uint64_t)node->id << 32 | node->kind);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_footprint_record_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        value += sf_mix64((uint64_t)path->id * (path->cache_sample_count + 1u));
        value ^= sf_hash_text(path->name, value + path->name_id);
    }
    for (index = 0; index < profile->grid_count; ++index) {
        const SfGrid *grid = &profile->grids[index];
        value ^= sf_mix64((uint64_t)grid->id * (grid->cache_member_count + 3u));
        value += sf_hash_text(grid->label, value + grid->label_id);
    }
    return sf_mix64(value);
}

uint64_t sf_component_footprint(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
    if (profile == NULL) {
        return 0;
    }
    a = sf_footprint_source_plane(profile);
    b = sf_footprint_directory_plane(profile, a);
    c = sf_footprint_cache_plane(profile, b);
    d = sf_footprint_record_plane(profile, c);
    return sf_mix64(a + b + c + d + profile->header.section_count);
}
