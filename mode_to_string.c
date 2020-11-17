#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

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

