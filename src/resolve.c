#include "internal.h"

static SfStatus sf_resolve_paths(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        SfPath *path = &profile->paths[index];
        path->node = sf_cache_node(profile, path->node_id);
        path->name = sf_cache_name(profile, path->name_id);
        if (path->node == NULL || path->name == NULL) {
            sf_set_error(profile, SF_STATUS_FORMAT, "path reference");
            return SF_STATUS_FORMAT;
        }
        path->payload_view = path->node->payload;
        path->payload_view_len = path->node->payload_len;
        if (index < profile->caches.payloads_cap) {
            profile->caches.payloads_by_path[index] = path->payload_view;
        }
    }
    return SF_STATUS_OK;
}

static SfStatus sf_resolve_grids(SfProfile *profile) {
    size_t grid_index;
    for (grid_index = 0; grid_index < profile->grid_count; ++grid_index) {
        SfGrid *grid = &profile->grids[grid_index];
        size_t member_index;
        grid->label = sf_cache_name(profile, grid->label_id);
        if (grid->label == NULL) {
            sf_set_error(profile, SF_STATUS_FORMAT, "grid label");
            return SF_STATUS_FORMAT;
        }
        grid->node_links = (SfNode **)sf_allocate(grid->member_count, sizeof(SfNode *));
        if (grid->node_links == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "grid links");
            return SF_STATUS_MEMORY;
        }
        for (member_index = 0; member_index < grid->member_count; ++member_index) {
            grid->node_links[member_index] = sf_cache_node(profile, grid->members[member_index]);
            if (grid->node_links[member_index] == NULL) {
                sf_set_error(profile, SF_STATUS_FORMAT, "grid node");
                return SF_STATUS_FORMAT;
            }
        }
    }
    return SF_STATUS_OK;
}

static SfStatus sf_resolve_annotations(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->annotation_count; ++index) {
        SfAnnotation *item = &profile->annotations[index];
        item->key = sf_cache_name(profile, item->key_id);
        item->node = sf_cache_node(profile, item->node_id);
        if (item->key == NULL || item->node == NULL) {
            sf_set_error(profile, SF_STATUS_FORMAT, "annotation reference");
            return SF_STATUS_FORMAT;
        }
    }
    return SF_STATUS_OK;
}

static SfStatus sf_resolve_palettes(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        SfPalette *item = &profile->palettes[index];
        item->name = sf_cache_name(profile, item->name_id);
        item->node = sf_cache_node(profile, item->node_id);
        if (item->name == NULL || item->node == NULL) {
            sf_set_error(profile, SF_STATUS_FORMAT, "palette reference");
            return SF_STATUS_FORMAT;
        }
    }
    return SF_STATUS_OK;
}

static SfStatus sf_resolve_links(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->link_count; ++index) {
        SfLink *item = &profile->links[index];
        item->grid = sf_find_grid_record(profile, item->source_grid);
        item->path = sf_find_path_record(profile, item->target_path);
        item->note = sf_cache_name(profile, item->note_id);
        if (item->grid == NULL || item->path == NULL || item->note == NULL) {
            sf_set_error(profile, SF_STATUS_FORMAT, "link reference");
            return SF_STATUS_FORMAT;
        }
    }
    return SF_STATUS_OK;
}

SfStatus sf_resolve_references(SfProfile *profile) {
    SfStatus status;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    status = sf_resolve_paths(profile);
    if (status != SF_STATUS_OK) {
        return status;
    }
    status = sf_resolve_grids(profile);
    if (status != SF_STATUS_OK) {
        return status;
    }
    status = sf_resolve_annotations(profile);
    if (status != SF_STATUS_OK) {
        return status;
    }
    status = sf_resolve_palettes(profile);
    if (status != SF_STATUS_OK) {
        return status;
    }
    return sf_resolve_links(profile);
}
