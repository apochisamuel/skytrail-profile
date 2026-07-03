#include "internal.h"

static uint64_t sf_annotation_text_value(const SfAnnotation *item) {
    uint64_t value = sf_hash_text(item->value, UINT64_C(0x65a4c3d19e2b7f08));
    size_t index;
    if (item->value == NULL) {
        return value;
    }
    for (index = 0; index < item->value_len; ++index) {
        uint8_t ch = (uint8_t)item->value[index];
        value ^= sf_mix64((uint64_t)ch << ((index % 8) * 8));
        value += ch == ':' ? UINT64_C(0x11) : UINT64_C(0x1d);
    }
    return sf_mix64(value);
}

static uint64_t sf_annotation_relation_value(const SfAnnotation *item) {
    uint64_t value = ((uint64_t)item->id << 40) | ((uint64_t)item->key_id << 20) | item->node_id;
    if (item->node != NULL) {
        value ^= sf_mix64((uint64_t)item->node->id << 32 | item->node->flags);
    }
    value ^= sf_hash_text(item->key, value);
    return sf_mix64(value);
}

uint64_t sf_metric_annotations(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x1d3e5f7092a4c6e8);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *item = &profile->annotations[index];
        value += sf_annotation_text_value(item);
        value ^= sf_annotation_relation_value(item);
        value = sf_mix64(value + index + item->flags);
    }
    return sf_mix64(value ^ profile->annotation_count);
}
