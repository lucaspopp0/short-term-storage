#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

char * cleanPath(char * s) {
    char *path = strdup(s);
    if (path[strlen(path) - 1] == '/') {
        path[strlen(path) - 1] = 0;
    }
    return path;
}

char * childPath(char * parent, char * child) {
    char * path = malloc(strlen(parent) + strlen(child) + 2);
    sprintf(path, "%s/%s", parent, child);
    return path;
}

char * timeAsString(time_t *now) {
    char *buffer = malloc(sizeof(char) * 1024);
    sprintf(buffer, "%s", ctime(now));
    buffer[strlen(buffer) - 1] = 0;
    return buffer;
}

int num_open_files() {
    int fd_counter = 0;
    int max_fd_number = 0;

    struct stat stats;

    int i = 0;
    for (i = 0;i <= max_fd_number;i++) {
        fstat(i, &stats);

        if (errno != EBADF) fd_counter++;
    }

    return fd_counter;
}

/**
 * Scan the contents of a folder and all sub-folders recursively, deleting
 * any file older than the max lifetime specified. Returns whether or not
 * the folder is empty afterwards.
 */
int clean_folder(char * folderpath, int maxlifetime, time_t now, int *files_deleted) {
    DIR *dir;
    struct dirent *dp;

    struct stat statbuffer;

    int can_delete = 1; // The result of the function

    if ((dir = opendir(folderpath)) == NULL) {
        perror(folderpath);
        return 2;
    }

    do {
        // Loop through folder contents
        if ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

            can_delete = 0;

            char *memberpath = childPath(folderpath, dp->d_name);

            // Get file stats
            if (stat(memberpath, &statbuffer) == -1) {
                perror(memberpath);
            } else {
                time_t mt = statbuffer.st_ctimespec.tv_sec;
                time_t diff = (now - mt) / 3600;

                if (dp->d_type == DT_DIR) {
                    int is_empty = clean_folder(memberpath, maxlifetime, now, files_deleted);

                    if (is_empty && diff >= maxlifetime) {
                        if (*files_deleted == 0) {
                            printf("\n");
                            *files_deleted = 1;
                        }

                        if (rmdir(memberpath) == -1)
                            perror(memberpath);
                        else
                            printf("[ DELETED ] %s\n", memberpath);
                    }

                } else {
                    if (diff >= maxlifetime) {
                        if (*files_deleted == 0) {
                            printf("\n");
                            *files_deleted = 1;
                        }

                        if (unlink(memberpath) == -1)
                            perror(memberpath);
                        else
                            printf("[ DELETED ] %s\n", memberpath);
                    }
                }
            }

            free(memberpath);
        }
    } while (dp != NULL);

    if (closedir(dir) == -1) {
        perror("Error closing directory");
    }

    return can_delete;
}

void truncateOutputs(char *out, char *err) {
    printf("Truncating outputs\n");
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        perror("Error truncating output");
        return;
    } else if (pid == 0) {
        int tmp = open(out, O_WRONLY | O_CREAT, 0600);
        close(STDOUT_FILENO);
        dup2(tmp, STDOUT_FILENO);
        char *argv[5] = { "tail", "-n", "50", out, NULL };
        execvp(argv[0], argv);
        exit(0);
    }

    waitpid(pid, NULL, 0);

    pid = fork();

    if (pid == -1) {
        perror("Error truncating error output");
        return;
    } else if (pid == 0) {
        int tmp = open(out, O_WRONLY | O_CREAT, 0600);
        close(STDERR_FILENO);
        dup2(tmp, STDERR_FILENO);
        char *argv[5] = { "tail", "-n", "50", err, NULL };
        execvp(argv[0], argv);
        exit(0);
    }

    waitpid(pid, NULL, 0);
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Invalid use of the sts command.\n");
        fprintf(stderr, "Proper use: ./sts <dirpath> <max hours>\n");
        fprintf(stderr, "        or: ./sts <dirpath> <max hours> <output file> <err file>\n");
        return 0;
    }

    int hasDirectedOutput = (argc == 5);

    if (hasDirectedOutput) {
        int errfd, outfd;

        if ((errfd = open(argv[4], O_CREAT | O_RDWR, 0600)) == -1) {
            perror("Error opening error file");
            return 1;
        }

        if ((outfd = open(argv[3], O_CREAT | O_RDWR, 0600)) == -1) {
            perror("Error opening output file");
            return 1;
        }

        lseek(outfd, 0, SEEK_END);
        lseek(errfd, 0, SEEK_END);

        close(STDERR_FILENO);
        close(STDOUT_FILENO);
        dup2(errfd, STDERR_FILENO);
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
        close(errfd);
    }

    char *filepath = cleanPath(argv[1]);
    int maxlifetime = atoi(argv[2]);

    printf("(STS) Short-term-storage set up on folder at '%s'. Max age allowed: %d hour(s).\n", filepath, maxlifetime);

    int sleepMinutes = 30;
    int firstRun = 1;
    int files_deleted;

    while (1) {

        files_deleted = 0;

        // Sleep until next run
        if (!firstRun) {
            if (hasDirectedOutput) {
                fflush(stdout);
                fflush(stderr);
            }

            sleep(sleepMinutes * 60);
        }

        // Indicate that we are no longer in the first run
        firstRun = 0;

        time_t now = time(NULL);
        char *runtime= timeAsString(&now);
        printf("(STS) Running at %s", runtime);
        free(runtime);

        // Clean the folder of any files too old
        int status = clean_folder(filepath, maxlifetime, now, &files_deleted);

        // If the folder does not exist, report an error and wait two hours
        if (status == 2) {
            perror("(STS) Unable to find specified folder");
            sleepMinutes = 120;
            continue;
        }

        if (files_deleted == 0)
            printf(" -> Done\n");

        // Truncate output and error files at 100 lines
        // if (hasDirectedOutput) {
        //     truncateOutputs(argv[3], argv[4]);
        // }

        // Otherwise, wait 30 minutes
        sleepMinutes = 30;
    }

    free(filepath);

    return 0;
}