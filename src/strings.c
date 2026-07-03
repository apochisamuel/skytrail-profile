#include "internal.h"

void sf_release_string_text(SfString *item) {
    if (item == NULL) {
        return;
    }
    free(item->text);
    item->text = NULL;
    item->length = 0;
    item->state = 2;
}

SfString *sf_find_string_record(SfProfile *profile, uint16_t id) {
    size_t index;
    if (profile == NULL) {
        return NULL;
    }
    for (index = 0; index < profile->string_count; ++index) {
        if (profile->strings[index].id == id && profile->strings[index].state == 1) {
            return &profile->strings[index];
        }
    }
    return NULL;
}

SfStatus sf_parse_strings(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_STRS, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "string count");
        return SF_STATUS_FORMAT;
    }
    profile->strings = (SfString *)sf_allocate(count, sizeof(SfString));
    if (count > 0 && profile->strings == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "string allocation");
        return SF_STATUS_MEMORY;
    }
    profile->string_count = count;
    for (index = 0; index < count; ++index) {
        SfString *item = &profile->strings[index];
        uint16_t text_len;
        const uint8_t *text_bytes;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &text_len) ||
            !sf_reader_u8(&reader, &item->group) || !sf_reader_u8(&reader, &item->flags) ||
            !sf_reader_bytes(&reader, &text_bytes, text_len)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "string entry");
            return SF_STATUS_FORMAT;
        }
        item->text = sf_copy_text(text_bytes, text_len);
        if (item->text == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "string text");
            return SF_STATUS_MEMORY;
        }
        item->length = text_len;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "string tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
