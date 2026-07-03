#include "internal.h"

void sf_set_error(SfProfile *profile, SfStatus status, const char *message) {
    if (profile == NULL) {
        return;
    }
    profile->status = status;
    if (message == NULL) {
        profile->error[0] = '\0';
        return;
    }
    snprintf(profile->error, sizeof(profile->error), "%s", message);
}

void sf_clear_error(SfProfile *profile) {
    if (profile == NULL) {
        return;
    }
    profile->status = SF_STATUS_OK;
    profile->error[0] = '\0';
}

const char *sf_status_string(SfStatus status) {
    switch (status) {
        case SF_STATUS_OK:
            return "ok";
        case SF_STATUS_ARGUMENT:
            return "argument";
        case SF_STATUS_FORMAT:
            return "format";
        case SF_STATUS_MEMORY:
            return "memory";
        case SF_STATUS_LIMIT:
            return "limit";
        case SF_STATUS_STATE:
            return "state";
        default:
            return "unknown";
    }
}

static void sf_release_profile_arrays(SfProfile *profile) {
    size_t index;
    if (profile == NULL) {
        return;
    }
    for (index = 0; index < profile->string_count; ++index) {
        sf_release_string_text(&profile->strings[index]);
    }
    for (index = 0; index < profile->node_count; ++index) {
        sf_release_node_payload(&profile->nodes[index]);
    }
    for (index = 0; index < profile->path_count; ++index) {
        sf_release_path_samples(&profile->paths[index]);
    }
    for (index = 0; index < profile->grid_count; ++index) {
        sf_release_grid_members(&profile->grids[index]);
        free(profile->grids[index].node_links);
        profile->grids[index].node_links = NULL;
    }
    for (index = 0; index < profile->annotation_count; ++index) {
        sf_release_annotation_value(&profile->annotations[index]);
    }
    for (index = 0; index < profile->palette_count; ++index) {
        sf_release_palette_tones(&profile->palettes[index]);
    }
    free(profile->directory);
    free(profile->strings);
    free(profile->nodes);
    free(profile->paths);
    free(profile->grids);
    free(profile->annotations);
    free(profile->palettes);
    free(profile->links);
    free(profile->caches.names_by_id);
    free(profile->caches.nodes_by_id);
    free(profile->caches.payloads_by_path);
    free(profile->caches.samples_by_path);
    free(profile->caches.members_by_grid);
    free(profile->caches.values_by_annotation);
    free(profile->caches.tones_by_palette);
}

void sf_profile_close(SfProfile *profile) {
    if (profile == NULL) {
        return;
    }
    sf_release_profile_arrays(profile);
    free(profile);
}
