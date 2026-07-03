#include "internal.h"

SfStatus sf_verify_checksum(SfProfile *profile) {
    SfReader reader;
    uint32_t stored;
    uint64_t computed;
    size_t index;
    SfStatus status;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    computed = sf_hash_bytes(profile->source, profile->source_size, UINT64_C(0x613f19bafe42d157));
    for (index = 0; index < profile->directory_count; ++index) {
        const SfDirectoryEntry *entry = &profile->directory[index];
        computed ^= sf_mix64(((uint64_t)entry->tag << 32) | entry->flags);
        computed += sf_mix64(entry->offset + entry->length + index);
    }
    profile->checksum_value = sf_mix64(computed);
    status = sf_section_reader(profile, SF_TAG_STAT, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u32(&reader, &stored) || reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "checksum section");
        return SF_STATUS_FORMAT;
    }
    profile->checksum_value ^= sf_mix64(stored);
    return SF_STATUS_OK;
}
