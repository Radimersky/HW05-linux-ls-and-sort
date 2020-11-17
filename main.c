#define _GNU_SOURCE
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <zconf.h>
#include <malloc.h>
#include <stdlib.h>
#include <pwd.h>
#include <getopt.h>
#include <grp.h>
#include <errno.h>
#include "common.h"


#define A 2
#define a 1

struct option longopts[] = {
    { "show", optional_argument, 0, 's' },
    { "a", optional_argument, 0, 'a' },
    { "A", optional_argument, 0, 'A' },
    { "all", optional_argument, 0, 'x' },
    { "almost-all", optional_argument, 0, 'y' }
};

int main(int argc, char **argv)
{
    int fileCount = 0;                 //Amount of files in cwd
    struct dirent *direntArray = NULL; // Dynamic memory that store all files in cwd

    long size;
    char *buf;
    char *cwd;                                                // Current working directory
    size = pathconf(".", _PC_PATH_MAX);                       // Get maximum length of path
    if ((buf = calloc((size_t)size, sizeof(char))) != NULL) { // Allocate buffer
        cwd = getcwd(buf, (size_t)size);                      // Get cwd
    } else {
        fprintf(stderr, "%s", "Failed to allocate file path\n");
        exit(-1);
    }

    int ch, indexptr = 0;
    char showArgs[10];
    for (int l = 0; l < 10; ++l) { // Init showArgs
        showArgs[l] = '\0';
    }
    opterr = 0;
    int mode = 0;
    while ((ch = getopt_long(argc, argv, "s::aAx::y::", longopts, &indexptr)) != -1) { //Parse input parameters
        switch (ch) {
        case 's':
            strcpy(showArgs, optarg);
            break;
        case 'a':
            mode = 1;
            break;
        case 'A':
            mode = 2;
            break;
        case 'x':
            mode = 1;
            break;
        case 'y':
            mode = 2;
            break;
        case '?':
            printf("invalid option -- '%c'\n", (char)optopt);
            break;
        default:
            exit(0);
        }
    }

    if (strpbrk(showArgs, "NI") == NULL) { // If input parameter does not contain N or I
        strncpy(showArgs, "RLUGSMN", 7);
    }

    /** Store files returned by readdir in dynamic array  */
    DIR *myDir;
    if ((myDir = opendir(cwd)) == NULL) {
        printf("cannot open directory %s\n", strerror(errno));
        exit(-1);
    }

    int memSize = 0;
    struct dirent *myEnt;

    while ((myEnt = readdir(myDir))) { // Read files from cwd
        if (fileCount == memSize) {    // Reallocate memory if needed
            memSize += (memSize + 1) * 2;
            if ((direntArray = realloc(direntArray, memSize * sizeof(struct dirent))) == NULL) {
                printf("%s\n", "reallocation of dirent array failed");
                free(buf); // Free buffer
                exit(-1);
            }
        }
        if (mode) {
            if (mode == a) { // Get all files
                memcpy(&direntArray[fileCount], myEnt, sizeof(struct dirent));
                fileCount++;
            } else if (mode == A) { // Get all files except . and ..
                if (strcmp(myEnt->d_name, ".") != 0) {
                    if (strcmp(myEnt->d_name, "..") != 0) {
                        memcpy(&direntArray[fileCount], myEnt, sizeof(struct dirent));
                        fileCount++;
                    }
                }
            }
        } else { // Get not hidden files
            if (myEnt->d_name[0] != '.') {
                if (strcmp(myEnt->d_name, ".") != 0) {
                    if (strcmp(myEnt->d_name, "..") != 0) {
                        memcpy(&direntArray[fileCount], myEnt, sizeof(struct dirent));
                        fileCount++;
                    }
                }
            }
        }
    }
    if ((closedir(myDir)) == -1) { // Reading from directory is no longer needed
        printf("cannot close directory %s\n", strerror(errno));
        free(buf);         // Free buffer
        free(direntArray); //Free files
        exit(-1);
    }
    /** */

    struct fileInfo fileInfoArr[fileCount];
    fillFileInfo(fileInfoArr, fileCount, direntArray); // Fully fills array of fileInfo with name of file and file statistics

    qsort((void *)fileInfoArr, (size_t)fileCount, sizeof(struct fileInfo), compare); // Sort files with comparator compare

    double maxLenOfHrdLnk = 1;
    double maxLenOfOwner = 0;
    double maxLenOfGroup = 0;
    double maxLenOfFileSize = 1;
    double maxLenOfFileName = 0;
    double maxLenOfInode = 0;

    for (int n = 0; n < fileCount; ++n) { // Find biggest number of hard links
        if (fileInfoArr[n].statistics->st_nlink > maxLenOfHrdLnk) {
            maxLenOfHrdLnk = fileInfoArr[n].statistics->st_nlink;
        }
    }
    maxLenOfHrdLnk = numPlaces((int)maxLenOfHrdLnk); // Get length of largest number

    size_t ownerLen;
    struct passwd *pwent;
    for (int j = 0; j < fileCount; ++j) { // Find longest user ID
        if ((pwent = getpwuid(fileInfoArr[j].statistics->st_uid)) == NULL) {
            printf("getpwuid() failed %s\n", strerror(errno));
        }
        if ((ownerLen = strlen(pwent->pw_name)) > maxLenOfOwner) {
            maxLenOfOwner = ownerLen;
        }
    }

    size_t groupLen;
    struct group *grp;
    for (int j = 0; j < fileCount; ++j) { // Find longest group ID
        if ((grp = getgrgid(fileInfoArr[j].statistics->st_gid)) == NULL) {
            fprintf(stderr, "getgrnam: %d failed\n", fileInfoArr[j].statistics->st_gid);
        } else if ((groupLen = strlen(grp->gr_name)) > maxLenOfGroup) {
            maxLenOfGroup = groupLen;
        }
    }

    for (int n = 0; n < fileCount; ++n) { // Find biggest number of file size
        if (fileInfoArr[n].statistics->st_size > maxLenOfFileSize) {
            maxLenOfFileSize = fileInfoArr[n].statistics->st_size;
        }
    }
    maxLenOfFileSize = numPlaces((int)maxLenOfFileSize); // Get length of largest file size

    for (int j = 0; j < fileCount; ++j) { // Find longest file name
        if (strlen(fileInfoArr[j].name) > maxLenOfFileName) {
            maxLenOfFileName = strlen(fileInfoArr[j].name);
        }
    }

    for (int n = 0; n < fileCount; ++n) { // Find biggest number of inode
        if (fileInfoArr[n].statistics->st_ino > maxLenOfInode) {
            maxLenOfInode = fileInfoArr[n].statistics->st_ino;
        }
    }
    maxLenOfInode = numPlaces((int)maxLenOfInode); // Get length of largest inode
    int i;
    for (int m = 0; m < fileCount; ++m) {
        i = 0;
        while (showArgs[i] != '\0') {
            switch (showArgs[i]) {
            case 'R':
                printAccRights(fileInfoArr[m].statistics->st_mode);
                break;
            case 'L':
                printHardLinks(fileInfoArr[m].statistics->st_nlink, (int)maxLenOfHrdLnk);
                break;
            case 'U':
                printUserName(fileInfoArr[m].statistics->st_uid, maxLenOfOwner);
                break;
            case 'G':
                printGroupName(fileInfoArr[m].statistics->st_gid, maxLenOfGroup);
                break;
            case 'S':
                printSize(fileInfoArr[m].statistics->st_size, (int)maxLenOfFileSize);
                break;
            case 'M':
                printLastMod(fileInfoArr[m].statistics->st_mtime);
                break;
            case 'N':
                printFileName(fileInfoArr[m].name, (int)maxLenOfFileName);
                break;
            case 'I':
                printInode(fileInfoArr[m].statistics->st_ino, (int)maxLenOfInode);
                break;
            default:
                break;
            }

            if (showArgs[i + 1] != '\0') { // Do not put space when printing last collum
                putchar(' ');
            }
            i++;
        }
        if (m != fileCount - 1) { // Do not put new line when printing last line
            putchar('\n');
        }
    }

    for (int k = 0; k < fileCount; ++k) { //Free statistics from fileInfo struct
        free(fileInfoArr[k].statistics);
    }
    free(buf);         // Free buffer
    free(direntArray); //Free files
}
