#include "skytrail.h"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SfStatus status = SF_STATUS_OK;
    SfProfile *profile;
    SfProfileInfo info;
    uint64_t result = 0;
    if (size < 64 || size > 16*1024*1024) return 0;
    profile = sf_profile_open_memory(data, size, &status);
    if (profile == nullptr) return 0;
    (void)sf_profile_info(profile, &info);
    (void)sf_profile_walk(profile, &result);
    (void)sf_profile_digest(profile);
    sf_profile_close(profile);
    return 0;
}
