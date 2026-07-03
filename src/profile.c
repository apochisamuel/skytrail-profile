#include "internal.h"

static SfStatus sf_run_pipeline(SfProfile *profile) {
    SfStatus status;
    status = sf_parse_header(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_directory(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_strings(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_nodes(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_paths(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_grids(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_annotations(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_palettes(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_parse_links(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_build_caches(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_resolve_references(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_verify_checksum(profile);
    if (status != SF_STATUS_OK) return status;
    status = sf_validate_profile(profile);
    if (status != SF_STATUS_OK) return status;
    return sf_crossref_run(profile);
}

SfProfile *sf_profile_open_memory(const uint8_t *data, size_t size, SfStatus *status_out) {
    SfProfile *profile;
    SfStatus status;
    if (status_out != NULL) {
        *status_out = SF_STATUS_ARGUMENT;
    }
    if (data == NULL || size < 16 || size > 16u * 1024u * 1024u) {
        return NULL;
    }
    profile = (SfProfile *)sf_allocate(1, sizeof(SfProfile));
    if (profile == NULL) {
        if (status_out != NULL) {
            *status_out = SF_STATUS_MEMORY;
        }
        return NULL;
    }
    profile->source = data;
    profile->source_size = size;
    sf_clear_error(profile);
    status = sf_run_pipeline(profile);
    if (status != SF_STATUS_OK) {
        if (status_out != NULL) {
            *status_out = status;
        }
        sf_profile_close(profile);
        return NULL;
    }
    if (status_out != NULL) {
        *status_out = SF_STATUS_OK;
    }
    return profile;
}

SfStatus sf_profile_info(const SfProfile *profile, SfProfileInfo *info_out) {
    if (profile == NULL || info_out == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    info_out->version = profile->header.version;
    info_out->section_count = profile->header.section_count;
    info_out->string_count = profile->string_count;
    info_out->node_count = profile->node_count;
    info_out->path_count = profile->path_count;
    info_out->grid_count = profile->grid_count;
    info_out->annotation_count = profile->annotation_count;
    info_out->palette_count = profile->palette_count;
    return SF_STATUS_OK;
}

size_t sf_profile_node_count(const SfProfile *profile) {
    return profile == NULL ? 0 : profile->node_count;
}

size_t sf_profile_path_count(const SfProfile *profile) {
    return profile == NULL ? 0 : profile->path_count;
}

size_t sf_profile_string_count(const SfProfile *profile) {
    return profile == NULL ? 0 : profile->string_count;
}

const char *sf_profile_lookup_name(const SfProfile *profile, uint16_t id) {
    return sf_cache_name(profile, id);
}

uint64_t sf_profile_digest(const SfProfile *profile) {
    if (profile == NULL) {
        return 0;
    }
    return sf_mix64(profile->checksum_value ^ profile->validation_value ^ profile->sequence_value);
}
