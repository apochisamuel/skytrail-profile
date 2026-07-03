#include "skytrail.h"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SfStatus status = SF_STATUS_OK;
    SfProfile *profile;
    uint64_t graph = 0;
    uint64_t timeline = 0;
    if (size < 64 || size > 16*1024*1024) return 0;
    profile = sf_profile_open_memory(data, size, &status);
    if (profile == nullptr) return 0;
    graph = sf_profile_graph_weight(profile);
    timeline = sf_profile_timeline_weight(profile);
    if ((graph ^ timeline) == 0xFFFFFFFFFFFFFFFFULL) {
        (void)sf_profile_path_count(profile);
    }
    sf_profile_close(profile);
    return 0;
}
