#ifndef IO_H
#define IO_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _IO_FILE FILE;

// Like strstr, but search from the end of the HAYSTACK
char* utils_rstrstr(const char* haystack, const char* needle);

// Like strncpy, with a "\0" appended to the destination
char* utils_strncpy (char* dest, const char* src, int count);

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

#if defined(__cplusplus)
}
#endif

#endif // IO_H
