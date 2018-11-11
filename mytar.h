#ifndef __MYTAR__
#define __MYTAR__

/* Header files */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

/* Size defines */
#define PATH_LIMIT 256
#define BLOCKSIZE 512
#define MAGIC_SIZE 6
#define COMMON_SIZE 8
#define SIZE_SIZE 12
#define FILENAME_SIZE 100
#define GROUPNAME_SIZE 32
#define PREFIX_SIZE 155
#define VERSION_SIZE 2
#define TYPEFLAG_SIZE 1

/* File type factors */
#define REGFILE '0'
#define ALTFILE 0
#define SYMLINK '2'
#define DIRECTORY '5'

/* Other defines */
#define MODE_MASK 0777

typedef struct mytar_t {
    /* Header format */
    char name[FILENAME_SIZE];
    char mode[COMMON_SIZE];
    char uid[COMMON_SIZE];
    char gid[COMMON_SIZE];
    char size[SIZE_SIZE];
    char mtime[SIZE_SIZE];
    char chksum[COMMON_SIZE];
    char typeflag[TYPEFLAG_SIZE];
    char linkname[FILENAME_SIZE];
    char magic[MAGIC_SIZE];
    char version[VERSION_SIZE];
    char uname[GROUPNAME_SIZE];
    char gname[GROUPNAME_SIZE];
    char devmajor[COMMON_SIZE];
    char devminor[COMMON_SIZE];
    char prefix[PREFIX_SIZE];

    char block[BLOCKSIZE];
} mytar_t;

/* If given a directory */
int create_tar(int fd, char *pathname, int verb, mytar_t **archive);

/* If given a file */
int create_regfile(int fd, char *pathname, int verb, mytar_t **archive);

/* Similar to create reg file but for the first directory */
int first_dir(int fd, char *pathname, int verb, mytar_t **archive);

/* Fill the header data with stuff from info from lstat */
mytar_t *fill_metadata(mytar_t *archive, struct stat buf, char *path);

/* Fill the block with the metadata */
mytar_t *fill_block(mytar_t *archive);

/* Not sure if needed */
mytar_t *zeroout(mytar_t *archive);

/* Calculate the checksum */
mytar_t *get_checksum(mytar_t *archive);

/* To check the output */
void print_metadata(mytar_t *archive);

/* Not necessary since I removed the linked list attribute */
void free_archive(mytar_t *archive);









#endif /* __MYTAR__ */
