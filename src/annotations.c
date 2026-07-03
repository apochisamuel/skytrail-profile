#include "internal.h"

void sf_release_annotation_value(SfAnnotation *item) {
    if (item == NULL) {
        return;
    }
    free(item->value);
    item->value = NULL;
    item->value_len = 0;
    item->state = 2;
}

SfStatus sf_parse_annotations(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_META, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "annotation count");
        return SF_STATUS_FORMAT;
    }
    profile->annotations = (SfAnnotation *)sf_allocate(count, sizeof(SfAnnotation));
    if (count > 0 && profile->annotations == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "annotation allocation");
        return SF_STATUS_MEMORY;
    }
    profile->annotation_count = count;
    for (index = 0; index < count; ++index) {
        SfAnnotation *item = &profile->annotations[index];
        uint16_t value_len;
        const uint8_t *value_bytes;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &item->key_id) ||
            !sf_reader_u16(&reader, &item->node_id) || !sf_reader_u8(&reader, &item->flags) ||
            !sf_reader_u16(&reader, &value_len) || !sf_reader_bytes(&reader, &value_bytes, value_len)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "annotation entry");
            return SF_STATUS_FORMAT;
        }
        if (value_len == 0) {
            sf_set_error(profile, SF_STATUS_FORMAT, "annotation value");
            return SF_STATUS_FORMAT;
        }
        item->value = sf_copy_text(value_bytes, value_len);
        if (item->value == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "annotation text");
            return SF_STATUS_MEMORY;
        }
        item->value_len = value_len;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "annotation tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
