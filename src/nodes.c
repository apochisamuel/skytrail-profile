#include "internal.h"

void sf_release_node_payload(SfNode *item) {
    if (item == NULL) {
        return;
    }
    free(item->payload);
    item->payload = NULL;
    item->payload_len = 0;
    item->state = 2;
}

SfNode *sf_find_node_record(SfProfile *profile, uint16_t id) {
    size_t index;
    if (profile == NULL) {
        return NULL;
    }
    for (index = 0; index < profile->node_count; ++index) {
        if (profile->nodes[index].id == id && profile->nodes[index].state != 0) {
            return &profile->nodes[index];
        }
    }
    return NULL;
}

SfStatus sf_parse_nodes(SfProfile *profile) {
    SfReader reader;
    uint16_t count;
    size_t index;
    SfStatus status = sf_section_reader(profile, SF_TAG_NODE, &reader);
    if (status != SF_STATUS_OK || reader.data == NULL) {
        return status;
    }
    if (!sf_reader_u16(&reader, &count)) {
        sf_set_error(profile, SF_STATUS_FORMAT, "node count");
        return SF_STATUS_FORMAT;
    }
    profile->nodes = (SfNode *)sf_allocate(count, sizeof(SfNode));
    if (count > 0 && profile->nodes == NULL) {
        sf_set_error(profile, SF_STATUS_MEMORY, "node allocation");
        return SF_STATUS_MEMORY;
    }
    profile->node_count = count;
    for (index = 0; index < count; ++index) {
        SfNode *item = &profile->nodes[index];
        uint16_t payload_len;
        const uint8_t *payload_bytes;
        if (!sf_reader_u16(&reader, &item->id) || !sf_reader_u16(&reader, &item->title_id) ||
            !sf_reader_u8(&reader, &item->kind) || !sf_reader_u8(&reader, &item->flags) ||
            !sf_reader_i16(&reader, &item->left) || !sf_reader_i16(&reader, &item->top) ||
            !sf_reader_i16(&reader, &item->right) || !sf_reader_i16(&reader, &item->bottom) ||
            !sf_reader_u16(&reader, &payload_len) || !sf_reader_bytes(&reader, &payload_bytes, payload_len)) {
            sf_set_error(profile, SF_STATUS_FORMAT, "node entry");
            return SF_STATUS_FORMAT;
        }
        if (payload_len == 0) {
            sf_set_error(profile, SF_STATUS_FORMAT, "node payload");
            return SF_STATUS_FORMAT;
        }
        item->payload = (uint8_t *)sf_allocate(payload_len, sizeof(uint8_t));
        if (item->payload == NULL) {
            sf_set_error(profile, SF_STATUS_MEMORY, "node data");
            return SF_STATUS_MEMORY;
        }
        memcpy(item->payload, payload_bytes, payload_len);
        item->payload_len = payload_len;
        item->state = 1;
    }
    if (reader.pos != reader.size) {
        sf_set_error(profile, SF_STATUS_FORMAT, "node tail");
        return SF_STATUS_FORMAT;
    }
    return SF_STATUS_OK;
}
