#include "internal.h"

bool sf_reader_init(SfReader *reader, const uint8_t *data, size_t size) {
    if (reader == NULL || data == NULL) {
        return false;
    }
    reader->data = data;
    reader->size = size;
    reader->pos = 0;
    return true;
}

bool sf_reader_remaining(const SfReader *reader, size_t count) {
    return reader != NULL && reader->pos <= reader->size && count <= reader->size - reader->pos;
}

bool sf_reader_u8(SfReader *reader, uint8_t *out) {
    if (!sf_reader_remaining(reader, 1) || out == NULL) {
        return false;
    }
    *out = reader->data[reader->pos++];
    return true;
}

bool sf_reader_u16(SfReader *reader, uint16_t *out) {
    uint16_t value;
    if (!sf_reader_remaining(reader, 2) || out == NULL) {
        return false;
    }
    value = (uint16_t)reader->data[reader->pos];
    value |= (uint16_t)reader->data[reader->pos + 1] << 8;
    reader->pos += 2;
    *out = value;
    return true;
}

bool sf_reader_i16(SfReader *reader, int16_t *out) {
    uint16_t raw;
    if (out == NULL || !sf_reader_u16(reader, &raw)) {
        return false;
    }
    *out = (int16_t)raw;
    return true;
}

bool sf_reader_u32(SfReader *reader, uint32_t *out) {
    uint32_t value;
    if (!sf_reader_remaining(reader, 4) || out == NULL) {
        return false;
    }
    value = (uint32_t)reader->data[reader->pos];
    value |= (uint32_t)reader->data[reader->pos + 1] << 8;
    value |= (uint32_t)reader->data[reader->pos + 2] << 16;
    value |= (uint32_t)reader->data[reader->pos + 3] << 24;
    reader->pos += 4;
    *out = value;
    return true;
}

bool sf_reader_bytes(SfReader *reader, const uint8_t **out, size_t count) {
    if (!sf_reader_remaining(reader, count) || out == NULL) {
        return false;
    }
    *out = reader->data + reader->pos;
    reader->pos += count;
    return true;
}

bool sf_reader_skip(SfReader *reader, size_t count) {
    if (!sf_reader_remaining(reader, count)) {
        return false;
    }
    reader->pos += count;
    return true;
}

bool sf_reader_slice(const uint8_t *data, size_t size, size_t offset, size_t length, SfReader *out) {
    if (data == NULL || out == NULL || offset > size || length > size - offset) {
        return false;
    }
    out->data = data + offset;
    out->size = length;
    out->pos = 0;
    return true;
}
