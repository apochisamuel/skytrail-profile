#include "skytrail.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, size_t *size_out) {
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *data;
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (data == NULL) {
        fclose(file);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *size_out = (size_t)size;
    return data;
}

int main(int argc, char **argv) {
    unsigned char *data;
    size_t size;
    SfStatus status = SF_STATUS_OK;
    SfProfileInfo info;
    SfProfile *profile;
    uint64_t walk = 0;
    if (argc != 2) return 2;
    data = read_file(argv[1], &size);
    if (data == NULL) return 3;
    profile = sf_profile_open_memory(data, size, &status);
    if (profile == NULL) {
        free(data);
        return 4;
    }
    if (sf_profile_info(profile, &info) != SF_STATUS_OK || info.node_count != 1 || info.path_count != 1) {
        sf_profile_close(profile);
        free(data);
        return 5;
    }
    if (sf_profile_walk(profile, &walk) != SF_STATUS_OK || walk == 0) {
        sf_profile_close(profile);
        free(data);
        return 6;
    }
    if (sf_profile_lookup_name(profile, 2) == NULL) {
        sf_profile_close(profile);
        free(data);
        return 7;
    }
    sf_profile_close(profile);
    free(data);
    return 0;
}
