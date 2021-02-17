#ifndef HMDB_MSMS_XML_DEF_H
#define HMDB_MSMS_XML_DEF_H

#define HMDB_MSMS_XML_FILENAME_ID_SEP    "_"
#define HMDB_MSMS_XML_FILENAME_ID_SEP2   "_ms_ms_spectrum_"
#define HMDB_MSMS_XML_FILENAME_END1      "_experimental"
#define HMDB_MSMS_XML_FILENAME_END2      "_predicted"
#define HMDB_MSMS_XML_FILENAME_SUFFIX    ".xml"
#define HMDB_MSMS_XML_ENTRY_BEGIN        "<ms-ms>"
#define HMDB_MSMS_XML_ENTRY_END          "</ms-ms>"

#define HMDB_MSMS_XML_ENTRY_ID_BEGIN             "<database-id>"
#define HMDB_MSMS_XML_ENTRY_ID_END               "</database-id>"
#define HMDB_MSMS_XML_ENTRY_SPECTRUM_ID_BEGIN    "<id>"
#define HMDB_MSMS_XML_ENTRY_SPECTRUM_ID_END      "</id>"

#define HMDB_MSMS_XML_ENTRY_POLARITY_BEGIN       "<ionization-mode>"
#define HMDB_MSMS_XML_ENTRY_POLARITY_END         "</ionization-mode>"
#define HMDB_MSMS_XML_ENTRY_POLARITY_POSITIVE    "positive"
#define HMDB_MSMS_XML_ENTRY_POLARITY_NEGATIVE    "negative"

#define HMDB_MSMS_XML_ENTRY_MZ_BEGIN     "<mass-charge>"
#define HMDB_MSMS_XML_ENTRY_MZ_END       "</mass-charge>"
#define HMDB_MSMS_XML_ENTRY_INT_BEGIN    "<intensity>"
#define HMDB_MSMS_XML_ENTRY_INT_END      "</intensity>"


#endif // HMDB_MSMS_XML_DEF_H
