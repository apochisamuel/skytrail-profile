#include "internal.h"

SfStatus sf_parse_links(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_LINK, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "link count");
        return SF_STATUS_FORMAT;
    }
    profile->links = (SfLink *)sf_allocate(count, sizeof(SfLink));
    if (count > 0 && profile->links == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "link allocation");
        return SF_STATUS_MEMORY;
    }
    profile->link_count = count;
    for (index = 0; index < count; ++index) {
        SfLink *item = &profile->links[index];
        if (!sf_reader_u16(&reader, &item->source_grid) || !sf_reader_u16(&reader, &item->target_path) ||
            !sf_reader_u16(&reader, &item->note_id) || !sf_reader_u8(&reader, &item->mode)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "link entry");
            return SF_STATUS_FORMAT;
        }
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "link tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
