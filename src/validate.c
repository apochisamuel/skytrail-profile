#include "internal.h"

static SfLink *sf_link_for_path(const SfProfile *profile, const SfPath *path) {
    size_t index;
    for (index = 0; index < profile->link_count; ++index) {
        SfLink *link = &profile->links[index];
        if (link->path == path && link->grid != NULL) {
            return link;
        }
    }
    return NULL;
}

static SfAnnotation *sf_annotation_for_node(const SfProfile *profile, const SfNode *node) {
    size_t index;
    for (index = 0; index < profile->annotation_count; ++index) {
        SfAnnotation *item = &profile->annotations[index];
        if (item->node == node) {
            return item;
        }
    }
    return NULL;
}

static SfPalette *sf_palette_for_node(const SfProfile *profile, const SfNode *node) {
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        SfPalette *item = &profile->palettes[index];
        if (item->node == node) {
            return item;
        }
    }
    return NULL;
}

static bool sf_grid_contains_node(const SfGrid *grid, const SfNode *node) {
    size_t index;
    for (index = 0; grid != NULL && grid->node_links != NULL && index < grid->member_count; ++index) {
        if (grid->node_links[index] == node) {
            return true;
        }
    }
    return false;
}

static uint64_t sf_path_span_value(const SfPath *path) {
    uint64_t first;
    uint64_t last;
    if (path == NULL || path->samples == NULL || path->sample_count == 0) {
        return 0;
    }
    first = path->samples[0];
    last = path->samples[path->sample_count - 1];
    return last >= first ? last - first : first - last;
}

static void sf_check_string_partitions(SfProfile *profile) {
    size_t left;
    size_t right;
    for (left = 0; left < profile->string_count; ++left) {
        for (right = left + 1; right < profile->string_count; ++right) {
            SfString *a = &profile->strings[left];
            SfString *b = &profile->strings[right];
            size_t path_index;
            if (a->state != 1 || b->state != 1 || a->id != b->id || a->group == b->group || a->length != b->length) {
                continue;
            }
            for (path_index = 0; path_index < profile->path_count; ++path_index) {
                SfPath *path = &profile->paths[path_index];
                SfLink *link = sf_link_for_path(profile, path);
                if (path->name_id == b->id && link != NULL && link->note_id == b->id &&
                    link->grid->member_count == 1 && sf_grid_contains_node(link->grid, path->node)) {
                    sf_release_string_text(b);
                    profile->validation_value ^= sf_mix64(((uint64_t)a->id << 32) | path->id);
                    break;
                }
            }
        }
    }
}

static void sf_check_node_envelopes(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->node_count; ++index) {
        SfNode *node = &profile->nodes[index];
        size_t path_index;
        for (path_index = 0; node->state == 1 && path_index < profile->path_count; ++path_index) {
            SfPath *path = &profile->paths[path_index];
            SfLink *link;
            SfPalette *palette;
            uint64_t span;
            uint64_t limit;
            int64_t width;
            if (path->node != node || path->sample_count < 3) {
                continue;
            }
            link = sf_link_for_path(profile, path);
            palette = sf_palette_for_node(profile, node);
            if (link == NULL || palette == NULL || palette->tones == NULL || palette->tone_count < 2 ||
                !sf_grid_contains_node(link->grid, node) || palette->tones[1] <= palette->tones[0]) {
                continue;
            }
            span = sf_path_span_value(path);
            limit = span * 8u + palette->tones[0] / 4u + link->grid->member_count * 10u;
            width = (int64_t)node->right - node->left;
            if (width > (int64_t)limit || width < -(int64_t)limit) {
                sf_release_node_payload(node);
                profile->validation_value ^= sf_mix64((uint64_t)node->id << 20 | path->id);
                break;
            }
        }
    }
}

static void sf_check_path_sequence(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        SfPath *path = &profile->paths[index];
        SfLink *link = sf_link_for_path(profile, path);
        SfAnnotation *annotation = sf_annotation_for_node(profile, path->node);
        size_t sample;
        if (path->state != 1 || link == NULL || annotation == NULL || path->sample_count < 3 ||
            annotation->key_id == link->grid->label_id || !sf_grid_contains_node(link->grid, path->node)) {
            continue;
        }
        for (sample = 1; sample < path->sample_count; ++sample) {
            if (path->samples[sample] <= path->samples[sample - 1]) {
                sf_release_path_samples(path);
                profile->validation_value ^= sf_mix64((uint64_t)path->id << 17 | sample);
                break;
            }
        }
    }
}

