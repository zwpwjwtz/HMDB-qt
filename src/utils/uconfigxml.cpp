#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "uconfigxml.h"
#include "uconfigxml_p.h"
#include "stdc.h"

#define UCONFIG_METADATA_KEY_FILENAME   "filename"
#define UCONFIG_METADATA_KEY_FILETYPE   "filetype"
#define UCONFIG_METADATA_VALUE_XML      "xml"

// Chars: for stream parsing only
#define UCONFIG_IO_XML_CHAR_TAG_BEGIN           '<'
#define UCONFIG_IO_XML_CHAR_TAG_MID             '/'
#define UCONFIG_IO_XML_CHAR_TAG_END             '>'
#define UCONFIG_IO_XML_CHAR_KEY_DEFINITION      '='
#define UCONFIG_IO_XML_CHAR_STRING              '"'
#define UCONFIG_IO_XML_CHAR_STRING2             '\''

// Delimitors as strings
#define UCONFIG_IO_XML_DELIMITER_NEWLINE        "\n"
#define UCONFIG_IO_XML_DELIMITER_KEYVAL         "="
#define UCONFIG_IO_XML_DELIMITER_ATTRIBUTE      " "
#define UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN  "<"
#define UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE "</"
#define UCONFIG_IO_XML_DELIMITER_TAG_END        ">"
#define UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF    "/>"
#define UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN  "<!--"
#define UCONFIG_IO_XML_DELIMITER_COMMENT_END    "-->"
#define UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN    "<![CDATA["
#define UCONFIG_IO_XML_DELIMITER_CDATA_END      "]]>"
#define UCONFIG_IO_XML_DELIMITER_XML_BEGIN      "<?xml "
#define UCONFIG_IO_XML_DELIMITER_XML_END        "?>"
#define UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN  "<!DOCTYPE "
#define UCONFIG_IO_XML_DELIMITER_DOCTYPE_END    ">"

#define UCONFIG_IO_XML_BUFFER_MAX               1024



bool UconfigXML::readUconfig(const char* filename, UconfigFile* config)
{
    return readUconfig(filename, config, true);
}

bool UconfigXML::writeUconfig(const char* filename, UconfigFile* config)
{
    return writeUconfig(filename, config, true);
}

bool UconfigXML::readUconfig(const char* filename,
                             UconfigFile* config,
                             bool skipBlankTextNode)
{
    if (!config)
        return false;

    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile)
        return false;

    bool success = UconfigXMLPrivate::freadEntry(inputFile,
                                                 config->rootEntry,
                                                 false,
                                                 skipBlankTextNode) > 0;

    if (success)
    {
        config->rootEntry.setType(UconfigXML::NormalEntry);

        // Add meta-data
        UconfigKeyObject tempKey;
        /* Basic information */
        tempKey.reset();
        tempKey.setName(UCONFIG_METADATA_KEY_FILENAME);
        tempKey.setType(ValueType::Chars);
        tempKey.setValue(filename, strlen(filename) + 1);
        config->metadata.addKey(&tempKey);
        tempKey.reset();
        tempKey.setName(UCONFIG_METADATA_KEY_FILETYPE);
        tempKey.setType(ValueType::Chars);
        tempKey.setValue(UCONFIG_METADATA_VALUE_XML,
                         strlen(UCONFIG_METADATA_VALUE_XML) + 1);
        config->metadata.addKey(&tempKey);
    }

    fclose(inputFile);
    return success;
}

bool UconfigXML::writeUconfig(const char* filename,
                              UconfigFile* config,
                              bool forceQuotingValue)
{
    if (!config)
        return false;

    FILE* outputFile = fopen(filename, "w");
    if (!outputFile)
        return false;

    bool success = true;
    if (config->rootEntry.subentryCount() > 0)
    {
        UconfigEntryObject* entryList = config->rootEntry.subentries();
        for (int i=0; i<config->rootEntry.subentryCount(); i++)
        {
            success &=
                UconfigXMLPrivate::fwriteEntry(outputFile, entryList[i],
                                               0, forceQuotingValue) > 0;
        }
        delete[] entryList;
    }

    fclose(outputFile);
    return success;
}


