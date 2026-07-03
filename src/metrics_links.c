#include "internal.h"

static uint64_t sf_link_endpoints(const SfLink *item) {
    uint64_t value = ((uint64_t)item->source_grid << 32) | item->target_path;
    value ^= (uint64_t)item->note_id << 7;
    value += (uint64_t)item->mode * UINT64_C(0x9e3779b9);
    if (item->grid != NULL) value ^= sf_mix64(item->grid->id);
    if (item->path != NULL) value += sf_mix64(item->path->id);
    return sf_mix64(value);
}

static uint64_t sf_link_note(const SfLink *item, uint64_t seed) {
    uint64_t value = sf_hash_text(item->note, seed);
    if (item->note != NULL) {
        size_t length = strlen(item->note);
        value ^= sf_mix64(length * 37u + item->mode);
    }
    return sf_mix64(value);
}

uint64_t sf_metric_links(const SfProfile *profile) {
    uint64_t value = UINT64_C(0xc81e764fb29ad035);
    size_t index;
    if (profile == NULL) return 0;
    for (index = 0; index < profile->link_count; ++index) {
        value ^= sf_link_endpoints(&profile->links[index]);
        value += sf_link_note(&profile->links[index], value + index);
        value = sf_mix64(value);
    }
    return sf_mix64(value ^ profile->link_count);
}
