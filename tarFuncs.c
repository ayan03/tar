#include "mytar.h"

int create_tar(int fd, char *basepath, int verb, mytar_t **archive) {
    DIR *d;
    struct dirent *dir;
    struct stat buf;
    char path[PATH_LIMIT];
    char clean_path[PATH_LIMIT];
    struct passwd *pws;
    struct group *grp;
    int jump = 148;
    int rfd = 0;
    int buf_size = 0;
    char boof[BLOCKSIZE] = { 0 };

    /* Base case */
    if (!(d = opendir(basepath))) {
        return 1;
    }
    /* Read through each file in directory */
    while ((dir = readdir(d)) != NULL) {
        /* Recurse through but not for "." and ".." */
        if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            /* Add paths to pathname */
            strcpy(path, basepath);
            strcat(path, "/");
            strcat(path, dir->d_name);

            if ((lstat(path, &buf)) == -1) {
                perror("Error: ");
            }

            *archive = fill_metadata(*archive, buf, path);
            *archive = fill_block(*archive);

            /* Set condition value */
            *archive = get_checksum(*archive);
            memcpy((*archive)->block + jump, (*archive)->chksum, COMMON_SIZE);

            /* Write the header portion */
            write(fd, (*archive)->block, BLOCKSIZE);
    
            /* Write the contents of the file if there is content */
            if ((rfd = open(path, O_RDONLY)) == -1) {
                perror("Error: ");
                return 1;
            }    
    
            while ((buf_size = read(rfd, boof, BLOCKSIZE)) > 0) {
                write(fd, boof, BLOCKSIZE);
                memset(boof, 0, BLOCKSIZE);
            }

            close(rfd);

            /* Check for symlinks */
            if (!S_ISLNK(buf.st_mode)) { 
                create_tar(fd, path, verb, archive);
            }
        }
    }
    closedir(d);
    return 0;
}
int first_dir(int fd, char *path, int verb, mytar_t **archive) {
    struct stat buf;
    int i = 0;
    int jump = 148;
    int rfd = 0;
    int buf_size = 0;
    char boof[BLOCKSIZE] = { 0 };
    /* Get info on first directory */
    if (lstat(path, &buf) == -1) {
        fprintf(stderr, "Unable to stat: %s\n", path);
        return 1;
    }
    /* Fill the data of the archive */
    *archive = fill_metadata(*archive, buf, path);
    strcat((*archive)->name, "/");
    *archive = fill_block(*archive);

    /* Set condition value */
    *archive = get_checksum(*archive);
    memcpy((*archive)->block + jump, (*archive)->chksum, COMMON_SIZE);

    /* Write the header portion */
    write(fd, (*archive)->block, BLOCKSIZE);
    
    /* Write the contents of the file if there is content */
    if ((rfd = open(path, O_RDONLY)) == -1) {
        perror("Error: ");
        return 1;
    }    
    
    while ((buf_size = read(rfd, boof, BLOCKSIZE)) > 0) {
        write(fd, boof, BLOCKSIZE);
        memset(boof, 0, BLOCKSIZE);
    }
    
    close(rfd);
    return 0;
}

int create_regfile(int fd, char *path, int verb, mytar_t **archive) {
    struct stat buf;
    int i = 0;
    int jump = 148;
    int rfd = 0;
    int buf_size = 0;
    char boof[BLOCKSIZE] = { 0 };

    if ((lstat(path, &buf)) == -1) {
        perror("Error: ");
        return 1;
    }
    /* Clear everything in the archive */
    *archive = fill_metadata(*archive, buf, path);
    *archive = fill_block(*archive);
    /* Set condition value */
    *archive = get_checksum(*archive);
    memcpy((*archive)->block + jump, (*archive)->chksum, COMMON_SIZE);

    /* Write the header portion */
    write(fd, (*archive)->block, BLOCKSIZE);
    
    /* Write the contents of the file if there is content */
    if ((rfd = open(path, O_RDONLY)) == -1) {
        perror("Error: ");
        return 1;
    }    
    
    while ((buf_size = read(rfd, boof, BLOCKSIZE)) > 0) {
        write(fd, boof, BLOCKSIZE);
        memset(boof, 0, BLOCKSIZE);
    }
    
    close(rfd);
    
    return 0;
}

mytar_t *fill_metadata(mytar_t *archive, struct stat buf, char *path) {
    struct passwd *pws;
    struct group *grp; 
    int i = 0;
    int path_size = strlen(path);
    int tot_size = 100; /* Size of the name char array */

    /* Path fits perfectly into name field */
    if (path_size == tot_size) {
        /* Clear path name */
        memset(archive->name, 0, FILENAME_SIZE);
        memcpy(archive->name, path, FILENAME_SIZE);
    }
    /* Path fits inside name field */
    else if (path_size < tot_size) {
        /* Clear path name */
        memset(archive->name, 0, FILENAME_SIZE);
        memcpy(archive->name, path, path_size);
        memset(archive->name + FILENAME_SIZE - 1,'\0', 1); 
    }
    /* Path overflows into prefix */
    
    else {  
        memset(archive->name, 0, FILENAME_SIZE);
        memset(archive->prefix, 0 , PREFIX_SIZE);
        strncpy(archive->name, path, tot_size);
        strcpy(archive->prefix, path + tot_size);
        if (path_size < PATH_LIMIT) {
            strcat(archive->prefix, "\0");
        }
    }

    sprintf((archive)->mode, "%07o", buf.st_mode & MODE_MASK);
    sprintf((archive)->uid, "%07o", buf.st_uid);           
    sprintf((archive)->gid, "%07o", buf.st_gid);           
    sprintf((archive)->size, "%011o", (int) buf.st_size);
    sprintf((archive)->mtime, "%011o", (int) buf.st_mtime);
    strcpy((archive)->magic, "ustar\0");
    strcpy((archive)->version, "00");

    /* User name and group name */
    pws = getpwuid(buf.st_uid);
    strcpy((archive)->uname, pws->pw_name);
    grp = getgrgid(buf.st_gid);

    strcpy((archive)->gname, grp->gr_name);

    /* Set devmajor and devminor to null */
    memset((archive)->devmajor, 0, COMMON_SIZE);
    memset((archive)->devminor, 0, COMMON_SIZE);
    /*
    sprintf((archive)->devmajor, "%08o", '\0');
    sprintf((archive)->devminor, "%08o", '\0');
    */
    
            

    /* Symlink */
    if (S_ISLNK(buf.st_mode)) {
        sprintf((archive)->size, "%011o", 0);
        (archive)->typeflag[0] = SYMLINK;
        if (readlink(path, archive->linkname, tot_size) < 0) {
            perror("Error: ");
        }
    }
    /* Directory */
    else if (S_ISDIR(buf.st_mode)) {
        sprintf((archive)->size, "%011o", 0);
        (archive)->typeflag[0] = DIRECTORY;
    }
 
    /* File */
    else {
        (archive)->typeflag[0] = REGFILE;
    }
    return archive;
}

