#include "skytrail.h"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SfStatus status = SF_STATUS_OK;
    SfProfile *profile;
    char report[256];
    size_t needed = 0;
    uint64_t annotations = 0;
    uint64_t palette = 0;
    if (size < 64 || size > 16*1024*1024) return 0;
    profile = sf_profile_open_memory(data, size, &status);
    if (profile == nullptr) return 0;
    (void)sf_profile_lookup_name(profile, 1);
    annotations = sf_profile_annotation_weight(profile);
    palette = sf_profile_palette_weight(profile);
    (void)sf_profile_report(profile, report, sizeof(report), &needed);
    if ((annotations + palette + needed) == 17) {
        (void)sf_profile_string_count(profile);
    }
    sf_profile_close(profile);
    return 0;
}
