#include "internal.h"

void sf_release_palette_tones(SfPalette *item) {
    if (item == NULL) {
        return;
    }
    free(item->tones);
    item->tones = NULL;
    item->tone_count = 0;
    item->state = 2;
}

SfStatus sf_parse_palettes(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_PALC, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "palette count");
        return SF_STATUS_FORMAT;
    }
    profile->palettes = (SfPalette *)sf_allocate(count, sizeof(SfPalette));
    if (count > 0 && profile->palettes == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "palette allocation");
        return SF_STATUS_MEMORY;
    }
    profile->palette_count = count;
    for (index = 0; index < count; ++index) {
        SfPalette *item = &profile->palettes[index];
        uint16_t tone_count;
        size_t tone_index;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &item->name_id) ||
            !sf_reader_u16(&reader, &item->node_id) || !sf_reader_u8(&reader, &item->flags) ||
            !sf_reader_u16(&reader, &tone_count)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "palette entry");
            return SF_STATUS_FORMAT;
        }
        if (tone_count == 0 || tone_count > 1024) {
            sf_set_error(profile, SF_STATUS_FORMAT, "palette tone count");
            return SF_STATUS_FORMAT;
        }
        item->tones = (uint16_t *)sf_allocate(tone_count, sizeof(uint16_t));
        if (item->tones == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "palette tones");
            return SF_STATUS_MEMORY;
        }
        for (tone_index = 0; tone_index < tone_count; ++tone_index) {
            if (!sf_reader_u16(&reader, &item->tones[tone_index])) {
                sf_set_error(profile, SF_STATUS_FORMAT, "palette tone");
                return SF_STATUS_FORMAT;
            }
        }
        item->tone_count = tone_count;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "palette tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