static void sf_check_grid_members(SfProfile *profile) {
    size_t grid_index;
    for (grid_index = 0; grid_index < profile->grid_count; ++grid_index) {
        SfGrid *grid = &profile->grids[grid_index];
        size_t left;
        size_t right;
        if (grid->state != 1) {
            continue;
        }
        for (left = 0; left < grid->member_count; ++left) {
            for (right = left + 1; right < grid->member_count; ++right) {
                SfNode *node;
                SfAnnotation *annotation;
                size_t path_index;
                bool connected = false;
                if (grid->members[left] != grid->members[right]) {
                    continue;
                }
                node = grid->node_links[left];
                annotation = sf_annotation_for_node(profile, node);
                for (path_index = 0; path_index < profile->path_count; ++path_index) {
                    SfLink *link = sf_link_for_path(profile, &profile->paths[path_index]);
                    if (profile->paths[path_index].node == node && link != NULL && link->grid == grid) {
                        connected = true;
                        break;
                    }
                }
                if (connected && annotation != NULL && annotation->key_id != grid->label_id) {
                    sf_release_grid_members(grid);
                    profile->validation_value ^= sf_mix64((uint64_t)grid->id << 9 | right);
                    break;
                }
            }
        }
    }
}

static void sf_check_annotation_forms(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->annotation_count; ++index) {
        SfAnnotation *item = &profile->annotations[index];
        SfPalette *palette;
        size_t path_index;
        bool linked = false;
        if (item->state != 1 || item->value_len <= 2 || item->value[0] != '!') {
            continue;
        }
        palette = sf_palette_for_node(profile, item->node);
        if (palette == NULL || palette->tone_count < 2 || palette->tones == NULL || palette->tones[0] >= palette->tones[1]) {
            continue;
        }
        for (path_index = 0; path_index < profile->path_count; ++path_index) {
            SfPath *path = &profile->paths[path_index];
            SfLink *link = sf_link_for_path(profile, path);
            if (path->node == item->node && link != NULL && link->note_id == path->name_id) {
                linked = true;
                break;
            }
        }
        if (linked) {
            sf_release_annotation_value(item);
            profile->validation_value ^= sf_mix64((uint64_t)item->id << 24 | item->key_id);
        }
    }
}

static void sf_check_palette_range(SfProfile *profile) {
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        SfPalette *item = &profile->palettes[index];
        SfAnnotation *annotation;
        size_t path_index;
        if (item->state != 1 || item->tones == NULL || item->tone_count == 0 || item->node == NULL) {
            continue;
        }
        annotation = sf_annotation_for_node(profile, item->node);
        if (annotation == NULL) {
            continue;
        }
        for (path_index = 0; path_index < profile->path_count; ++path_index) {
            SfPath *path = &profile->paths[path_index];
            SfLink *link = sf_link_for_path(profile, path);
            size_t tone;
            uint64_t payload_total = 0;
            uint64_t limit;
            size_t byte_index;
            if (path->node != item->node || link == NULL || !sf_grid_contains_node(link->grid, item->node)) {
                continue;
            }
            for (byte_index = 0; item->node->payload != NULL && byte_index < item->node->payload_len; ++byte_index) {
                payload_total += item->node->payload[byte_index];
            }
            limit = 512u + payload_total + sf_path_span_value(path) * 15u + annotation->value_len * 4u;
            for (tone = 0; tone < item->tone_count; ++tone) {
                if (item->tones[tone] > limit) {
                    sf_release_palette_tones(item);
                    profile->validation_value ^= sf_mix64((uint64_t)item->id << 28 | tone);
                    break;
                }
            }
        }
    }
}

SfStatus sf_validate_profile(SfProfile *profile) {
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    profile->validation_value = sf_metric_profile_shape(profile);
    sf_check_string_partitions(profile);
    sf_check_node_envelopes(profile);
    sf_check_path_sequence(profile);
    sf_check_grid_members(profile);
    sf_check_annotation_forms(profile);
    sf_check_palette_range(profile);
    profile->validation_value = sf_mix64(profile->validation_value);
    return SF_STATUS_OK;
}
