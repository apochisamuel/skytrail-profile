#include "internal.h"

static size_t sf_max_string_id(const SfProfile *profile) {
    size_t index;
    size_t maximum = 0;
    for (index = 0; index < profile->string_count; ++index) {
        if (profile->strings[index].id > maximum) {
            maximum = profile->strings[index].id;
        }
    }
    return maximum;
}

static size_t sf_max_node_id(const SfProfile *profile) {
    size_t index;
    size_t maximum = 0;
    for (index = 0; index < profile->node_count; ++index) {
        if (profile->nodes[index].id > maximum) {
            maximum = profile->nodes[index].id;
        }
    }
    return maximum;
}

const char *sf_cache_name(const SfProfile *profile, uint16_t id) {
    if (profile == NULL || id >= profile->caches.names_cap) {
        return NULL;
    }
    return profile->caches.names_by_id[id];
}

SfNode *sf_cache_node(const SfProfile *profile, uint16_t id) {
    if (profile == NULL || id >= profile->caches.nodes_cap) {
        return NULL;
    }
    return profile->caches.nodes_by_id[id];
}

static SfStatus sf_allocate_cache_vectors(SfProfile *profile) {
    size_t string_cap = sf_max_string_id(profile) + 1;
    size_t node_cap = sf_max_node_id(profile) + 1;
    if (string_cap > 0) {
        profile->caches.names_by_id = (const char **)sf_allocate(string_cap, sizeof(const char *));
        if (profile->caches.names_by_id == NULL) {
            return SF_STATUS_MEMORY;
        }
        profile->caches.names_cap = string_cap;
    }
    if (node_cap > 0) {
        profile->caches.nodes_by_id = (SfNode **)sf_allocate(node_cap, sizeof(SfNode *));
        if (profile->caches.nodes_by_id == NULL) {
            return SF_STATUS_MEMORY;
        }
        profile->caches.nodes_cap = node_cap;
    }
    profile->caches.payloads_by_path = (const uint8_t **)sf_allocate(profile->path_count, sizeof(const uint8_t *));
    profile->caches.samples_by_path = (const uint16_t **)sf_allocate(profile->path_count, sizeof(const uint16_t *));
    profile->caches.members_by_grid = (const uint16_t **)sf_allocate(profile->grid_count, sizeof(const uint16_t *));
    profile->caches.values_by_annotation = (const char **)sf_allocate(profile->annotation_count, sizeof(const char *));
    profile->caches.tones_by_palette = (const uint16_t **)sf_allocate(profile->palette_count, sizeof(const uint16_t *));
    if ((profile->path_count > 0 && (profile->caches.payloads_by_path == NULL || profile->caches.samples_by_path == NULL)) ||
        (profile->grid_count > 0 && profile->caches.members_by_grid == NULL) ||
        (profile->annotation_count > 0 && profile->caches.values_by_annotation == NULL) ||
        (profile->palette_count > 0 && profile->caches.tones_by_palette == NULL)) {
        return SF_STATUS_MEMORY;
    }
    profile->caches.payloads_cap = profile->path_count;
    profile->caches.samples_cap = profile->path_count;
    profile->caches.members_cap = profile->grid_count;
    profile->caches.values_cap = profile->annotation_count;
    profile->caches.tones_cap = profile->palette_count;
    return SF_STATUS_OK;
}

SfStatus sf_build_caches(SfProfile *profile) {
    size_t index;
    SfStatus status;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    status = sf_allocate_cache_vectors(profile);
    if (status != SF_STATUS_OK) {
        sf_set_error(profile, status, "cache allocation");
        return status;
    }
    for (index = 0; index < profile->string_count; ++index) {
        SfString *item = &profile->strings[index];
        if (item->id < profile->caches.names_cap) {
            profile->caches.names_by_id[item->id] = item->text;
        }
        profile->caches.string_hash ^= sf_hash_text(item->text, (uint64_t)item->id + index);
    }
    for (index = 0; index < profile->node_count; ++index) {
        SfNode *item = &profile->nodes[index];
        if (item->id < profile->caches.nodes_cap) {
            profile->caches.nodes_by_id[item->id] = item;
        }
        profile->caches.node_hash ^= sf_hash_bytes(item->payload, item->payload_len, (uint64_t)item->id + index);
    }
    for (index = 0; index < profile->path_count; ++index) {
        profile->caches.samples_by_path[index] = profile->paths[index].samples;
        profile->paths[index].cache_sample_count = profile->paths[index].sample_count;
        profile->caches.topology_hash ^= sf_mix64((uint64_t)profile->paths[index].id << 32 | profile->paths[index].node_id);
    }
    for (index = 0; index < profile->grid_count; ++index) {
        profile->caches.members_by_grid[index] = profile->grids[index].members;
        profile->grids[index].cache_member_count = profile->grids[index].member_count;
        profile->caches.topology_hash ^= sf_mix64((uint64_t)profile->grids[index].id << 32 | profile->grids[index].label_id);
    }
    for (index = 0; index < profile->annotation_count; ++index) {
        profile->caches.values_by_annotation[index] = profile->annotations[index].value;
    }
    for (index = 0; index < profile->palette_count; ++index) {
        profile->caches.tones_by_palette[index] = profile->palettes[index].tones;
        profile->palettes[index].cache_tone_count = profile->palettes[index].tone_count;
    }
    return SF_STATUS_OK;
}
