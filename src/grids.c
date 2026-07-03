#include "internal.h"

void sf_release_grid_members(SfGrid *item) {
    if (item == NULL) {
        return;
    }
    free(item->members);
    item->members = NULL;
    item->member_count = 0;
    item->state = 2;
}

SfGrid *sf_find_grid_record(SfProfile *profile, uint16_t id) {
    size_t index;
    if (profile == NULL) {
        return NULL;
    }
    for (index = 0; index < profile->grid_count; ++index) {
        if (profile->grids[index].id == id && profile->grids[index].state != 0) {
            return &profile->grids[index];
        }
    }
    return NULL;
}

SfStatus sf_parse_grids(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_GRID, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "grid count");
        return SF_STATUS_FORMAT;
    }
    profile->grids = (SfGrid *)sf_allocate(count, sizeof(SfGrid));
    if (count > 0 && profile->grids == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "grid allocation");
        return SF_STATUS_MEMORY;
    }
    profile->grid_count = count;
    for (index = 0; index < count; ++index) {
        SfGrid *item = &profile->grids[index];
        uint16_t member_count;
        size_t member_index;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &item->label_id) ||
            !sf_reader_u8(&reader, &item->flags) || !sf_reader_u16(&reader, &member_count)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "grid entry");
            return SF_STATUS_FORMAT;
        }
        if (member_count == 0 || member_count > 2048) {
            sf_set_error(profile, SF_STATUS_FORMAT, "grid member count");
            return SF_STATUS_FORMAT;
        }
        item->members = (uint16_t *)sf_allocate(member_count, sizeof(uint16_t));
        if (item->members == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "grid members");
            return SF_STATUS_MEMORY;
        }
        for (member_index = 0; member_index < member_count; ++member_index) {
            if (!sf_reader_u16(&reader, &item->members[member_index])) {
                sf_set_error(profile, SF_STATUS_FORMAT, "grid member");
                return SF_STATUS_FORMAT;
            }
        }
        item->member_count = member_count;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "grid tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
