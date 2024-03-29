#ifndef HMDBXML_DEF_H
#define HMDBXML_DEF_H

#define HMDBXML_FILENAME_SUFFIX    ".xml"
#define HMDBXML_HEADER             "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define HMDBXML_LIST_ENTRY_BEGIN    "<metabolite>"
#define HMDBXML_LIST_ENTRY_END      "</metabolite>"
#define HMDBXML_LIST_ENTRY_ID_BEGIN "<accession>"
#define HMDBXML_LIST_ENTRY_ID_END   "</accession>"

#define HMDBXML_ENTRY_NAME_METABOLITE   "metabolite"

#define HMDBXML_ENTRY_PROP_ID           "accession"
#define HMDBXML_ENTRY_PROP_NAME         "name"
#define HMDBXML_ENTRY_PROP_FORMULAR     "chemical_formula"
#define HMDBXML_ENTRY_PROP_AVGMASS      "average_molecular_weight"
#define HMDBXML_ENTRY_PROP_MONOMASS     "monisotopic_molecular_weight"
#define HMDBXML_ENTRY_PROP_STATUS       "status"
#define HMDBXML_ENTRY_PROP_SMILES       "smiles"
#define HMDBXML_ENTRY_PROP_INCHI        "inchi"
#define HMDBXML_ENTRY_PROP_INCHIKEY     "inchikey"
#define HMDBXML_ENTRY_PROP_STATE        "state"
#define HMDBXML_ENTRY_PROP_SPECTRA      "spectra"
#define HMDBXML_ENTRY_PROP_SPECTRUM     "spectrum"
#define HMDBXML_ENTRY_PROP_SPECTRUMID   "spectrum_id"

#define HMDBXML_ENTRY_VALUE_MSMS        "Specdb::MsMs"

#endif // HMDBXML_DEF_H
