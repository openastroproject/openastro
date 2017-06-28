#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cwchar>
#include <cstring>
#include <fcntl.h>
#include <stdlib.h>
//#include <stdio.h>
#if HAVE_SYS_SENDFILE_H
#include <sys/sendfile.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "pipp_utf8.h"

// 64-bit fseek for various platforms
#if !HAVE_FOPEN64
#define fopen64 fopen
#endif


// ------------------------------------------
// fopen_utf8
// ------------------------------------------
FILE *fopen_utf8(
    const char *filename,
    const char *mode)
{
    // Open file, Linux supports UTF-8 filenames by default
    FILE *ret = fopen64(filename, mode);
    return ret;
}


// ------------------------------------------
// rename_utf8
// ------------------------------------------
int rename_utf8( 
   const char *oldname, 
   const char *newname)
{
    // Do the rename, Linux supports UTF-8 filenames by default
    int ret = rename (oldname, newname);
    return ret;
}


// ------------------------------------------
// remove_utf8
// ------------------------------------------
int remove_utf8( 
   const char *path)
{
    // Remove file
    int ret = remove(path);
    return ret;
}


// ------------------------------------------
// copy_file_utf8
// ------------------------------------------
bool copy_file_utf8(
   const char *oldname, 
   const char *newname)
{
    int read_fd;
    int write_fd;
    struct stat stat_buf;
#if HAVE_SYS_SENDFILE_H
    off_t offset = 0;
#else
    void* buffer;
    off_t bytes_left;
    off_t transfer;
    int   ret = 0;

    if (!( buffer = malloc ( 16384 ))) {
        return false;
    }
#endif

    // Open the input file
    read_fd = open(oldname, O_RDONLY);
    if (read_fd < 0) {
        // Source file did not open
        return false;
    }

    // Stat the input file to obtain its size
    fstat(read_fd, &stat_buf);

    // Open the output file for writing, with the same permissions as the source file
    write_fd = open(newname, O_WRONLY | O_CREAT, stat_buf.st_mode);

#if HAVE_SYS_SENDFILE_H
    // Blast the bytes from one file to the other
    ssize_t ret = sendfile(write_fd, read_fd, &offset, stat_buf.st_size);
#else

    bytes_left = stat_buf.st_size;
    while ( bytes_left ) {
      transfer = ( bytes_left > 16384 ) ? 16384 : bytes_left;
      if (( ret = read ( read_fd, buffer, transfer )) != transfer ) {
        break;
      }
      if (( ret = write ( write_fd, buffer, transfer )) != transfer ) {
        break;
      }
      bytes_left -= transfer;
    }

#endif

    // Close up
    close(read_fd);
    close(write_fd);

    return ret >= 0;	
}


// ------------------------------------------
// is_dirctory_utf8
// ------------------------------------------
bool is_directory_utf8(
    const char *path)
{
    struct stat myStat;
    if ((stat(path, &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR)) {
        // path exists and is a directory
        return true;
    }

    return false;
}


// ------------------------------------------
// create_directories_utf8
// ------------------------------------------
bool create_directories_utf8(
    const char *path)
{
    int ret = mkdir(path, 0777);
    return ret == 0;
}


// ------------------------------------------
// pipp_get_filename_from_filepath
// ------------------------------------------
const char *pipp_get_filename_from_filepath(
    const char *path)
{
    // Find last occurance of '/'
    const char *name = strrchr(path, '/');

    if (name == NULL) {
        // '/' was not found, look for '\\' instead
        name = strrchr(path, '\\');
    }

    if (name == NULL) {
        // Neither '/' or '\\' was found - there is no path to remove
        name = path;
    }

    // Filepath has been removed, move past character
    name++;

    return name;
}

