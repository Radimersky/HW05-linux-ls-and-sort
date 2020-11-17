#define _GNU_SOURCE
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <zconf.h>
#include <malloc.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <wchar.h>

struct fileInfo
{
    char *name;
    struct stat *statistics;
};

void mode_to_string(mode_t mode, char str[11]);
void fillFileInfo(struct fileInfo *fileInfoArr, int fileCount, struct dirent *direntArray);
int compare(const void *v1, const void *v2);
void printAccRights(mode_t info);
void printHardLinks(nlink_t link, int maxLength);
double numPlaces(int n);
void printUserName(uid_t user, double maxLength);
void printGroupName(gid_t group, double maxLength);
void printSize(off_t size, int maxLength);
void printLastMod(time_t time);
void printFileName(const char *name, int maxLength);
void printInode(ino_t inode, int maxLength);

