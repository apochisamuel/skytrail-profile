#include "internal.h"

static uint64_t sf_string_byte_classes(const SfString *item) {
    uint64_t counts[4] = {0, 0, 0, 0};
    size_t index;
    uint64_t value;
    if (item->text == NULL) {
        return sf_mix64(item->id);
    }
    for (index = 0; index < item->length; ++index) {
        uint8_t ch = (uint8_t)item->text[index];
        if (ch >= 'a' && ch <= 'z') counts[0]++;
        else if (ch >= 'A' && ch <= 'Z') counts[1]++;
        else if (ch >= '0' && ch <= '9') counts[2]++;
        else counts[3]++;
    }
    value = counts[0] | (counts[1] << 12) | (counts[2] << 24) | (counts[3] << 36);
    return sf_mix64(value ^ item->id);
}

static uint64_t sf_string_shape(const SfString *item) {
    uint64_t value = sf_hash_text(item->text, ((uint64_t)item->id << 16) | item->group);
    size_t index;
    for (index = 1; item->text != NULL && index < item->length; ++index) {
        int delta = (unsigned char)item->text[index] - (unsigned char)item->text[index - 1];
        value ^= sf_mix64((uint64_t)(delta < 0 ? -delta : delta) + index * 13u);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_strings(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xb31d84f952c6ae07);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->string_count; ++index) {
        const SfString *item = &profile->strings[index];
        value += sf_string_byte_classes(item);
        value ^= sf_string_shape(item);
        value = sf_mix64(value + item->flags + index);
    }
    return sf_mix64(value);
}
