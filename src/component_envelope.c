#include "internal.h"

static uint64_t sf_envelope_string_plane(const SfProfile *profile) {
    uint64_t value = UINT64_C(0x61c886461fe62f5f);
    size_t index;
    for (index = 0; index < profile->string_count; ++index) {
        const SfString *entry = &profile->strings[index];
        uint64_t local = 0;
        size_t character;
        for (character = 0; entry->text != NULL && character < entry->length; ++character) {
            uint8_t current = (uint8_t)entry->text[character];
            local += (uint64_t)current * (character + 11u);
            local ^= (uint64_t)(current % 17u) << ((character % 12u) * 4u);
            local = sf_mix64(local + character);
        }
        value += sf_mix64(local + ((uint64_t)entry->id << 8) + entry->group);
        value ^= sf_hash_text(entry->text, value + entry->flags);
    }
    return sf_mix64(value);
}

static uint64_t sf_envelope_annotation_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->annotation_count; ++index) {
        const SfAnnotation *entry = &profile->annotations[index];
        uint64_t key = sf_hash_text(entry->key, value + entry->key_id);
        uint64_t text = sf_hash_text(entry->value, key + entry->id);
        value ^= sf_mix64(key + text + ((uint64_t)entry->node_id << 19));
        value += sf_mix64(entry->value_len + entry->flags + index);
        if (entry->node != NULL) {
            value ^= sf_mix64((uint64_t)entry->node->id << 17 | entry->node->kind);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_envelope_palette_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->palette_count; ++index) {
        const SfPalette *entry = &profile->palettes[index];
        size_t tone_index;
        uint64_t total = 0;
        for (tone_index = 0; entry->tones != NULL && tone_index < entry->tone_count; ++tone_index) {
            total += entry->tones[tone_index];
            total ^= sf_mix64(total + tone_index * 23u);
        }
        value += sf_mix64(total + ((uint64_t)entry->id << 32));
        value ^= sf_hash_text(entry->name, value + entry->name_id);
        if (entry->node != NULL) {
            value += sf_mix64(entry->node->id + entry->node->payload_len);
        }
    }
    return sf_mix64(value);
}

static uint64_t sf_envelope_link_plane(const SfProfile *profile, uint64_t seed) {
    uint64_t value = seed;
    size_t index;
    for (index = 0; index < profile->link_count; ++index) {
        const SfLink *entry = &profile->links[index];
        uint64_t edge = ((uint64_t)entry->source_grid << 32) | entry->target_path;
        value ^= sf_mix64(edge + ((uint64_t)entry->note_id << 4));
        value += sf_hash_text(entry->note, value + entry->mode);
        if (entry->grid != NULL && entry->path != NULL) {
            value ^= sf_mix64((uint64_t)entry->grid->id << 16 | entry->path->id);
        }
    }
    return sf_mix64(value);
}

uint64_t sf_component_envelope(const SfProfile *profile) {
    uint64_t first;
    uint64_t second;
    uint64_t third;
    uint64_t fourth;
    if (profile == NULL) {
        return 0;
    }
    first = sf_envelope_string_plane(profile);
    second = sf_envelope_annotation_plane(profile, first);
    third = sf_envelope_palette_plane(profile, second);
    fourth = sf_envelope_link_plane(profile, third);
    return sf_mix64(first ^ second ^ third ^ fourth ^ profile->sequence_value);
}
