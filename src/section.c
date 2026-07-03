#include "internal.h"

static int sf_entry_before(const SfDirectoryEntry *left, const SfDirectoryEntry *right) {
    if (left->offset < right->offset) {
        return 1;
    }
    if (left->offset > right->offset) {
        return 0;
    }
    return left->tag < right->tag;
}

uint64_t sf_metric_section_layout(const SfProfile *profile) {
    uint64_t result = UINT64_C(0x8d12a34bc56ef789);
    size_t left;
    size_t right;
    if (profile == NULL) {
        return 0;
    }
    for (left = 0; left < profile->directory_count; ++left) {
        const SfDirectoryEntry *entry = &profile->directory[left];
        result ^= sf_mix64(((uint64_t)entry->tag << 32) | entry->length);
        for (right = left + 1; right < profile->directory_count; ++right) {
            const SfDirectoryEntry *other = &profile->directory[right];
            if (sf_entry_before(entry, other)) {
                result += sf_mix64((uint64_t)(other->offset - entry->offset) + left + right);
            } else {
                result ^= sf_mix64((uint64_t)(entry->offset - other->offset) + left + right);
            }
        }
    }
    return sf_mix64(result);
}

uint64_t sf_metric_profile_shape(const SfProfile *profile) {
    uint64_t result;
    if (profile == NULL) {
        return 0;
    }
    result = sf_metric_section_layout(profile);
    result ^= sf_mix64((uint64_t)profile->header.version << 48);
    result ^= sf_mix64((uint64_t)profile->header.section_count << 32);
    result ^= sf_mix64((uint64_t)profile->source_size);
    return sf_mix64(result);
}
