#ifndef SKYTRAIL_H
#define SKYTRAIL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SfProfile SfProfile;

typedef enum SfStatus {
    SF_STATUS_OK = 0,
    SF_STATUS_ARGUMENT = 1,
    SF_STATUS_FORMAT = 2,
    SF_STATUS_MEMORY = 3,
    SF_STATUS_LIMIT = 4,
    SF_STATUS_STATE = 5
} SfStatus;

typedef struct SfProfileInfo {
    uint16_t version;
    uint16_t section_count;
    size_t string_count;
    size_t node_count;
    size_t path_count;
    size_t grid_count;
    size_t annotation_count;
    size_t palette_count;
} SfProfileInfo;

SfProfile *sf_profile_open_memory(const uint8_t *data, size_t size, SfStatus *status_out);
void sf_profile_close(SfProfile *profile);
SfStatus sf_profile_info(const SfProfile *profile, SfProfileInfo *info_out);
size_t sf_profile_node_count(const SfProfile *profile);
size_t sf_profile_path_count(const SfProfile *profile);
size_t sf_profile_string_count(const SfProfile *profile);
const char *sf_profile_lookup_name(const SfProfile *profile, uint16_t id);
uint64_t sf_profile_digest(const SfProfile *profile);
uint64_t sf_profile_graph_weight(const SfProfile *profile);
uint64_t sf_profile_timeline_weight(const SfProfile *profile);
uint64_t sf_profile_annotation_weight(const SfProfile *profile);
uint64_t sf_profile_palette_weight(const SfProfile *profile);
SfStatus sf_profile_walk(const SfProfile *profile, uint64_t *result_out);
SfStatus sf_profile_report(const SfProfile *profile, char *buffer, size_t capacity, size_t *needed_out);
const char *sf_status_string(SfStatus status);

#ifdef __cplusplus
}
#endif

#endif
