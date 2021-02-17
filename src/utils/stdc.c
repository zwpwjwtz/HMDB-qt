#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdc.h"

#define FALSE                   0
#define TRUE                    1
#define UTILS_FILE_BUFFER_LEN   1024


int utils_strpos(const char* haystack, const char* needle)
{
    const char* pos = strstr(haystack, needle);
    if (pos)
        return (int)(pos - haystack);
    else
        return -1;
}

int utils_strnstr(const char* haystack, const char* needle, int length)
{
    const char* pos = strstr(haystack, needle);
    if (pos - haystack + strlen(needle) <= length)
        return pos;
    else
        return NULL;
}

char* utils_rstrstr(const char* haystack, const char* needle)
{
    unsigned long hLength = strlen(haystack);
    unsigned long nLength = strlen(needle);
    if (nLength > hLength)
        return NULL;

    const char* p;
    for (p = haystack + hLength - nLength; p >= haystack; p--)
    {
        if (strncmp(p, needle, nLength) == 0)
            return p;
    }
    return NULL;
}

int utils_isspace(const char* str, int length)
{
    while (length > 0)
    {
        if (*str != ' ' || *str != '\0' ||
            *str != '\t' || *str != '\v'  ||
            *str != '\n' || *str != '\r' || *str != '\f')
            break;
        str++;
        length--;
    }
    return length == 0;
}

char* utils_strncpy (char* dest, const char* src, int count)
{
    char* pos = strncpy(dest, src, count);
    dest[count] = '\0';
    return pos;
}

char* utils_tolower(const char* str, int length)
{
    char* dest = malloc(sizeof(char) * length);
    if (!dest)
        return NULL;

    char* p = dest;
    while (length > 0)
    {
        *(p++) = tolower(*(str++));
        length--;
    }

    return dest;
}

int utils_fseekstr(const char* delimiter, FILE* stream)
{
    long count = 0;

    int readLength;
    const long delimiterLength = strlen(delimiter);
    const long seekLength = UTILS_FILE_BUFFER_LEN + delimiterLength;
    char* buffer = malloc(seekLength * sizeof(char));
    char* pos;
    while (TRUE)
    {
        readLength = fread(buffer, 1, seekLength, stream);
        if (readLength <= 0)
            break;

        pos = utils_strnstr(buffer, delimiter, readLength);
        if (pos)
        {
            count += pos - buffer;
            fseek(stream, pos-buffer - readLength + delimiterLength, SEEK_CUR);
            break;
        }

        if (feof(stream))
            break;

        fseek(stream, -delimiterLength, SEEK_CUR);
        count += readLength - delimiterLength;
    }
    free(buffer);

    if (readLength > 0)
        return count;
    else
    {
        fseek(stream, -count, SEEK_CUR);
        return 0;
    }
}

int utils_getdelim(char** lineptr, int* n,
                   const char* delimiter, FILE* stream)
{
    int bufferSize;

    // See if the size limit is valid
    if (n != NULL && *n > 0)
        bufferSize = *n;
    else
        bufferSize = UTILS_FILE_BUFFER_LEN;

    // Reallocating memory for the buffer
    if (*lineptr == NULL)
    {
        *lineptr = (char*)(malloc(bufferSize));
        if (*lineptr == NULL)
            return -1;
    }

    char* p1 = *lineptr;
    char* p2 = *lineptr + bufferSize;
    char* newBuffer;
    int newBufferSize;
    int allocationFailed = FALSE;

    long readLength;
    const long delimiterLength = strlen(delimiter);
    const long seekLength = UTILS_FILE_BUFFER_LEN + delimiterLength;
    char* buffer = malloc(seekLength * sizeof(char));
    char* pos;

    while (TRUE)
    {
        readLength = fread(buffer, 1, seekLength, stream);
        if (readLength <= 0)
            break;

        pos = utils_strnstr(buffer, delimiter, readLength);
        if (pos)
        {
            // See if we need to increase the size of the buffer
            if (p1 + (pos - buffer) + 1 >= p2)
            {
                if (n != NULL && *n > 0)
                {
                    // Size limited by caller; stop reading
                    utils_strncpy(p1, buffer, p2 - p1 - 1);
                    p1 = p2;
                    break;
                }

                newBufferSize = bufferSize * 2;
                newBuffer = (char*)(realloc(*lineptr, newBufferSize));
                if (newBuffer == NULL)
                {
                    allocationFailed = TRUE;
                    break;
                }
                p1 += newBuffer - *lineptr;
                p2 = newBuffer + newBufferSize;
                *lineptr = newBuffer;
                bufferSize = newBufferSize;
            }
            utils_strncpy(p1, buffer, pos - buffer);
            p1 += pos - buffer;
            fseek(stream, pos - buffer + delimiterLength - readLength,
                  SEEK_CUR);
            break;
        }

        if (feof(stream))
            break;

        // See if we need to increase the size of the buffer
        if (p1 + readLength - delimiterLength + 1 >= p2)
        {
            if (n != NULL && *n > 0)
            {
                // Size limited by caller; stop reading
                utils_strncpy(p1, buffer, p2 - p1 - 1);
                p1 = p2;
                break;
            }

            newBufferSize = bufferSize * 2;
            newBuffer = (char*)(realloc(*lineptr, newBufferSize));
            if (newBuffer == NULL)
            {
                readLength = 0;
                break;
            }
            p1 += newBuffer - *lineptr;
            p2 = newBuffer + newBufferSize;
            *lineptr = newBuffer;
            bufferSize = newBufferSize;
        }
        utils_strncpy(p1, buffer, readLength - delimiterLength);
        p1 += readLength - delimiterLength;
        fseek(stream, -delimiterLength, SEEK_CUR);

    }
    free(buffer);

    if (allocationFailed)
        return -1;
    else
        return p1 - *lineptr;
}

int utils_fpeek(FILE* stream, char* buffer, int n)
{
    int readLength;
    if (n >= 0)
    {
        readLength = fread(buffer, sizeof(char), n, stream);
        fseek(stream, -readLength, SEEK_CUR);
    }
    else
    {
        long oldPos = ftell(stream);
        fseek(stream, n, SEEK_CUR);
        long newPos = ftell(stream);
        readLength = -fread(buffer, sizeof(char), oldPos - newPos, stream);
    }
    return readLength;
}

int utils_fpeekCmp(FILE *stream, const char* string, int n)
{
    if (n == 0)
    {
        n = strlen(string);
        if (n == 0)
            return 0;
    }

    // Read stream by customized fpeek():
    // the size parameter and the return value could be negative
    char* buffer = malloc(sizeof(char) * abs(n));
    int readLength = utils_fpeek(stream, buffer, n);
    if (strncmp(buffer, string, abs(n)) != 0)
        readLength = 0;

    free(buffer);
    return readLength;
}
