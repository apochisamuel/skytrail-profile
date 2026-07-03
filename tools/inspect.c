#include "skytrail.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_profile(const char *path, size_t *size_out) {
    FILE *file = fopen(path, "rb");
    long length;
    unsigned char *data;
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)length);
    if (data == NULL) {
        fclose(file);
        return NULL;
    }
    if (fread(data, 1, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *size_out = (size_t)length;
    return data;
}

int main(int argc, char **argv) {
    unsigned char *data;
    size_t size;
    SfStatus status = SF_STATUS_OK;
    SfProfileInfo info;
    SfProfile *profile;
    char report[320];
    size_t needed = 0;
    if (argc != 2) return 2;
    data = read_profile(argv[1], &size);
    if (data == NULL) return 3;
    profile = sf_profile_open_memory(data, size, &status);
    if (profile == NULL) {
        fprintf(stderr, "profile status: %s\n", sf_status_string(status));
        free(data);
        return 4;
    }
    if (sf_profile_info(profile, &info) != SF_STATUS_OK ||
        sf_profile_report(profile, report, sizeof(report), &needed) != SF_STATUS_OK) {
        sf_profile_close(profile);
        free(data);
        return 5;
    }
    printf("%s\n", report);
    sf_profile_close(profile);
    free(data);
    return 0;
}