mytar_t *fill_block(mytar_t *archive) {
    /* Perhaps the most inefficient filling a header block */
    int i = 0;
    int header_block = 500;
    int jump = 0;
    char chksum_buf[COMMON_SIZE];
    char null_buf[SIZE_SIZE] = { 0 };
    
    /* Set checksum to all spaces */
    for (i = 0; i < COMMON_SIZE; i++) {
        chksum_buf[i] = ' ';
    }
    
    /* Memcpy information into the block attribute */
    memcpy(archive->block, archive->name, FILENAME_SIZE);
    jump += FILENAME_SIZE;
    memcpy(archive->block + jump, archive->mode, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->uid, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->gid, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->size, SIZE_SIZE);
    jump += SIZE_SIZE;
    memcpy(archive->block + jump, archive->mtime, SIZE_SIZE);
    jump += SIZE_SIZE;
    memcpy(archive->block + jump, chksum_buf, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->typeflag, TYPEFLAG_SIZE);
    jump += TYPEFLAG_SIZE;
    memcpy(archive->block + jump, archive->linkname, FILENAME_SIZE);
    jump += FILENAME_SIZE;
    memcpy(archive->block + jump, archive->magic, MAGIC_SIZE);
    jump += MAGIC_SIZE;
    memcpy(archive->block + jump, archive->version, VERSION_SIZE);
    jump += VERSION_SIZE;
    memcpy(archive->block + jump, archive->uname, GROUPNAME_SIZE);
    jump += GROUPNAME_SIZE;
    memcpy(archive->block + jump, archive->gname, GROUPNAME_SIZE);
    jump += GROUPNAME_SIZE;
    memcpy(archive->block + jump, archive->devmajor, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->devminor, COMMON_SIZE);
    jump += COMMON_SIZE;
    memcpy(archive->block + jump, archive->prefix, PREFIX_SIZE);
    jump += PREFIX_SIZE;
    memcpy(archive->block + jump, null_buf, SIZE_SIZE);


    return archive;
}

/* Not sure if this function is necessary */
mytar_t *zeroout(mytar_t *archive) {
    char zeroed[200] = { 0 };
    int size_100 = 100;
    int size_8 = 8;
    int size_32 = 32;
    int size_6 = 6;
    int size_12 = 12;
    int size_155 = 155;

    strncpy(archive->name, zeroed, size_100);
    strncpy(archive->mode, zeroed, size_8);
    strncpy(archive->uid, zeroed, size_8);
    strncpy(archive->gid, zeroed, size_8);
    strncpy(archive->size, zeroed, size_12);
    strncpy(archive->mtime, zeroed, size_12);
    strncpy(archive->chksum, zeroed, size_8);
    strncpy(archive->linkname, zeroed, size_100);
    strncpy(archive->uname, zeroed, size_32);
    strncpy(archive->gname, zeroed, size_32);
    strncpy(archive->devmajor, zeroed, size_8);
    strncpy(archive->devminor, zeroed, size_8);
    strncpy(archive->prefix, zeroed, size_155);
    return archive;
}

mytar_t *get_checksum(mytar_t *archive) {
    int i = 0;
    unsigned int check = 0;
    for (i = 0; i < 500; i++) {
        check += (unsigned char) archive->block[i];
    }
    sprintf(archive->chksum, "%.7o", check);
    return archive;
}

mytar_t *make_header() {
    mytar_t *archive = NULL;
    archive = calloc(1, sizeof(mytar_t));
    if (archive == NULL) {
        perror("Error: ");
    }
    return archive;
}


void print_metadata(mytar_t *archive) {
    printf("Name: %s\n", archive->name);
    printf("Mode: %s\n", archive->mode);
    printf("Uid: %s\n", archive->uid);
    printf("Gid: %s\n", archive->gid);
    printf("Size: %s\n", archive->size);
    printf("Mtime: %s\n", archive->mtime);
   /* printf("Chksum: %s\n", archive->chksum);*/
    printf("Typeflag: %s\n", archive->typeflag);
    printf("Linkname: %s\n", archive->linkname);
    printf("Magic: %s\n", archive->magic);
    printf("Version: %s\n", archive->version);
    printf("Username: %s\n", archive->uname);
    printf("Group Name: %s\n", archive->gname);
    printf("Devmajor: %s\n", archive->devmajor);
    printf("Devminor: %s\n", archive->devminor);
    printf("Prefix: %s\n", archive->prefix);
    printf("\n");
}
