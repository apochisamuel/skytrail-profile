#include "internal.h"

void *sf_allocate(size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        return NULL;
    }
    if (count == 0 || size == 0) {
        return calloc(1, 1);
    }
    return calloc(count, size);
}

void *sf_resize(void *ptr, size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        return NULL;
    }
    if (count == 0 || size == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, count * size);
}

char *sf_copy_text(const uint8_t *bytes, size_t length) {
    char *text;
    if (length > SF_MAX_TEXT_BYTES) {
        return NULL;
    }
    text = (char *)sf_allocate(length + 1, sizeof(char));
    if (text == NULL) {
        return NULL;
    }
    if (length > 0 && bytes != NULL) {
        memcpy(text, bytes, length);
    }
    text[length] = '\0';
    return text;
}

size_t sf_next_capacity(size_t current, size_t needed) {
    size_t value = current == 0 ? 4 : current;
    while (value < needed) {
        if (value > SIZE_MAX / 2) {
            return needed;
        }
        value *= 2;
    }
    return value;
}

uint64_t sf_mix64(uint64_t value) {
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31;
    return value;
}

uint64_t sf_hash_bytes(const uint8_t *data, size_t length, uint64_t seed) {
    uint64_t value = seed ^ (uint64_t)length * UINT64_C(0x9e3779b185ebca87);
    size_t index;
    for (index = 0; index < length; ++index) {
        value ^= (uint64_t)data[index] + UINT64_C(0x9e3779b97f4a7c15) + (value << 6) + (value >> 2);
        value = sf_mix64(value);
    }
    return value;
}

uint64_t sf_hash_text(const char *text, uint64_t seed) {
    if (text == NULL) {
        return sf_mix64(seed ^ UINT64_C(0x7f4a7c159e3779b9));
    }
    return sf_hash_bytes((const uint8_t *)text, strlen(text), seed);
}

uint32_t sf_tag_from_bytes(const uint8_t bytes[4]) {
    return SF_TAG(bytes[0], bytes[1], bytes[2], bytes[3]);
}
