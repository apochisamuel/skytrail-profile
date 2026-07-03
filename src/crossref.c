#include "internal.h"

static uint64_t sf_crossref_names(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x4bcdec7a7f051d39);
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        if (path->name != NULL) {
            value ^= sf_hash_text(path->name, value + path->id + index);
        }
    }
    for (index = 0; index < profile->link_count; ++index) {
        const SfLink *link = &profile->links[index];
        if (link->note != NULL) {
            value += sf_hash_text(link->note, value ^ link->mode);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_crossref_payloads(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x73a54f2b9c1de807);
    size_t index;
    for (index = 0; index < profile->path_count; ++index) {
        const SfPath *path = &profile->paths[index];
        if (path->payload_view != NULL && path->payload_view_len > 0) {
            size_t pick = (index + path->id) % path->payload_view_len;
            value ^= sf_mix64((uint64_t)path->payload_view[pick] + ((uint64_t)path->payload_view_len << 16));
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_crossref_samples(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x0e749c2f653ab981);
    size_t index;
    for (index = 0; index < profile->caches.samples_cap; ++index) {
        const uint16_t *samples = profile->caches.samples_by_path[index];
        size_t count = profile->paths[index].cache_sample_count;
        if (samples != NULL && count > 0) {
            value += sf_mix64((uint64_t)samples[count - 1] << 19 | samples[0]);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_crossref_members(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x2ab7e5d1028c94f6);
    size_t index;
    for (index = 0; index < profile->caches.members_cap; ++index) {
        const uint16_t *members = profile->caches.members_by_grid[index];
        size_t count = profile->grids[index].cache_member_count;
        if (members != NULL && count > 0) {
            value ^= sf_mix64(((uint64_t)members[0] << 24) | members[count - 1]);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_crossref_annotations(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x51e82d3bfa0c4697);
    size_t index;
    for (index = 0; index < profile->caches.values_cap; ++index) {
        const char *text = profile->caches.values_by_annotation[index];
        if (text != NULL) {
            value ^= sf_hash_text(text, value + index);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_crossref_palettes(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9341bc6afde02875);
    size_t index;
    for (index = 0; index < profile->caches.tones_cap; ++index) {
        const uint16_t *tones = profile->caches.tones_by_palette[index];
        size_t count = profile->palettes[index].cache_tone_count;
        if (tones != NULL && count > 0) {
            value += sf_mix64((uint64_t)tones[0] * 257u + tones[count - 1]);
        }
    }
    return sf_mix64(value);
}

SfStatus sf_crossref_run(SfProfile *profile) {
    uint64_t value;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    value = sf_crossref_names(profile);
    value ^= sf_crossref_payloads(profile);
    value += sf_crossref_samples(profile);
    value ^= sf_crossref_members(profile);
    value += sf_crossref_annotations(profile);
    value ^= sf_crossref_palettes(profile);
    profile->sequence_value = sf_mix64(value);
    return SF_STATUS_OK;
}