// Parse the value of an attribute from a given expression.
// Normally, all values are wrapped in a pair of quotes and
// they are seen as strings.
// Here we try to parse values that are not delimited by quotes,
// and assign it a specific value type.
bool UconfigXMLKey::parseValue(const char* expression, int length)
{
    if (!expression)
        return false;

    char* pTail;

    ValueType valueType = guessValueType(expression, length);
    switch (valueType)
    {
        case ValueType::Chars:
        {
            // Ignore wrapping quotes when storing
            setValue(&expression[1], length - sizeof(char) * 2);
            break;
        }
        case ValueType::Integer:
        {
            // Store the number as an "int"
            int tempInt = int(strtol(expression, &pTail, 0));
            if (pTail > expression)
                setValue((char*)(&tempInt), sizeof(int));
            break;
        }
        case ValueType::Float:
        case ValueType::Double:
        {
            // Store the number as a "double"
            double tempDouble = strtod(expression, &pTail);
            if (pTail > expression)
                setValue((char*)(&tempDouble), sizeof(double));
            break;
        }
        default:
            setValue(expression, length);
    }

    setType(valueType);
    return true;
}

int UconfigXMLKey::fwriteValue(FILE* file, bool forceWrappingQuotes)
{
    int length = 0;
    char buffer[UCONFIG_IO_XML_BUFFER_MAX];

    if (ValueType(type()) == ValueType::Chars)
        forceWrappingQuotes = true;

    if (forceWrappingQuotes)
    {
        // Put an opening quote if required
        fputc(UCONFIG_IO_XML_CHAR_STRING, file);
        length++;
    }

    switch (ValueType(type()))
    {
        case ValueType::Integer:
            length = snprintf(buffer, UCONFIG_IO_XML_BUFFER_MAX, "%d", *(int*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        case ValueType::Float:
            length = snprintf(buffer, UCONFIG_IO_XML_BUFFER_MAX, "%f", *(float*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        case ValueType::Double:
            length = snprintf(buffer, UCONFIG_IO_XML_BUFFER_MAX, "%f", *(double*)(value()));
            fwrite(buffer, length, sizeof(char), file);
            break;
        case ValueType::Chars:
        default:
            length = valueSize();
            fwrite(value(), length, sizeof(char), file);
    }

    if (forceWrappingQuotes)
    {
        // Put a closing quote if required
        fputc(UCONFIG_IO_XML_CHAR_STRING, file);
        length++;
    }

    return length;
}


int UconfigXMLPrivate::freadEntry(FILE* file,
                                  UconfigEntryObject& entry,
                                  bool inTag,
                                  bool skipBlankTextNode)
{
    int retValue;
    int parsedLen = 0;
    bool nextKey = false;
    bool preOpening = false;
    bool preClosing = false;
    bool closing = false;
    bool parsingString = false;
    char bufferChar;
    char lastStringDelimitor = '\0';
    std::vector<char> buffer, keyName;
    UconfigXMLKey tempKey;
    UconfigEntryObject tempSubentry;

    entry.reset();
    while (true)
    {
        // Read from file char by char
        retValue = fgetc(file);
        if (retValue == -1)
        {
            // EOF
            break;
        }

        parsedLen++;
        bufferChar = char(retValue);
        if (parsingString)
        {
            buffer.push_back(bufferChar);
            if (bufferChar == lastStringDelimitor)
            {
                // Quit string parsing mode
                parsingString = false;
            }
        }
        else if (!inTag)
        {
            if (bufferChar == UCONFIG_IO_XML_CHAR_TAG_BEGIN)
            {
                // Both opening and closing tag are possible
                // We need to read one more char to determine
                preOpening = true;
            }
            else
            {
                // Parsing text entry
                buffer.push_back(bufferChar);
            }
        }
        else
        switch (bufferChar)
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // White spaces: possible delimitors for attributes
                nextKey = true;
                break;
            case UCONFIG_IO_XML_CHAR_STRING:
            case UCONFIG_IO_XML_CHAR_STRING2:
                // Enter string parsing mode
                parsingString = true;
                lastStringDelimitor = bufferChar;
                buffer.push_back(bufferChar);
                break;
            case UCONFIG_IO_XML_CHAR_TAG_BEGIN:
                preOpening = true;
                break;
            case UCONFIG_IO_XML_CHAR_TAG_END:
                nextKey = true;
                preClosing = true;
                break;
            case UCONFIG_IO_XML_CHAR_TAG_MID:
                nextKey = true;
                closing = true;
                break;
            case UCONFIG_IO_XML_CHAR_KEY_DEFINITION:
                keyName = buffer;
                buffer.clear();
                break;
            default:
                buffer.push_back(bufferChar);
        }

        if (preOpening)
        {
            preOpening = false;

            // Store previous read chars (if any) as a text entry
            if (buffer.size() > 0)
            {
                if (!skipBlankTextNode ||
                    !utils_isspace(buffer.data(), buffer.size()))
                {
                    tempKey.setName(NULL);
                    tempKey.setType(ValueType::Raw);
                    tempKey.setValue(buffer.data(), buffer.size());
                    tempSubentry.reset();
                    tempSubentry.setType(UconfigXML::TextEntry);
                    tempSubentry.addKey(&tempKey);
                    entry.appendSubentry(&tempSubentry);
                }
                buffer.clear();
            }

            // Determine the tag's nature from its left part
            fseek(file, -1, SEEK_CUR);
            parsedLen--;

            retValue = parseComment(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseCDATA(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseXMLDelcaration(file, tempSubentry);
            if (retValue <= 0)
                retValue = parseDoctype(file, tempSubentry);

            if (retValue <= 0)
            {
                // Normal element
                // Try to read one more char to determine its nature
                fseek(file, 1, SEEK_CUR);
                parsedLen++;

                retValue = fgetc(file);
                if (retValue == -1)
                {
                    // EOF
                    break;
                }
                parsedLen++;
                bufferChar = char(retValue);

                if (bufferChar == UCONFIG_IO_XML_CHAR_TAG_MID)
                {
                    // Closing tag of an element
                    inTag = true;
                    closing = true;
                }
                else
                {
                    // Subentry: parse it recursively
                    fseek(file, -1, SEEK_CUR);
                    parsedLen--;

                    retValue = freadEntry(file, tempSubentry, true);
                    if (retValue > 0)
                    {
                        tempSubentry.setType(UconfigXML::NormalEntry);
                        entry.appendSubentry(&tempSubentry);
                        parsedLen += retValue;
                    }
                }
            }
            else
            {
                entry.appendSubentry(&tempSubentry);
                parsedLen += retValue;
            }
        }

        if (nextKey)
        {
            nextKey = false;

            if (buffer.size() > 0)
            {
                // Deal with element names, attribute names and values
                if (!entry.name())
                {
                    entry.setName(buffer.data(), buffer.size());
                }
                else if (keyName.size() == 0)
                {
                    keyName = buffer;
                }
                else
                {
                    tempKey.parseValue(buffer.data(), buffer.size());
                    tempKey.setName(keyName.data(), keyName.size());
                    entry.addKey(&tempKey);
                    keyName.clear();
                }

                buffer.clear();
            }
        }

        if (preClosing)
        {
            preClosing = false;
            inTag = false;

            // Deal with the right part of opening/closing tags
            if (closing)
            {
                if (buffer.size() > 0)
                {
                    // Extra syntax check for non self-closing tags:
                    // See if the name in the closing tag matches
                    // that in the opening tag
                    if (strncmp(entry.name(),
                                buffer.data(),
                                entry.nameSize()) != 0)
                    {
                        // Tags not matching; ignored
                        // Further error information should be printed
                    }
                }
                break;
            }
        }
    }

    return parsedLen;
}

bool UconfigXMLPrivate::fwriteEntry(FILE* file,
                                    UconfigEntryObject& entry,
                                    int level,
                                    bool forceQuotingValue)
{
    switch (UconfigXML::EntryType(entry.type()))
    {
        case UconfigXML::NormalEntry:
        {
            // First write the opening tag of the entry
            fwrite(UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_TAG_BEGINOPEN),
                   file);
            fwrite(entry.name(), sizeof(char), entry.nameSize(), file);

            // Then write the attributes (keys)
            if (entry.keyCount() > 0)
            {
                fwrite(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE),
                       file);
                fwriteEntryKeys(file, entry, false, forceQuotingValue);
            }

            // Then write subentries (if any)
            UconfigEntryObject* subentryList = entry.subentries();
            if (subentryList)
            {
                fwrite(UCONFIG_IO_XML_DELIMITER_TAG_END,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_TAG_END),
                       file);

                for (int i=0; i<entry.subentryCount(); i++)
                    fwriteEntry(file, subentryList[i],
                                level + 1, forceQuotingValue);

                // Finally write the closing tag of the entry
                fwrite(UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_TAG_BEGINCLOSE),
                       file);
                fwrite(entry.name(),
                       sizeof(char),
                       entry.nameSize(),
                       file);
                fwrite(UCONFIG_IO_XML_DELIMITER_TAG_END,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_TAG_END),
                       file);

                delete[] subentryList;
            }
            else
            {
                // Use self-closing tag
                fwrite(UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF,
                       sizeof(char),
                       strlen(UCONFIG_IO_XML_DELIMITER_TAG_ENDSELF),
                       file);
            }
            break;
        }
        case UconfigXML::DoctypeEntry:
            fwrite(UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN),
                   file);
            fwriteEntryKeys(file, entry, true);
            fwrite(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END),
                   file);
            break;
        case UconfigXML::XMLDeclEntry:
            fwrite(UCONFIG_IO_XML_DELIMITER_XML_BEGIN,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_XML_BEGIN),
                   file);
            fwriteEntryKeys(file, entry, false, forceQuotingValue);
            fwrite(UCONFIG_IO_XML_DELIMITER_XML_END,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_XML_END),
                   file);
            break;
        case UconfigXML::CommentEntry:
            fwrite(UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN),
                   file);
            fwriteEntryKeys(file, entry, true);
            fwrite(UCONFIG_IO_XML_DELIMITER_COMMENT_END,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END),
                   file);
            break;
        case UconfigXML::CDATAEntry:
            fwrite(UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN,
                   sizeof(char),
                   strlen(UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN),
                   file);
            fwriteEntryKeys(file, entry, true);
            fwrite(UCONFIG_IO_XML_DELIMITER_CDATA_END,
                sizeof(char),
                strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END),
                file);
            break;
        case UconfigXML::TextEntry:
            fwriteEntryKeys(file, entry, true);
            break;
        default:;
    }

    return true;
}

