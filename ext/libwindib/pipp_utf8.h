#ifndef PIPP_UTF8_H
#define PIPP_UTF8_H

#include <cstdio>

FILE *fopen_utf8(
    const char *filename,
	const char *mode);


int rename_utf8( 
   const char *oldname, 
   const char *newname);


int remove_utf8( 
   const char *path);


bool copy_file_utf8(
   const char *oldname, 
   const char *newname);


bool is_directory_utf8(
    const char *path);

bool create_directories_utf8(
    const char *path);
    
const char *pipp_get_filename_from_filepath(
    const char *path);

#endif  // PIPP_UTF8_H
