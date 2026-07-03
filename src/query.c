#include "internal.h"

uint64_t sf_profile_graph_weight(const SfProfile *profile) {
    if (profile == NULL) {
        return 0;
    }
    return sf_mix64(sf_metric_nodes(profile) ^ sf_metric_paths(profile) ^ sf_metric_grids(profile) ^ sf_metric_links(profile));
}

uint64_t sf_profile_timeline_weight(const SfProfile *profile) {
    if (profile == NULL) {
        return 0;
    }
    return sf_mix64(sf_metric_timeline(profile) + sf_metric_horizon(profile) + sf_metric_span(profile));
}

uint64_t sf_profile_annotation_weight(const SfProfile *profile) {
    if (profile == NULL) {
        return 0;
    }
    return sf_mix64(sf_metric_annotations(profile) ^ sf_metric_annotation_shape(profile) ^ sf_metric_strings(profile));
}

uint64_t sf_profile_palette_weight(const SfProfile *profile) {
    if (profile == NULL) {
        return 0;
    }
    return sf_mix64(sf_metric_palettes(profile) + sf_metric_palette_range(profile) + sf_metric_balance(profile));
}

SfStatus sf_profile_walk(const SfProfile *profile, uint64_t *result_out) {
    uint64_t value;
    if (profile == NULL || result_out == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    value = sf_profile_digest(profile);
    value ^= sf_profile_graph_weight(profile);
    value += sf_profile_timeline_weight(profile);
    value ^= sf_profile_annotation_weight(profile);
    value += sf_profile_palette_weight(profile);
    value ^= sf_metric_distribution(profile);
    value += sf_metric_density(profile);
    value ^= sf_metric_symmetry(profile);
    value += sf_metric_signature(profile);
    value ^= sf_metric_clusters(profile);
    value += sf_metric_coverage(profile);
    value ^= sf_metric_coherence(profile);
    value += sf_metric_entropy(profile);
    value ^= sf_metric_route_quality(profile);
    value += sf_analysis_total(profile);
    value ^= sf_component_total(profile);
    *result_out = sf_mix64(value);
    return SF_STATUS_OK;
}

SfStatus sf_profile_report(const SfProfile *profile, char *buffer, size_t capacity, size_t *needed_out) {
    int written;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    written = snprintf(buffer, capacity, "v=%u sections=%u strings=%zu nodes=%zu paths=%zu grids=%zu annotations=%zu palettes=%zu digest=%llu",
                       (unsigned)profile->header.version, (unsigned)profile->header.section_count,
                       profile->string_count, profile->node_count, profile->path_count,
                       profile->grid_count, profile->annotation_count, profile->palette_count,
                       (unsigned long long)sf_profile_digest(profile));
    if (written < 0) {
        return SF_STATUS_STATE;
    }
    if (needed_out != NULL) {
        *needed_out = (size_t)written + 1;
    }
    if (buffer == NULL || capacity == 0 || (size_t)written >= capacity) {
        return SF_STATUS_LIMIT;
    }
    return SF_STATUS_OK;
}