bool UconfigXMLPrivate::fwriteEntryKeys(FILE* file,
                                        UconfigEntryObject& entry,
                                        bool valueOnly,
                                        bool forceQuotingValue)
{
    UconfigXMLKey* keyList = (UconfigXMLKey*)(entry.keys());
    if (!keyList)
        return false;

    const int kvDLength = strlen(UCONFIG_IO_XML_DELIMITER_KEYVAL);
    const int aDLength = strlen(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE);

    for (int i=0; i<entry.keyCount(); i++)
    {
        if (i > 0)
        {
            fwrite(UCONFIG_IO_XML_DELIMITER_ATTRIBUTE,
                       sizeof(char), aDLength, file);
        }

        // Write the name of the attribute if any
        if (keyList[i].name() && !valueOnly)
        {
            fwrite(keyList[i].name(),
                   sizeof(char),
                   keyList[i].nameSize(),
                   file);
            fwrite(UCONFIG_IO_XML_DELIMITER_KEYVAL,
                   sizeof(char), kvDLength, file);
        }

        // Write the value of the attribute
        keyList[i].fwriteValue(file, forceQuotingValue);
    }

    delete[] keyList;
    return true;
}

// Parse an attribute expression of format "NAME=VALUE"
// If COMPLETE is false, then the "NAME=" part can be omitted,
// and the parsed content is stored as key value.
// Return the number of chars(bytes) that have been parsed
int UconfigXMLPrivate::parseTagAttribute(const char* expression,
                                         UconfigKeyObject& key,
                                         int expressionLength,
                                         bool complete)
{
    if (expressionLength <= 0)
        expressionLength = strlen(expression);

    // Try to find the delimitor between NAME and VALUE
    int pos = utils_strpos(expression, UCONFIG_IO_XML_DELIMITER_KEYVAL);
    if (pos + 1 >= expressionLength)
        return 0;

    if (pos <= 0)
    {
        if (complete)
        {
            // Both NAME and VALUE are required: parsing failed
            return 0;
        }
        else
        {
            // Ignore parsing name
            key.reset();
            pos = 0;
        }
    }
    else
    {
        // Extract attribute's name
        char* keyName = new char[pos + 1];
        utils_strncpy(keyName, expression, pos);

        key.reset();
        key.setName(keyName);
        delete[] keyName;

        pos += strlen(UCONFIG_IO_XML_DELIMITER_KEYVAL);
    }

    // Try to find the end position of VALUE
    int pos2 = pos;
    bool end = false;
    bool parsingString = false;
    char lastStringDelimitor = '\0';
    while (pos2 < expressionLength)
    {
        if (parsingString)
        {
            if (expression[pos2] == lastStringDelimitor)
                parsingString = false;
        }
        else
        switch (expression[pos2])
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '/':
            case '>':
                end = true;
                break;
            case '"':
            case '\'':
                parsingString = true;
                lastStringDelimitor = expression[pos2];
                break;
            default:;
        }
        if (end)
            break;

        pos2++;
    }
    // Extract attribute's value
    if (pos2 > pos)
    {
        key.setType(ValueType::Raw);
        key.setValue(&expression[pos], pos2 - pos);
    }

    return pos2 + 1;
}

