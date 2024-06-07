#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_THREADS 16
#define SIZE_FILEPATH 500
#define CHUNK_SIZE 1024

typedef struct {
    char filePath[SIZE_FILEPATH];
} FileData;

pthread_mutex_t lock;
FileData *fileList = NULL;
size_t fileListSize = 0;
bool deleteDuplicates = false;
bool recursiveSearch = false;
int fileCount[5] = {0}; // Number of each file type: 0-txt, 1-jpg, 2-pdf, 3-py, 4-others

// Function to write log to a file
void write_log(const char *logFilePath, const char *message) {
    FILE *logFile = fopen(logFilePath, "a");
    if (logFile == NULL) {
        fprintf(stderr, "Error opening log file: %s\n", logFilePath);
        return;
    }

    fprintf(logFile, "%s\n", message);
    fclose(logFile);
}

// Function to compare files
void *compare_files(void *arg) {
    char *filePath = (char *)arg;
    FileData data;
    strcpy(data.filePath, filePath);

    pthread_mutex_lock(&lock);
    if (fileListSize % MAX_THREADS == 0) {
        void *newListPtr = realloc(fileList, (fileListSize + MAX_THREADS) * sizeof(FileData));
        if (newListPtr == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        fileList = (FileData *)newListPtr;
    }
    fileList[fileListSize++] = data;
    pthread_mutex_unlock(&lock);

    return NULL;
}

// Function to process directory
void process_directory(const char *basePath, pthread_t *threads, char (*filePaths)[SIZE_FILEPATH], int *thread_count) {
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    if ((dir = opendir(basePath)) == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", basePath);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode) && recursiveSearch) {
                process_directory(path, threads, filePaths, thread_count);
            } else if (S_ISREG(statbuf.st_mode)) {
                snprintf(filePaths[*thread_count],
                         sizeof(filePaths[*thread_count]), "%s", path);
                pthread_create(&threads[*thread_count], NULL, compare_files, filePaths[*thread_count]);

                (*thread_count)++;
                if (*thread_count >= MAX_THREADS) {
                    for (int i = 0; i < MAX_THREADS; i++) {
                        pthread_join(threads[i], NULL);
                    }
                    *thread_count = 0;
                }
                // Update file type count
                char *extension = strrchr(entry->d_name, '.');
                if (extension != NULL) {
                    if (strcmp(extension, ".txt") == 0) fileCount[0]++;
                    else if (strcmp(extension, ".jpg") == 0) fileCount[1]++;
                    else if (strcmp(extension, ".pdf") == 0) fileCount[2]++;
                    else if (strcmp(extension, ".py") == 0) fileCount[3]++;
                    else fileCount[4]++;
                }
            }
        }
    }

    for (int i = 0; i < *thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    *thread_count = 0;

    closedir(dir);
}

// Function to compare file contents
bool compare_file_contents(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return false;
    }

    char buf1[CHUNK_SIZE], buf2[CHUNK_SIZE];
    size_t bytesRead1, bytesRead2;
    while ((bytesRead1 = fread(buf1, 1, CHUNK_SIZE, f1)) > 0 && (bytesRead2 = fread(buf2, 1, CHUNK_SIZE, f2)) > 0) {
        if (bytesRead1 != bytesRead2 || memcmp(buf1, buf2, bytesRead1) != 0) {
            fclose(f1);
            fclose(f2);
            return false;
        }
    }

    bool equal = (feof(f1) && feof(f2));
    fclose(f1);
    fclose(f2);
    return equal;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(
            "Usage: %s [options] <directory> \n where options include:\n "
            "\t-d\n \t\tdelete duplicates and keep one instance\n"
            "\t-r\n \t\trecursively search subdirectories\n",
            argv[0]);
        return 1;
    }

    char *directoryPath = NULL;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-d") == 0) {
                deleteDuplicates = true;
            } else if (strcmp(argv[i], "-r") == 0) {
                recursiveSearch = true;
            }
        } else {
            directoryPath = argv[i];
        }
    }

    // Define the path for the log file
    char logFilePath[SIZE_FILEPATH];
    snprintf(logFilePath, sizeof(logFilePath), "%s/duplicate_files.log", directoryPath);

    // Write a header to the log file
    write_log(logFilePath, "Duplicate Files Log");
    write_log(logFilePath, "-------------------");
    write_log(logFilePath, "Starting duplicate file search...");

    pthread_t threads[MAX_THREADS];
    char filePaths[MAX_THREADS][SIZE_FILEPATH];
    int thread_count = 0;
    fileList = (FileData *)malloc(MAX_THREADS * sizeof(FileData));
    if (fileList == NULL) {
        fprintf(stderr, "Initial memory allocation failed\n");
        return 1;
    }

    pthread_mutex_init(&lock, NULL);
    process_directory(directoryPath, threads, filePaths, &thread_count);
    pthread_mutex_destroy(&lock);
    printf("Total files processed: %zu\n", fileListSize); // Print total number of files processed

    size_t total_path_size_before = 0;
    for (size_t i = 0; i < fileListSize; i++) {
        total_path_size_before += strlen(fileList[i].filePath);
    }
    printf("Total size of paths before removing duplicates: %zu bytes\n", total_path_size_before);
    write_log(logFilePath, "Duplicate file search completed.");

    bool *printed = (bool *)calloc(fileListSize, sizeof(bool));
    if (printed == NULL) {
        fprintf(stderr, "Memory allocation for printed flags failed\n");
        return 1;
    }

    for (size_t i = 0; i < fileListSize; i++) {
        bool found_duplicate = false;
        if (printed[i]) continue;
        for (size_t j = i + 1; j < fileListSize; j++) {
            if (i != j && compare_file_contents(fileList[i].filePath, fileList[j].filePath)) {
                if (!printed[i]) {
                    printf("Duplicate Group:   [Contents]  ");
                    printf("\n");
                    printf("  %s\n", fileList[i].filePath);
                    printed[i] = true;
                    found_duplicate = true;
                }
                if (!printed[j]) {
                    printf("  %s\n", fileList[j].filePath);
                    printed[j] = true;
                    if (deleteDuplicates) {
                        if (remove(fileList[j].filePath) != 0) {
                            fprintf(stderr, "Error deleting file: %s\n", fileList[j].filePath);
                        }
                    }
                }
            }
        }

        if (found_duplicate) {
            printf("\n");
        }
    }

    size_t total_path_size_after = 0;
    for (size_t i = 0; i < fileListSize; i++) {
        if (!printed[i]) {
            total_path_size_after += strlen(fileList[i].filePath);
        }
    }
    printf("Total size of paths after removing duplicates: %zu bytes\n", total_path_size_after);

    printf("\nNumber of each file type:\n");
    printf("- .txt: %d\n", fileCount[0]);
    printf("- .jpg: %d\n", fileCount[1]);
    printf("- .pdf: %d\n", fileCount[2]);
    printf("- .py: %d\n", fileCount[3]);
    printf("- Others: %d\n", fileCount[4]);

    free(printed);
    free(fileList);

    return 0;
}
