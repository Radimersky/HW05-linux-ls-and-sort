#include "common.h"

int compare(const void *v1, const void *v2)
{
    const struct fileInfo *p1 = (struct fileInfo *)v1;
    const struct fileInfo *p2 = (struct fileInfo *)v2;

    char *first = p1->name;
    bool isFirstHidden = false;

    char *second = p2->name;
    bool isSecondHidden = false;

    if (first[0] == '.' && first[1] != '\0') { // If is hidden file or folder
        first += sizeof(char);
        isFirstHidden = true;
    }
    if (second[0] == '.' && second[1] != '\0') { // If is hidden file or folder
        second += sizeof(char);
        isSecondHidden = true;
    }

    if (strcmp(first, second) == 0 && isFirstHidden && !isSecondHidden) { //Not hidden has priority over hidden
        return 1;
    }

    if (strcmp(first, second) == 0 && !isFirstHidden && isSecondHidden) { //Not hidden has priority over hidden
        return -1;
    }
    return strcmp(first, second);
}

/**
 * Prints access rights
 * @param info access rights to print
 */
void printAccRights(const mode_t info)
{
    char str[11];
    mode_to_string(info, str);
    printf("%s", str);
}

/**
 * Prints amount of hard links of file
 * @param link to print
 * @param maxLength number of digits that the file with biggest amount of hard links contain
 */
void printHardLinks(const nlink_t link, const int maxLength)
{
    printf("%*zd", maxLength, link);
}

/**
 * Prints user name of file owner
 * @param user to print
 * @param length longest user id that will be printed
 */
void printUserName(const uid_t user, const double length)
{
    struct passwd *pwent;
    if ((pwent = getpwuid(user)) == NULL) {
        printf("getpwuid() failed %s\n", strerror(errno));
        printf("%d%*s", user, (int)length - (int)numPlaces(user), "");
    } else {
        printf("%s%*s", pwent->pw_name, (int)length - (int)strlen(pwent->pw_name), "");
    }
}

/**
 * Prints group name of file
 * @param group to print
 * @param maxLength of group name that will be printed
 */
void printGroupName(const gid_t group, const double maxLength)
{
    assert(maxLength);
    struct group *grp;
    if ((grp = getgrgid(group)) == NULL) {
        fprintf(stderr, "getgrnam: %d failed\n", group);
    } else {
        printf("%s%*s", grp->gr_name, (int)maxLength - (int)strlen(grp->gr_name), "");
    }
}

/**
 * Prints size of file
 * @param size to print
 * @param maxLength amount of digits of largest file size that will be printed
 */
void printSize(const off_t size, const int maxLength)
{
    printf("%*zd", maxLength, size);
}

/**
 * Prints time of last modification
 * @param time to print
 */
void printLastMod(const time_t time)
{
    struct tm lastModTime;
    localtime_r(&time, &lastModTime);
    char buffer[30];
    if ((strftime(buffer, sizeof(buffer), "%b %e %Y %R", &lastModTime)) == 0) {
        printf("%s\n", "strftime() failed");
        exit(-1);
    }
    printf("%s", buffer);
}

/**
 * Prints file name
 * @param name of file to print
 * @param maxLength of file name that will be printed
 */
void printFileName(const char *name, const int maxLength)
{
    printf("%s%*s", name, maxLength - (int)strlen(name), "");
}

/**
 * Prints inode of file
 * @param inode to print
 * @param maxLength of inode that will be printed
 */
void printInode(const ino_t inode, const int maxLength)
{
    printf("%*zd", maxLength, inode);
}

/**
 * Gets amount of digits if number
 * @param n number to examine
 * @return amount of digits of file
 */
double numPlaces(int n)
{
    if (n < 0) {
        n = (n == INT_MIN) ? INT_MAX : -n;
    }
    if (n < 10) {
        return 1;
    }
    if (n < 100) {
        return 2;
    }
    if (n < 1000) {
        return 3;
    }
    if (n < 10000) {
        return 4;
    }
    if (n < 100000) {
        return 5;
    }
    if (n < 1000000) {
        return 6;
    }
    if (n < 10000000) {
        return 7;
    }
    if (n < 100000000) {
        return 8;
    }
    if (n < 1000000000) {
        return 9;
    }

    return 10;
}

/**
 * Fills *fileIndoArr of fileCount length with file name from *direntArray and file stat
 * @param fileInfoArr to fill
 * @param fileCount amount of files in folder
 * @param direntArray to get files from
 */
void fillFileInfo(struct fileInfo *fileInfoArr, const int fileCount, struct dirent *direntArray)
{
    if (fileInfoArr == NULL || fileCount < 0 || direntArray == NULL) {
        printf("Some parameter is wrong in fillFileInfo %s", "");
        exit(-1);
    }
    for (int i = 0; i < fileCount; ++i) {
        if ((fileInfoArr[i].statistics = calloc(1, sizeof(struct stat))) == NULL) {
            printf("%s\n", "calloc() failed");
        }
        fileInfoArr[i].name = direntArray[i].d_name;
        if (lstat(direntArray[i].d_name, fileInfoArr[i].statistics)) {
            printf("stat: %s\nwhile processing file %s\n", strerror(errno), direntArray[i].d_name);
        }
    }
}

/**
 * @brief Converts value of file mode to zero terminated string
 * @param mode file mode
 * @param str buffer to fill with string
 */
void mode_to_string(mode_t mode, char str[11])
{
    strcpy(str, "----------"); // 10 x -
    char *p = str;


    if (S_ISDIR(mode))
        *p = 'd';
    p++;

    // Owner

    if (mode & S_IRUSR)
        *p = 'r';
    p++;
    if (mode & S_IWUSR)
        *p = 'w';
    p++;
    if (mode & S_IXUSR)
        *p = 'x';
    p++;

    // Group

    if (mode & S_IRGRP)
        *p = 'r';
    p++;
    if (mode & S_IWGRP)
        *p = 'w';
    p++;
    if (mode & S_IXGRP)
        *p = 'x';
    p++;

    // Others

    if (mode & S_IROTH)
        *p = 'r';
    p++;
    if (mode & S_IWOTH)
        *p = 'w';
    p++;
    if (mode & S_IXOTH)
        *p = 'x';
    p++;
}