// Try to parse a comment section from the current reading position
// of the file, with a constraint of maximum parsing length.
// Any parsed comment is stored as a key attached to the entry.
int UconfigXMLPrivate::parseComment(FILE* file,
                                    UconfigEntryObject& entry,
                                    int maxLength)
{
    if (!file)
        return 0;

    // Check the opening part of the comment section
    int readLength = utils_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_COMMENT_BEGIN,
                                      0);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::CommentEntry);

    // Find the closing part of the comment section
    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = utils_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_COMMENT_END,
                                     file);
    if (contentLength > 0)
    {
        // Extract the comment
        UconfigKeyObject key;
        key.setType(ValueType::Raw);
        key.setValue(buffer, contentLength);
        entry.addKey(&key);
        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    // Checking the closing part of the comment section,
    // even we have already searched for it in utils_getdelim().

    if (utils_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_COMMENT_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END)) < 0)
    {
        // Valid closing part
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_COMMENT_END);
    }
    else
    {
        // Premature ending: the comment section is incomplete
        // We shall report this error in the future
    }

    return readLength;
}

// Try to parse a CDATA section from the file.
// Similar to the function UconfigXMLPrivate::parseComment().
// See its comment for detailed explanation
int UconfigXMLPrivate::parseCDATA(FILE* file,
                                  UconfigEntryObject& entry,
                                  int maxLength)
{
    if (!file)
        return 0;

    int readLength = utils_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_CDATA_BEGIN,
                                      0);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::CDATAEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = utils_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_CDATA_END,
                                     file);
    if (contentLength > 0)
    {
        UconfigKeyObject key;
        key.setType(ValueType::Raw);
        key.setValue(buffer, contentLength);
        entry.addKey(&key);
        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (utils_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_CDATA_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_CDATA_END);

    return readLength;
}

