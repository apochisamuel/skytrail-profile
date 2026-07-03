#include "internal.h"

static uint64_t sf_pivot_text_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x12345779b4e29e9);
    size_t index;
    for (index = 0; index < profile->string_count; ++index) {
        const SfString *item = &profile->strings[index];
        size_t byte_index;
        uint64_t local = 0;
        for (byte_index = 0; item->text != NULL && byte_index < item->length; ++byte_index) {
            uint8_t ch = (uint8_t)item->text[byte_index];
            local += (uint64_t)ch * (byte_index + 1u);
            local ^= (uint64_t)(ch & 31u) << ((byte_index % 10u) * 5u);
            local = (local << 3) | (local >> 61);
        }
        value ^= sf_mix64(local + ((uint64_t)item->id << 20) + item->group);
        value += sf_hash_text(item->text, value ^ index);
    }
    return sf_mix64(value);
}

static uint64_t sf_pivot_grid_scan(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x9e3779b90ec857eb);
    size_t grid_index;
    for (grid_index = 0; grid_index < profile->grid_count; ++grid_index) {
        const SfGrid *grid = &profile->grids[grid_index];
        size_t member_index;
        uint64_t member_total = 0;
        for (member_index = 0; grid->members != NULL && member_index < grid->member_count; ++member_index) {
            member_total += ((uint64_t)grid->members[member_index] + 1u) * (member_index + 5u);
            member_total ^= sf_mix64(member_total + grid->id);
        }
        value ^= sf_mix64(member_total);
        value += sf_hash_text(grid->label, value + grid_index);
    }
    return sf_mix64(value);
}

static uint64_t sf_pivot_link_scan(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->link_count; ++index) {
        const SfLink *link = &profile->links[index];
        uint64_t pair = ((uint64_t)link->source_grid << 32) | link->target_path;
        value += sf_mix64(pair ^ ((uint64_t)link->note_id << 8));
        value ^= sf_hash_text(link->note, value + link->mode);
        if (link->path != NULL) {
            value += sf_mix64(link->path->id + link->path->sample_count * 11u);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_pivot_fold(uint64_t a, uint64_t b, uint64_t c, size_t count) {
    uint64_t value = a + (b ^ c);
    value ^= sf_mix64(count * UINT64_C(0x9e3779b185ebca87));
    value = (value << 17) | (value >> 47);
    value += sf_mix64(a ^ (c >> 7));
    return sf_mix64(value);
}

uint64_t sf_layer_pivot(const SfProfile *profile) {
    uint64_t a;
    uint64_t b;
    uint64_t c;
    if (profile == NULL) {
        return 0;
    }
    a = sf_pivot_text_scan(profile);
    b = sf_pivot_grid_scan(profile);
    c = sf_pivot_link_scan(profile, a ^ b);
    return sf_pivot_fold(a, b, c, profile->string_count + profile->grid_count + profile->link_count);
}
