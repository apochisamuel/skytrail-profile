#include "internal.h"

void sf_release_path_samples(SfPath *item) {
    if (item == NULL) {
        return;
    }
    free(item->samples);
    item->samples = NULL;
    item->sample_count = 0;
    item->state = 2;
}

SfPath *sf_find_path_record(SfProfile *profile, uint16_t id) {
    size_t index;
    if (profile == NULL) {
        return NULL;
    }
    for (index = 0; index < profile->path_count; ++index) {
        if (profile->paths[index].id == id && profile->paths[index].state != 0) {
            return &profile->paths[index];
        }
    }
    return NULL;
}

SfStatus sf_parse_paths(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_PATH, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "path count");
        return SF_STATUS_FORMAT;
    }
    profile->paths = (SfPath *)sf_allocate(count, sizeof(SfPath));
    if (count > 0 && profile->paths == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "path allocation");
        return SF_STATUS_MEMORY;
    }
    profile->path_count = count;
    for (index = 0; index < count; ++index) {
        SfPath *item = &profile->paths[index];
        uint16_t sample_count;
        size_t sample_index;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &item->node_id) ||
            !sf_reader_u16(&reader, &item->name_id) || !sf_reader_u8(&reader, &item->flags) ||
            !sf_reader_u16(&reader, &sample_count)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "path entry");
            return SF_STATUS_FORMAT;
        }
        if (sample_count == 0 || sample_count > 4096) {
            sf_set_error(profile, SF_STATUS_FORMAT, "path sample count");
            return SF_STATUS_FORMAT;
        }
        item->samples = (uint16_t *)sf_allocate(sample_count, sizeof(uint16_t));
        if (item->samples == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "path samples");
            return SF_STATUS_MEMORY;
        }
        for (sample_index = 0; sample_index < sample_count; ++sample_index) {
            if (!sf_reader_u16(&reader, &item->samples[sample_index])) {
                sf_set_error(profile, SF_STATUS_FORMAT, "path sample");
                return SF_STATUS_FORMAT;
            }
        }
        item->sample_count = sample_count;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "path tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