// Try to parse a XML declaration section from the file.
// Similar to the function UconfigXMLPrivate::parseComment().
int UconfigXMLPrivate::parseXMLDelcaration(FILE* file,
                                           UconfigEntryObject& entry,
                                           int maxLength)
{
    if (!file)
        return 0;

    int readLength = utils_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_XML_BEGIN,
                                      0);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::XMLDeclEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = utils_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_XML_END,
                                     file);
    if (contentLength > 0)
    {
        // Try to parse attributes in the tag one by one
        int pos = 0;
        int attributeLength;
        UconfigKeyObject key;
        while (pos < contentLength)
        {
            attributeLength = parseTagAttribute(&buffer[pos],
                                                key,
                                                contentLength - pos);
            if (attributeLength > 0)
            {
                entry.addKey(&key);
                pos += attributeLength;
            }
            else
                break;
        }

        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (utils_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_XML_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_XML_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_XML_END);

    return readLength;
}

// Try to parse a DOCTYPE section from the file.
// Similar to the function UconfigXMLPrivate::parseXMLDelcaration().
// See its comment for detailed explanation
int UconfigXMLPrivate::parseDoctype(FILE* file,
                                    UconfigEntryObject& entry,
                                    int maxLength)
{
    if (!file)
        return 0;

    int readLength = utils_fpeekCmp(file,
                                      UCONFIG_IO_XML_DELIMITER_DOCTYPE_BEGIN,
                                      0);
    if (readLength == 0 || (maxLength > 0  && readLength > maxLength))
        return 0;

    fseek(file, readLength, SEEK_CUR);
    entry.reset();
    entry.setType(UconfigXML::DoctypeEntry);

    char* buffer = NULL;
    int contentLength = maxLength > 0 ? maxLength - readLength : 0;
    contentLength = utils_getdelim(&buffer,
                                     &contentLength,
                                     UCONFIG_IO_XML_DELIMITER_DOCTYPE_END,
                                     file);
    if (contentLength > 0)
    {
        int pos = 0;
        int attributeLength;
        UconfigKeyObject key;
        while (pos < contentLength)
        {
            attributeLength = parseTagAttribute(&buffer[pos],
                                                key,
                                                contentLength - pos,
                                                false);
            if (attributeLength > 0)
            {
                entry.addKey(&key);
                pos += attributeLength;
            }
            else
                break;
        }

        readLength += contentLength;
    }
    if (buffer)
        free(buffer);

    if (utils_fpeekCmp(file,
                         UCONFIG_IO_XML_DELIMITER_DOCTYPE_END,
                         -strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END)) < 0)
        readLength += strlen(UCONFIG_IO_XML_DELIMITER_DOCTYPE_END);

    return readLength;
}
