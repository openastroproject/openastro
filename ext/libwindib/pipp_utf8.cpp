#include <Windows.h>
#include <cstdio>
#include <cwchar>
#include <cstring>
#include "pipp_utf8.h"


// ------------------------------------------
// fopen_utf8
// ------------------------------------------
FILE *fopen_utf8(
    const char *filename,
	const char *mode)
{
    // Convert filename from utf-8 to wchat_t
    int length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, 0, 0);
    wchar_t *w_fname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, w_fname, length);

    // Convert mode from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, mode, -1, 0, 0);
    wchar_t *w_mode = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, w_mode, length);

    // Actually open file
    FILE *ret = _wfopen(w_fname, w_mode);

    // Delete string buffers
    delete [] w_fname;
    delete [] w_mode;

    return ret;
}


// ------------------------------------------
// rename_utf8
// ------------------------------------------
int rename_utf8( 
   const char *oldname, 
   const char *newname)
{
    int length;

    // Convert oldname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, oldname, -1, 0, 0);
    wchar_t *w_oldname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, oldname, -1, w_oldname, length);

    // Convert newname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, newname, -1, 0, 0);
    wchar_t *w_newname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, newname, -1, w_newname, length);

    // Do the rename
    int ret = _wrename(w_oldname, w_newname);

    // Delete string buffers
    delete [] w_oldname;
    delete [] w_newname;

    return ret;
}


int remove_utf8( 
   const char *path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path, -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, w_path, length);

    int ret = _wremove(w_path);

    delete [] w_path;

    return ret;
}


bool copy_file_utf8(
   const char *oldname, 
   const char *newname)
{
    int length;

    // Convert oldname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, oldname, -1, 0, 0);
    wchar_t *w_oldname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, oldname, -1, w_oldname, length);

    // Convert newname from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, newname, -1, 0, 0);
    wchar_t *w_newname = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, newname, -1, w_newname, length);

    int ret = CopyFileW(
        w_oldname,
        w_newname,
        false);

    // Delete string buffers
    delete [] w_oldname;
    delete [] w_newname;

    return ret != 0;
}



bool is_directory_utf8(
    const char *path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path, -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, w_path, length);

    DWORD ret = GetFileAttributesW(w_path);
    delete [] w_path;

    if (ret == INVALID_FILE_ATTRIBUTES) {
        // Nothing exists with this path
        return false;
    }

    if (ret & FILE_ATTRIBUTE_DIRECTORY) {
        // Path exists and it is a directory
        return true;
    }

    return false;// Path is not a directory
}



bool create_directories_utf8(
    const char *path)
{
    int length;

    // Convert path from utf-8 to wchat_t
    length = MultiByteToWideChar(CP_UTF8, 0, path, -1, 0, 0);
    wchar_t *w_path = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, w_path, length);

    int ret = CreateDirectoryW(
      w_path,
      NULL
    );

    delete [] w_path;

    return ret != 0;
}


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
