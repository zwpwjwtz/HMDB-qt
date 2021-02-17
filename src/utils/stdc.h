#ifndef IO_H
#define IO_H

#if defined(__cplusplus)
extern "C" {
#endif


// Find the position of the first occurance of a substring in string
int utils_strpos(const char* haystack, const char* needle);

// Like strstr, but search up to LENGTH bytes
int utils_strnstr(const char* haystack, const char* needle, int length);

// Like strstr, but search from the end of the HAYSTACK
char* utils_rstrstr(const char* haystack, const char* needle);

// Like isspace, applicable to a whole string
int utils_isspace(const char* str, int length);

// Like strncpy, with a "\0" appended to the destination
char* utils_strncpy (char* dest, const char* src, int count);

// Like tolower(), but deal with a string
char* utils_tolower(const char* str, int length);

// Like fseek, seek forward until a DELIMITER appears in the stream
// Return the number of bytes skipped.
int utils_fseekstr(const char* delimiter, FILE* stream);

// Like getdelim, read up to (and including) a DELIMITER
// from STREAM into *LINEPTR. The DELIMITER can be a multi-char string.
// If *N > 0, the maximum length of string read from STREAM is *N;
// otherwise, its length is unlimited (except by the file size).
int utils_getdelim(char** __restrict lineptr,
                   int* __restrict n,
                   const char* __restrict delimiter,
                   FILE* __restrict stream);

// Read N chars from STREAM, the go back to where we were
// before the read
int utils_fpeek(FILE* stream, char* buffer, int n);

// Like Uconfig_freadCmp, but go back to where we were before the read
int utils_fpeekCmp(FILE* stream, const char* string, int n);

#if defined(__cplusplus)
}
#endif

#endif // IO_H
