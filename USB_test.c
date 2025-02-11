#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <time.h>

void write_speed_test(const char *file_path) {
    FILE *file = fopen(file_path, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    const size_t buffer_size = 1024 * 1024;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }
    memset(buffer, 0, buffer_size);

    size_t total_size = 0;
    clock_t start_time = clock();
    for (size_t i = 0; i < 1024; ++i) { // Write 1GB of data
        size_t written = fwrite(buffer, 1, buffer_size, file);
        if (written != buffer_size) {
            perror("Failed to write to file");
            break;
        }
        total_size += written;
    }
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Write speed: %.2f MB/s\n", total_size / (1024.0 * 1024.0) / elapsed_time);

    free(buffer);
    fclose(file);
}

void read_speed_test(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    const size_t buffer_size = 1024 * 1024; // 1MB
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    size_t total_size = 0;
    clock_t start_time = clock();
    while (fread(buffer, 1, buffer_size, file) == buffer_size) {
        total_size += buffer_size;
    }
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Read speed: %.2f MB/s\n", total_size / (1024.0 * 1024.0) / elapsed_time);

    free(buffer);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *device_name = argv[1];
    FILE *mnt_file;
    struct mntent *mnt_entry;
    const char *mount_point = NULL;

    // Open /proc/mounts file
    mnt_file = setmntent("/proc/mounts", "r");
    if (mnt_file == NULL) {
        perror("Failed to open /proc/mounts");
        return EXIT_FAILURE;
    }

    // Iterate through mount points
    while ((mnt_entry = getmntent(mnt_file)) != NULL) {
        if (strcmp(mnt_entry->mnt_fsname, device_name) == 0) {
            mount_point = mnt_entry->mnt_dir;
            break;
        }
    }

    if (mount_point == NULL) {
        fprintf(stderr, "Mount point for device %s not found\n", device_name);
        endmntent(mnt_file);
        return EXIT_FAILURE;
    }

    printf("Device %s is mounted at %s\n", device_name, mount_point);

    // Construct the path for the test file
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s/testfile", mount_point);

    write_speed_test(file_path);
    read_speed_test(file_path);

    endmntent(mnt_file);
    return EXIT_SUCCESS;
}
