#include "internal.h"

SfStatus sf_parse_header(SfProfile *profile) {
    SfReader reader;
    uint8_t magic[4];
    if (profile == NULL || profile->source == NULL || profile->source_size < 16) {
        return SF_STATUS_ARGUMENT;
    }
    sf_reader_init(&reader, profile->source, profile->source_size);
    if (!sf_reader_remaining(&reader, 4)) {
        return SF_STATUS_FORMAT;
    }
    memcpy(magic, reader.data + reader.pos, 4);
    reader.pos += 4;
    if (magic[0] != SF_MAGIC_0 || magic[1] != SF_MAGIC_1 || magic[2] != SF_MAGIC_2 || magic[3] != SF_MAGIC_3) {
        sf_set_error(profile, SF_STATUS_FORMAT, "header magic");
        return SF_STATUS_FORMAT;
    }
    if (!sf_reader_u16(&reader, &profile->header.version) ||
        !sf_reader_u16(&reader, &profile->header.section_count) ||
        !sf_reader_u32(&reader, &profile->header.directory_offset) ||
        !sf_reader_u32(&reader, &profile->header.declared_size)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "header fields");
        return SF_STATUS_FORMAT;
    }
    if (profile->header.version != 1 || profile->header.section_count == 0 || profile->header.section_count > SF_MAX_SECTIONS) {
        sf_set_error(profile, SF_STATUS_FORMAT, "header range");
        return SF_STATUS_FORMAT;
    }
    if (profile->header.declared_size != profile->source_size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "header size");
        return SF_STATUS_FORMAT;
    }
    if (profile->header.directory_offset < 16 || profile->header.directory_offset > profile->source_size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "directory offset");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
