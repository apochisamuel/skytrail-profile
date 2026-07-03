#include "internal.h"

SfStatus sf_parse_directory(SfProfile *profile) {
    SfReader reader;
    size_t index;
    if (profile == NULL) {
        return SF_STATUS_ARGUMENT;
    }
    if (!sf_reader_slice(profile->source, profile->source_size, profile->header.directory_offset,
                         (size_t)profile->header.section_count * 16u, &reader)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "directory span");
        return SF_STATUS_FORMAT;
    }
    profile->directory = (SfDirectoryEntry *)sf_allocate(profile->header.section_count, sizeof(SfDirectoryEntry));
    if (profile->directory == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "directory allocation");
        return SF_STATUS_MEMORY;
    }
    profile->directory_count = profile->header.section_count;
    for (index = 0; index < profile->directory_count; ++index) {
        uint8_t tag_bytes[4];
        SfDirectoryEntry *entry = &profile->directory[index];
        if (!sf_reader_remaining(&reader, 4)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "directory tag");
            return SF_STATUS_FORMAT;
        }
        memcpy(tag_bytes, reader.data + reader.pos, 4);
        reader.pos += 4;
        entry->tag = sf_tag_from_bytes(tag_bytes);
        if (!sf_reader_u32(&reader, &entry->offset) || !sf_reader_u32(&reader, &entry->length) || !sf_reader_u32(&reader, &entry->flags)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "directory entry");
            return SF_STATUS_FORMAT;
        }
        if (entry->length > SF_MAX_SECTION_BYTES || entry->offset > profile->source_size || entry->length > profile->source_size - entry->offset) {
            sf_set_error(profile, SF_STATUS_FORMAT, "section span");
            return SF_STATUS_FORMAT;
        }
    }
    return SF_STATUS_OK;
}

const SfDirectoryEntry *sf_find_section(const SfProfile *profile, uint32_t tag) {
    size_t index;
    if (profile == NULL) {
        return NULL;
    }
    for (index = 0; index < profile->directory_count; ++index) {
        if (profile->directory[index].tag == tag) {
            return &profile->directory[index];
        }
    }
    return NULL;
}

SfStatus sf_section_reader(const SfProfile *profile, uint32_t tag, SfReader *reader) {
    const SfDirectoryEntry *entry = sf_find_section(profile, tag);
    if (entry == NULL) {
        if (reader != NULL) {
            reader->data = NULL;
            reader->size = 0;
            reader->pos = 0;
        }
        return SF_STATUS_OK;
    }
    if (!sf_reader_slice(profile->source, profile->source_size, entry->offset, entry->length, reader)) {
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
