#include "internal.h"

static uint64_t sf_notch_source_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x12345765edf3687);
    size_t index;
    size_t stride = (size_t)(8);
    for (index = 0; index < profile->source_size; index += stride) {
        uint64_t byte = profile->source[index];
        value ^= sf_mix64(byte + index * 257u);
        value += (byte << ((index / stride) % 13u));
        value = (value << 7) | (value >> 57);
    }
    return sf_mix64(value);
}

static uint64_t sf_notch_record_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9e3779b9178c919d);
    size_t index;
    for (index = 0; index < profile->string_count; ++index) {
        const SfString *string = &profile->strings[index];
        value += sf_hash_text(string->text, value + string->id);
        value ^= sf_mix64((uint64_t)string->group << 32 | string->flags);
    }
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *annotation = &profile->annotations[index];
        value ^= sf_hash_text(annotation->key, value + annotation->id);
        value += sf_hash_text(annotation->value, value + annotation->node_id);
    }
    return sf_mix64(value);
}

static uint64_t sf_notch_section_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x94d049bb133111eb);
    size_t index;
    for (index = 0; index < profile->directory_count; ++index) {
        const SfDirectoryEntry *entry = &profile->directory[index];
        value += sf_mix64(entry->tag + entry->offset + entry->length);
        value ^= sf_mix64((uint64_t)entry->flags << (index % 19u));
    }
    value ^= sf_metric_section_layout(profile);
    return sf_mix64(value);
}

static uint64_t sf_notch_settle(const SfProfile *profile, uint64_t a, uint64_t b, uint64_t c) {
    uint64_t value = a + sf_mix64(b);
    size_t index;
    value ^= sf_mix64(c + profile->validation_value);
    for (index = 0; index < profile->link_count; ++index) {
        value += sf_mix64(profile->links[index].mode + index * 41u);
        value ^= sf_hash_text(profile->links[index].note, value);
    }
    return sf_mix64(value);
}

uint64_t sf_layer_notch(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_notch_source_scan(profile);
    b = sf_notch_record_scan(profile);
    c = sf_notch_section_scan(profile);
    return sf_notch_settle(profile, a, b, c);
}
