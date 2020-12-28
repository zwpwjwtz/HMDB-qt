#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdc.h"

#define UTILS_FILE_BUFFER_MAX   1024


char* utils_rstrstr(const char* haystack, const char* needle)
{
    unsigned long hLength = strlen(haystack);
    unsigned long nLength = strlen(needle);
    if (nLength > hLength)
        return NULL;

    for (const char* p = haystack + hLength - nLength; p >= haystack; p--)
    {
        if (strncmp(p, needle, nLength))
            return p;
    }
    return NULL;
}

char* utils_strncpy (char* dest, const char* src, int count)
{
    dest[count] = '\0';
    return strncpy(dest, src, count);
}

int utils_fseekstr(const char* delimiter, FILE* stream)
{
    long count = 0;

    int readLength;
    const long delimiterLength = strlen(delimiter);
    const long seekLength = UTILS_FILE_BUFFER_MAX + delimiterLength;
    char* buffer = malloc(seekLength * sizeof(char));
    char* pos;
    while (1)
    {
        readLength = fread(buffer, 1, seekLength, stream);
        if (readLength <= 0)
            break;

        pos = strstr(buffer, delimiter);
        if (pos)
        {
            count += pos - buffer;
            fseek(stream, pos - buffer + delimiterLength - readLength, SEEK_CUR);
            break;
        }

        fseek(stream, -delimiterLength, SEEK_CUR);
        count += readLength - delimiterLength;
    }
    free(buffer);

    if (readLength > 0)
        return count;
}

int utils_getdelim(char** lineptr, int* n,
                   const char* delimiter, FILE* stream)
{
    int bufferSize;

    // See if the size limit is valid
    if (n != NULL && *n > 0)
        bufferSize = *n;
    else
        bufferSize = UTILS_FILE_BUFFER_MAX;

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

    long readLength;
    const long delimiterLength = strlen(delimiter);
    const long seekLength = UTILS_FILE_BUFFER_MAX + delimiterLength;
    char* buffer = malloc(seekLength * sizeof(char));
    char* pos;

    while (1)
    {
        readLength = fread(buffer, 1, seekLength, stream);
        if (readLength <= 0 || feof(stream))
            break;

        pos = strstr(buffer, delimiter);
        if (pos)
        {
            // See if we need to increase the size of the buffer
            if (p1 + (pos - buffer) + 1 >= p2)
            {
                if (n != NULL && *n > 0)
                {
                    // Size limited by caller; stop reading
                    strncpy(p1, buffer, p2 - p1 - 1);
                    *p2 = '\0';
                    return p2 - *lineptr;
                }

                newBufferSize = bufferSize * 2;
                newBuffer = (char*)(realloc(*lineptr, newBufferSize));
                if (newBuffer == NULL)
                    return -1;
                p1 += newBuffer - *lineptr;
                p2 = newBuffer + newBufferSize;
                *lineptr = newBuffer;
                bufferSize = newBufferSize;
            }
            strncpy(p1, buffer, pos - buffer);
            p1 += pos - buffer;
            fseek(stream, pos - buffer - readLength, SEEK_CUR);
            break;
        }

        // See if we need to increase the size of the buffer
        if (p1 + readLength - delimiterLength + 1 >= p2)
        {
            if (n != NULL && *n > 0)
            {
                // Size limited by caller; stop reading
                strncpy(p1, buffer, p2 - p1 - 1);
                *p2 = '\0';
                return p2 - *lineptr;
            }

            newBufferSize = bufferSize * 2;
            newBuffer = (char*)(realloc(*lineptr, newBufferSize));
            if (newBuffer == NULL)
                return -1;
            p1 += newBuffer - *lineptr;
            p2 = newBuffer + newBufferSize;
            *lineptr = newBuffer;
            bufferSize = newBufferSize;
        }
        strncpy(p1, buffer, readLength - delimiterLength);
        p1 += readLength - delimiterLength;
        fseek(stream, -delimiterLength, SEEK_CUR);
    }

    if (readLength <= 0)
        return -1;
    else
        return p1 == *lineptr ? -1 : p1 - *lineptr;
}
