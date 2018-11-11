    struct passwd *pws;
    struct group *grp; 

    /* Fill header information in */ /* Mtime and Size issues */
    sprintf((archive)->mode, "%07o", buf.st_mode);
    sprintf((archive)->uid, "%07o", buf.st_uid);           
    sprintf((archive)->gid, "%07o", buf.st_gid);           
    strcpy((archive)->magic, "ustar\0");
    strcpy((archive)->version, "00");
    sprintf((archive)->devmajor, "%08o", major(buf.st_dev));
    sprintf((archive)->devminor, "%08o", minor(buf.st_dev));
            
    /* User name and group name */
    pws = getpwuid(buf.st_uid);
    strcpy((archive)->uname, pws->pw_name);
    grp = getgrgid(buf.st_gid);
    strcpy((archive)->gname, grp->gr_name);

    /* Symlink */
    if (S_ISLNK(buf.st_mode)) {
        sprintf((archive)->size, "%011o", 0);
        (archive)->typeflag[0] = SYMLINK;
    }
    /* Directory */
    else if (S_ISDIR(buf.st_mode)) {
        sprintf((archive)->size, "%011o", 0);
        (archive)->typeflag[0] = DIRECTORY;
    }
 
    /* File */
    else {
        /* OR REG ALT FILE FIX */
        (archive)->typeflag[0] = REGFILE;
    }
