/**
 * \file frame.cpp
 * Generalized frame.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Aug 2007
 *
 * Copyright (C) 2007-2018  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "frame.h"
#include <QMap>
#include <QStringList>
#include <QRegularExpression>
#include <QCoreApplication>
#include "pictureframe.h"

namespace {

const char* const fieldIdNames[] = {
  "Unknown",
  QT_TRANSLATE_NOOP("@default", "Text Encoding"),
  QT_TRANSLATE_NOOP("@default", "Text"),
  QT_TRANSLATE_NOOP("@default", "URL"),
  QT_TRANSLATE_NOOP("@default", "Data"),
  QT_TRANSLATE_NOOP("@default", "Description"),
  QT_TRANSLATE_NOOP("@default", "Owner"),
  QT_TRANSLATE_NOOP("@default", "Email"),
  QT_TRANSLATE_NOOP("@default", "Rating"),
  QT_TRANSLATE_NOOP("@default", "Filename"),
  QT_TRANSLATE_NOOP("@default", "Language"),
  QT_TRANSLATE_NOOP("@default", "Picture Type"),
  QT_TRANSLATE_NOOP("@default", "Image format"),
  QT_TRANSLATE_NOOP("@default", "Mimetype"),
  QT_TRANSLATE_NOOP("@default", "Counter"),
  QT_TRANSLATE_NOOP("@default", "Identifier"),
  QT_TRANSLATE_NOOP("@default", "Volume Adjustment"),
  QT_TRANSLATE_NOOP("@default", "Number of Bits"),
  QT_TRANSLATE_NOOP("@default", "Volume Change Right"),
  QT_TRANSLATE_NOOP("@default", "Volume Change Left"),
  QT_TRANSLATE_NOOP("@default", "Peak Volume Right"),
  QT_TRANSLATE_NOOP("@default", "Peak Volume Left"),
  QT_TRANSLATE_NOOP("@default", "Timestamp Format"),
  QT_TRANSLATE_NOOP("@default", "Content Type"),

  QT_TRANSLATE_NOOP("@default", "Price"),
  QT_TRANSLATE_NOOP("@default", "Date"),
  QT_TRANSLATE_NOOP("@default", "Seller"),
  nullptr
};

const char* const textEncodingNames[] = {
  QT_TRANSLATE_NOOP("@default", "ISO-8859-1"),
  QT_TRANSLATE_NOOP("@default", "UTF16"),
  QT_TRANSLATE_NOOP("@default", "UTF16BE"),
  QT_TRANSLATE_NOOP("@default", "UTF8"),
  nullptr
};

const char* const timestampFormatNames[] = {
  QT_TRANSLATE_NOOP("@default", "Other"),
  QT_TRANSLATE_NOOP("@default", "MPEG frames as unit"),
  QT_TRANSLATE_NOOP("@default", "Milliseconds as unit"),
  nullptr
};

const char* const contentTypeNames[] = {
  QT_TRANSLATE_NOOP("@default", "Other"),
  QT_TRANSLATE_NOOP("@default", "Lyrics"),
  QT_TRANSLATE_NOOP("@default", "Text transcription"),
  QT_TRANSLATE_NOOP("@default", "Movement/part name"),
  QT_TRANSLATE_NOOP("@default", "Events"),
  QT_TRANSLATE_NOOP("@default", "Chord"),
  QT_TRANSLATE_NOOP("@default", "Trivia/pop up"),
  nullptr
};


/**
 * Get name of frame from type.
 *
 * @param type type
 *
 * @return name.
 */
const char* getNameFromType(Frame::Type type)
{
  static const char* const names[] = {
    QT_TRANSLATE_NOOP("@default", "Title"),           // FT_Title,
    QT_TRANSLATE_NOOP("@default", "Artist"),          // FT_Artist,
    QT_TRANSLATE_NOOP("@default", "Album"),           // FT_Album,
    QT_TRANSLATE_NOOP("@default", "Comment"),         // FT_Comment,
    QT_TRANSLATE_NOOP("@default", "Date"),            // FT_Date,
    QT_TRANSLATE_NOOP("@default", "Track Number"),    // FT_Track,
    QT_TRANSLATE_NOOP("@default", "Genre"),           // FT_Genre,
                                  // FT_LastV1Frame = FT_Track,
    QT_TRANSLATE_NOOP("@default", "Album Artist"),    // FT_AlbumArtist
    QT_TRANSLATE_NOOP("@default", "Arranger"),        // FT_Arranger,
    QT_TRANSLATE_NOOP("@default", "Author"),          // FT_Author,
    QT_TRANSLATE_NOOP("@default", "BPM"),             // FT_Bpm,
    QT_TRANSLATE_NOOP("@default", "Catalog Number"),  // FT_CatalogNumber,
    QT_TRANSLATE_NOOP("@default", "Compilation"),     // FT_Compilation,
    QT_TRANSLATE_NOOP("@default", "Composer"),        // FT_Composer,
    QT_TRANSLATE_NOOP("@default", "Conductor"),       // FT_Conductor,
    QT_TRANSLATE_NOOP("@default", "Copyright"),       // FT_Copyright,
    QT_TRANSLATE_NOOP("@default", "Disc Number"),     // FT_Disc,
    QT_TRANSLATE_NOOP("@default", "Encoded-by"),      // FT_EncodedBy,
    QT_TRANSLATE_NOOP("@default", "Encoder Settings"), // FT_EncoderSettings,
    QT_TRANSLATE_NOOP("@default", "Encoding Time"),   // FT_EncodingTime,
    QT_TRANSLATE_NOOP("@default", "Grouping"),        // FT_Grouping,
    QT_TRANSLATE_NOOP("@default", "Initial Key"),     // FT_InitialKey,
    QT_TRANSLATE_NOOP("@default", "ISRC"),            // FT_Isrc,
    QT_TRANSLATE_NOOP("@default", "Language"),        // FT_Language,
    QT_TRANSLATE_NOOP("@default", "Lyricist"),        // FT_Lyricist,
    QT_TRANSLATE_NOOP("@default", "Lyrics"),          // FT_Lyrics,
    QT_TRANSLATE_NOOP("@default", "Media"),           // FT_Media,
    QT_TRANSLATE_NOOP("@default", "Mood"),            // FT_Mood,
    QT_TRANSLATE_NOOP("@default", "Original Album"),  // FT_OriginalAlbum,
    QT_TRANSLATE_NOOP("@default", "Original Artist"), // FT_OriginalArtist,
    QT_TRANSLATE_NOOP("@default", "Original Date"),   // FT_OriginalDate,
    QT_TRANSLATE_NOOP("@default", "Description"),     // FT_Description,
    QT_TRANSLATE_NOOP("@default", "Performer"),       // FT_Performer,
    QT_TRANSLATE_NOOP("@default", "Picture"),         // FT_Picture,
    QT_TRANSLATE_NOOP("@default", "Publisher"),       // FT_Publisher,
    QT_TRANSLATE_NOOP("@default", "Release Country"), // FT_ReleaseCountry,
    QT_TRANSLATE_NOOP("@default", "Remixer"),         // FT_Remixer,
    QT_TRANSLATE_NOOP("@default", "Sort Album"),      // FT_SortAlbum,
    QT_TRANSLATE_NOOP("@default", "Sort Album Artist"), // FT_SortAlbumArtist,
    QT_TRANSLATE_NOOP("@default", "Sort Artist"),     // FT_SortArtist,
    QT_TRANSLATE_NOOP("@default", "Sort Composer"),   // FT_SortComposer,
    QT_TRANSLATE_NOOP("@default", "Sort Name"),       // FT_SortName,
    QT_TRANSLATE_NOOP("@default", "Subtitle"),        // FT_Subtitle,
    QT_TRANSLATE_NOOP("@default", "Website"),         // FT_Website,
    QT_TRANSLATE_NOOP("@default", "WWW Audio File"),  // FT_WWWAudioFile,
    QT_TRANSLATE_NOOP("@default", "WWW Audio Source"), // FT_WWWAudioSource,
    QT_TRANSLATE_NOOP("@default", "Release Date"),    // FT_ReleaseDate,
    QT_TRANSLATE_NOOP("@default", "Rating"),          // FT_Rating,
    QT_TRANSLATE_NOOP("@default", "Work")             // FT_Work,
                                  // FT_LastFrame = FT_Work
  };
  Q_STATIC_ASSERT(sizeof(names) / sizeof(names[0]) == Frame::FT_LastFrame + 1);
  return type <= Frame::FT_LastFrame ? names[type] : "Unknown";
}

/**
 * Get map of non unified frame names to display names.
 * @return mapping of frame names to display names.
 */
QMap<QByteArray, QByteArray> getDisplayNamesOfIds()
{
  static const struct StrOfId {
    const char* id;
    const char* str;
  } strOfId[] = {
    { "AENC", QT_TRANSLATE_NOOP("@default", "Audio Encryption") },
    { "ASPI", QT_TRANSLATE_NOOP("@default", "Audio Seek Point") },
    { "CHAP", QT_TRANSLATE_NOOP("@default", "Chapter") },
    { "COMR", QT_TRANSLATE_NOOP("@default", "Commercial") },
    { "CTOC", QT_TRANSLATE_NOOP("@default", "Table of Contents") },
    { "ENCR", QT_TRANSLATE_NOOP("@default", "Encryption Method") },
    { "EQU2", QT_TRANSLATE_NOOP("@default", "Equalization") },
    { "EQUA", QT_TRANSLATE_NOOP("@default", "Equalization") },
    { "ETCO", QT_TRANSLATE_NOOP("@default", "Event Timing Codes") },
    { "GEOB", QT_TRANSLATE_NOOP("@default", "General Object") },
    { "GRID", QT_TRANSLATE_NOOP("@default", "Group Identification") },
    { "GRP1", QT_TRANSLATE_NOOP("@default", "Grouping") },
    { "LINK", QT_TRANSLATE_NOOP("@default", "Linked Information") },
    { "MCDI", QT_TRANSLATE_NOOP("@default", "Music CD Identifier") },
    { "MLLT", QT_TRANSLATE_NOOP("@default", "MPEG Lookup Table") },
    { "MVIN", QT_TRANSLATE_NOOP("@default", "Movement Number") },
    { "MVNM", QT_TRANSLATE_NOOP("@default", "Movement Name") },
    { "OWNE", QT_TRANSLATE_NOOP("@default", "Ownership") },
    { "PCNT", QT_TRANSLATE_NOOP("@default", "Play Counter") },
    { "PCST", QT_TRANSLATE_NOOP("@default", "Podcast") },
    { "POPM", QT_TRANSLATE_NOOP("@default", "Popularimeter") },
    { "POSS", QT_TRANSLATE_NOOP("@default", "Position Synchronisation") },
    { "PRIV", QT_TRANSLATE_NOOP("@default", "Private") },
    { "RBUF", QT_TRANSLATE_NOOP("@default", "Recommended Buffer Size") },
    { "RVA2", QT_TRANSLATE_NOOP("@default", "Volume Adjustment") },
    { "RVAD", QT_TRANSLATE_NOOP("@default", "Volume Adjustment") },
    { "RVRB", QT_TRANSLATE_NOOP("@default", "Reverb") },
    { "SEEK", QT_TRANSLATE_NOOP("@default", "Seek") },
    { "SIGN", QT_TRANSLATE_NOOP("@default", "Signature") },
    { "SYLT", QT_TRANSLATE_NOOP("@default", "Synchronized Lyrics") },
    { "SYTC", QT_TRANSLATE_NOOP("@default", "Synchronized Tempo Codes") },
    { "TCAT", QT_TRANSLATE_NOOP("@default", "Podcast Category") },
    { "TDAT", QT_TRANSLATE_NOOP("@default", "Date") },
    { "TDEN", QT_TRANSLATE_NOOP("@default", "Encoding Time") },
    { "TDES", QT_TRANSLATE_NOOP("@default", "Podcast Description") },
    { "TDLY", QT_TRANSLATE_NOOP("@default", "Playlist Delay") },
    { "TDOR", QT_TRANSLATE_NOOP("@default", "Original Release Time") },
    { "TDRC", QT_TRANSLATE_NOOP("@default", "Recording Time") },
    { "TDRL", QT_TRANSLATE_NOOP("@default", "Release Time") },
    { "TDTG", QT_TRANSLATE_NOOP("@default", "Tagging Time") },
    { "TFLT", QT_TRANSLATE_NOOP("@default", "File Type") },
    { "TGID", QT_TRANSLATE_NOOP("@default", "Podcast Identifier") },
    { "TIME", QT_TRANSLATE_NOOP("@default", "Time") },
    { "TKWD", QT_TRANSLATE_NOOP("@default", "Podcast Keywords") },
    { "TLEN", QT_TRANSLATE_NOOP("@default", "Length") },
    { "TOFN", QT_TRANSLATE_NOOP("@default", "Original Filename") },
    { "TOWN", QT_TRANSLATE_NOOP("@default", "File Owner") },
    { "TPRO", QT_TRANSLATE_NOOP("@default", "Produced Notice") },
    { "TRDA", QT_TRANSLATE_NOOP("@default", "Recording Date") },
    { "TRSN", QT_TRANSLATE_NOOP("@default", "Radio Station Name") },
    { "TRSO", QT_TRANSLATE_NOOP("@default", "Radio Station Owner") },
    { "TSIZ", QT_TRANSLATE_NOOP("@default", "Size") },
    { "TXXX", QT_TRANSLATE_NOOP("@default", "User-defined Text") },
    { "UFID", QT_TRANSLATE_NOOP("@default", "Unique File Identifier") },
    { "USER", QT_TRANSLATE_NOOP("@default", "Terms of Use") },
    { "WCOM", QT_TRANSLATE_NOOP("@default", "Commercial URL") },
    { "WCOP", QT_TRANSLATE_NOOP("@default", "Copyright URL") },
    { "WFED", QT_TRANSLATE_NOOP("@default", "Podcast Feed") },
    { "WORS", QT_TRANSLATE_NOOP("@default", "Official Radio Station") },
    { "WPAY", QT_TRANSLATE_NOOP("@default", "Payment") },
    { "WPUB", QT_TRANSLATE_NOOP("@default", "Official Publisher") },
    { "WXXX", QT_TRANSLATE_NOOP("@default", "User-defined URL") },
    { "BAND", QT_TRANSLATE_NOOP("@default", "Album Artist") },
    { "CONTACT", QT_TRANSLATE_NOOP("@default", "Contact") },
    { "CONTENTGROUP", QT_TRANSLATE_NOOP("@default", "Grouping") },
    { "DESCRIPTION", QT_TRANSLATE_NOOP("@default", "Description") },
    { "DISCTOTAL", QT_TRANSLATE_NOOP("@default", "Total Discs") },
    { "ENCODER", QT_TRANSLATE_NOOP("@default", "Encoder") },
    { "ENCODER_OPTIONS", QT_TRANSLATE_NOOP("@default", "Encoder Settings") },
    { "ENCODEDBY", QT_TRANSLATE_NOOP("@default", "Encoded-by") },
    { "ENCODING", QT_TRANSLATE_NOOP("@default", "Encoding") },
    { "ENGINEER", QT_TRANSLATE_NOOP("@default", "Engineer") },
    { "ENSEMBLE", QT_TRANSLATE_NOOP("@default", "Ensemble") },
    { "GUESTARTIST", QT_TRANSLATE_NOOP("@default", "Guest Artist") },
    { "IsVBR", QT_TRANSLATE_NOOP("@default", "VBR") },
    { "iTunPGAP", QT_TRANSLATE_NOOP("@default", "Gapless Playback") },
    { "LABEL", QT_TRANSLATE_NOOP("@default", "Label") },
    { "LABELNO", QT_TRANSLATE_NOOP("@default", "Label Number") },
    { "LICENSE", QT_TRANSLATE_NOOP("@default", "License") },
    { "LOCATION", QT_TRANSLATE_NOOP("@default", "Location") },
    { "OPUS", QT_TRANSLATE_NOOP("@default", "Opus") },
    { "ORIGARTIST", QT_TRANSLATE_NOOP("@default", "Original Artist") },
    { "ORGANIZATION", QT_TRANSLATE_NOOP("@default", "Organization") },
    { "PARTNUMBER", QT_TRANSLATE_NOOP("@default", "Part Number") },
    { "PRODUCER", QT_TRANSLATE_NOOP("@default", "Producer") },
    { "PRODUCTNUMBER", QT_TRANSLATE_NOOP("@default", "Product Number") },
    { "RECORDINGDATE", QT_TRANSLATE_NOOP("@default", "Recording Date") },
    { "REMIXEDBY", QT_TRANSLATE_NOOP("@default", "Remixer") },
    { "TOTALDISCS", QT_TRANSLATE_NOOP("@default", "Total Discs") },
    { "TOTALTRACKS", QT_TRANSLATE_NOOP("@default", "Total Tracks") },
    { "TRACKTOTAL", QT_TRANSLATE_NOOP("@default", "Total Tracks") },
    { "UNKNOWN", QT_TRANSLATE_NOOP("@default", "Unknown") },
    { "Unknown", QT_TRANSLATE_NOOP("@default", "Unknown") },
    { "VERSION", QT_TRANSLATE_NOOP("@default", "Version") },
    { "VOLUME", QT_TRANSLATE_NOOP("@default", "Volume") },
    { "WWW", QT_TRANSLATE_NOOP("@default", "User-defined URL") },
    { "WM/AlbumArtistSortOrder", QT_TRANSLATE_NOOP("@default", "Sort Album Artist") },
    { "WM/Comments", QT_TRANSLATE_NOOP("@default", "Comment") },
    { "WM/MCDI", QT_TRANSLATE_NOOP("@default", "MCDI") },
    { "WM/Mood", QT_TRANSLATE_NOOP("@default", "Mood") },
    { "WM/OriginalFilename", QT_TRANSLATE_NOOP("@default", "Original Filename") },
    { "WM/OriginalLyricist", QT_TRANSLATE_NOOP("@default", "Original Lyricist") },
    { "WM/PromotionURL", QT_TRANSLATE_NOOP("@default", "Commercial URL") },
    { "WM/SharedUserRating", QT_TRANSLATE_NOOP("@default", "User Rating") },
    { "WM/UserWebURL", QT_TRANSLATE_NOOP("@default", "User-defined URL") },
    { "akID", QT_TRANSLATE_NOOP("@default", "Account Type") },
    { "apID", QT_TRANSLATE_NOOP("@default", "Purchase Account") },
    { "atID", QT_TRANSLATE_NOOP("@default", "Artist ID") },
    { "catg", QT_TRANSLATE_NOOP("@default", "Category") },
    { "cnID", QT_TRANSLATE_NOOP("@default", "Catalog ID") },
    { "cond", QT_TRANSLATE_NOOP("@default", "Conductor") },
    { "desc", QT_TRANSLATE_NOOP("@default", "Description") },
    { "geID", QT_TRANSLATE_NOOP("@default", "Genre ID") },
    { "hdvd", QT_TRANSLATE_NOOP("@default", "HD Video") },
    { "keyw", QT_TRANSLATE_NOOP("@default", "Keyword") },
    { "ldes", QT_TRANSLATE_NOOP("@default", "Long Description") },
    { "pcst", QT_TRANSLATE_NOOP("@default", "Podcast") },
    { "pgap", QT_TRANSLATE_NOOP("@default", "Gapless Playback") },
    { "plID", QT_TRANSLATE_NOOP("@default", "Album ID") },
    { "purd", QT_TRANSLATE_NOOP("@default", "Purchase Date") },
    { "rtng", QT_TRANSLATE_NOOP("@default", "Rating/Advisory") },
    { "sfID", QT_TRANSLATE_NOOP("@default", "Country Code") },
    { "sosn", QT_TRANSLATE_NOOP("@default", "Sort Show") },
    { "stik", QT_TRANSLATE_NOOP("@default", "Media Type") },
    { "tven", QT_TRANSLATE_NOOP("@default", "TV Episode") },
    { "tves", QT_TRANSLATE_NOOP("@default", "TV Episode Number") },
    { "tvnn", QT_TRANSLATE_NOOP("@default", "TV Network Name") },
    { "tvsh", QT_TRANSLATE_NOOP("@default", "TV Show Name") },
    { "tvsn", QT_TRANSLATE_NOOP("@default", "TV Season") },
    { "year", QT_TRANSLATE_NOOP("@default", "Year") },
    { "\251mvn", QT_TRANSLATE_NOOP("@default", "Movement Name") },
    { "\251mvi", QT_TRANSLATE_NOOP("@default", "Movement Number") },
    { "\251mvc", QT_TRANSLATE_NOOP("@default", "Movement Count") },
    { "shwm", QT_TRANSLATE_NOOP("@default", "Show Work & Movement") },
    { "ownr", QT_TRANSLATE_NOOP("@default", "Owner") },
    { "purl", QT_TRANSLATE_NOOP("@default", "Podcast URL") },
    { "egid", QT_TRANSLATE_NOOP("@default", "Podcast GUID") },
    { "cmID", QT_TRANSLATE_NOOP("@default", "Composer ID") },
    { "xid ", QT_TRANSLATE_NOOP("@default", "XID") },
    { "IARL", QT_TRANSLATE_NOOP("@default", "Archival Location") },
    { "ICMS", QT_TRANSLATE_NOOP("@default", "Commissioned") },
    { "ICRP", QT_TRANSLATE_NOOP("@default", "Cropped") },
    { "IDIM", QT_TRANSLATE_NOOP("@default", "Dimensions") },
    { "IDPI", QT_TRANSLATE_NOOP("@default", "Dots Per Inch") },
    { "IKEY", QT_TRANSLATE_NOOP("@default", "Keywords") },
    { "ILGT", QT_TRANSLATE_NOOP("@default", "Lightness") },
    { "IPLT", QT_TRANSLATE_NOOP("@default", "Number of Colors") },
    { "ISBJ", QT_TRANSLATE_NOOP("@default", "Subject") },
    { "ISHP", QT_TRANSLATE_NOOP("@default", "Sharpness") },
    { "ISRF", QT_TRANSLATE_NOOP("@default", "Source Form") }
  };
  static QMap<QByteArray, QByteArray> idStrMap;
  if (idStrMap.isEmpty()) {
    // first time initialization
    for (const auto& soi : strOfId) {
      idStrMap.insert(soi.id, soi.str);
    }
  }
  return idStrMap;
}

/**
 * Get a reduced field list without fields which are only supported by a
 * specific tag format.
 * @param fieldList original field list
 * @return reduced field list.
 */
Frame::FieldList reducedFieldList(const Frame::FieldList& fieldList)
{
  Frame::FieldList reduced;
  for (const Frame::Field& fld : fieldList) {
    if (fld.m_id != Frame::ID_ImageFormat &&
        fld.m_id != Frame::ID_ImageProperties) {
      reduced.append(fld);
    }
  }
  return reduced;
}

}

Frame::ExtendedType::ExtendedType(const QString& name) :
  m_type(getTypeFromName(name)), m_name(name)
{
}

Frame::ExtendedType::ExtendedType(Type type) :
  m_type(type), m_name(QString::fromLatin1(getNameFromType(type)))
{
}

/**
 * Get name of type.
 * @return name.
 */
QString Frame::ExtendedType::getName() const
{
  return m_type != FT_Other ? QString::fromLatin1(getNameFromType(m_type))
                            : m_name;
}

/**
 * Get translated name of type.
 * @return name.
 */
QString Frame::ExtendedType::getTranslatedName() const
{
  return m_type != FT_Other
      ? QCoreApplication::translate("@default", getNameFromType(m_type))
      : m_name;
}


/**
 * Constructor.
 */
Frame::Frame()
  : m_index(-1), m_marked(FrameNotice::None), m_valueChanged(false)
{
}

/**
 * Constructor.
 */
Frame::Frame(Type type, const QString& value,
             const QString& name, int index)
  : m_extendedType(type, name), m_index(index), m_value(value),
    m_marked(FrameNotice::None), m_valueChanged(false)
{
}

/**
 * Constructor.
 * @param type  type and internal name
 * @param value value
 * @param index index inside tag, -1 if unknown
 */
Frame::Frame(const ExtendedType& type, const QString& value, int index)
  : m_extendedType(type), m_index(index), m_value(value),
    m_marked(FrameNotice::None), m_valueChanged(false)
{
}

/**
 * Set the value from a field in the field list.
 */
void Frame::setValueFromFieldList()
{
  if (!getFieldList().empty()) {
    for (auto fldIt = getFieldList().constBegin();
         fldIt != getFieldList().constEnd();
         ++fldIt) {
      int id = (*fldIt).m_id;
      if (id == ID_Text ||
          id == ID_Description ||
          id == ID_Url) {
        m_value = (*fldIt).m_value.toString();
        if (id == ID_Text) {
          // highest priority, will not be overwritten
          break;
        }
      }
    }
  }
}

/**
 * Set a field in the field list from the value.
 */
void Frame::setFieldListFromValue()
{
  if (!fieldList().empty()) {
    auto it = fieldList().end(); // clazy:exclude=detaching-member
    for (auto fldIt = fieldList().begin(); fldIt != fieldList().end(); ++fldIt) { // clazy:exclude=detaching-member
      int id = (*fldIt).m_id;
      if (id == ID_Text ||
          id == ID_Description ||
          id == ID_Url) {
        it = fldIt;
        if (id == ID_Text) {
          // highest priority, will not be overwritten
          break;
        }
      } else if (id == ID_Rating) {
        bool ok;
        int rating = m_value.toInt(&ok);
        if (ok) {
          (*fldIt).m_value = rating;
          break;
        }
      }
    }
    if (it != fieldList().end()) {
      (*it).m_value = m_value;
    }
  }
}

/**
 * Convert string (e.g. "track/total number of tracks") to number.
 *
 * @param str string to convert
 * @param ok  if not 0, true is returned here if conversion is ok
 *
 * @return number in string ignoring total after slash.
 */
int Frame::numberWithoutTotal(const QString& str, bool* ok)
{
  int slashPos = str.indexOf(QLatin1Char('/'));
#if QT_VERSION >= 0x060000
  return slashPos == -1 ? str.toInt(ok) : str.left(slashPos).toInt(ok);
#else
  return slashPos == -1 ? str.toInt(ok) : str.leftRef(slashPos).toInt(ok);
#endif
}

/**
 * Get value as integer.
 * @return value.
 */
int Frame::getValueAsNumber() const
{
  if (isInactive()) {
    return -1;
  } else if (isEmpty()) {
    return 0;
  } else {
    return numberWithoutTotal(m_value);
  }
}

/**
 * Set value as integer.
 * @param n value as number
 */
void Frame::setValueAsNumber(int n)
{
  if (n == -1) {
    m_value = QString();
  } else if (n == 0) {
    m_value = QLatin1String("");
  } else {
    m_value.setNum(n);
  }
}

/**
 * Get the value of a field.
 *
 * @param id field ID
 *
 * @return field value, invalid if field not found.
 */
QVariant Frame::getFieldValue(FieldId id) const
{
  for (auto it = m_fieldList.constBegin(); it != m_fieldList.constEnd(); ++it) {
    if ((*it).m_id == id) {
      return (*it).m_value;
    }
  }
  return QVariant();
}

/**
 * Set value as string and mark it as changed if it is changed.
 * This method will avoid setting "different" representations.
 * @param value value as string
 */
void Frame::setValueIfChanged(const QString& value)
{
  if (value != differentRepresentation()) {
    QString oldValue(getValue());
    if (value != oldValue && !(value.isEmpty() && oldValue.isEmpty())) {
      setValue(value);
      setValueChanged();
    }
  }
}

/**
 * Check if the fields in another frame are equal.
 *
 * @param other other frame
 *
 * @return true if equal.
 */
bool Frame::isEqual(const Frame& other) const
{
  if (getType() != other.getType() || getValue() != other.getValue())
    return false;

  const FieldList& otherFieldList = other.getFieldList();
  if (m_fieldList.size() != otherFieldList.size())
    return false;

  for (auto thisIt = m_fieldList.constBegin(), otherIt = otherFieldList.constBegin();
       thisIt != m_fieldList.constEnd() && otherIt != otherFieldList.constEnd();
       ++thisIt, ++otherIt) {
    if (thisIt->m_id != otherIt->m_id || thisIt->m_value != otherIt->m_value) {
      return false;
    }
  }

  return true;
}

/**
 * Set value of a field.
 *
 * @param frame frame to set
 * @param id    field ID
 * @param value field value
 *
 * @return true if field found and set.
 */
bool Frame::setField(Frame& frame, FieldId id, const QVariant& value)
{
  for (auto it = frame.fieldList().begin(); it != frame.fieldList().end(); ++it) { // clazy:exclude=detaching-member
    if ((*it).m_id == id) {
      (*it).m_value = value;
      if (id == ID_Description) frame.setValue(value.toString());
      return true;
    }
  }
  return false;
}

/**
 * Set value of a field.
 *
 * @param frame frame to set
 * @param fieldName name of field, can be English or translated
 * @param value field value
 *
 * @return true if field found and set.
 */
bool Frame::setField(Frame& frame, const QString& fieldName,
                     const QVariant& value)
{
  const FieldId id = Field::getFieldId(fieldName);
  if (id != ID_NoField) {
#if QT_VERSION >= 0x060000
    QMetaType valueType = value.metaType();
    QMetaType fieldType;
#else
    QVariant::Type valueType = value.type();
    QVariant::Type fieldType;
#endif
    switch (id) {
    case ID_TextEnc:
    case ID_PictureType:
    case ID_Counter:
    case ID_VolumeAdj:
    case ID_NumBits:
    case ID_VolChgRight:
    case ID_VolChgLeft:
    case ID_PeakVolRight:
    case ID_PeakVolLeft:
    case ID_TimestampFormat:
    case ID_ContentType:
#if QT_VERSION >= 0x060000
      fieldType = QMetaType(QMetaType::Int);
#else
      fieldType = QVariant::Int;
#endif
      break;
    case ID_Data:
#if QT_VERSION >= 0x060000
      fieldType = QMetaType(QMetaType::QByteArray);
#else
      fieldType = QVariant::ByteArray;
#endif
      break;
    default:
#if QT_VERSION >= 0x060000
      fieldType = QMetaType(QMetaType::QString);
#else
      fieldType = QVariant::String;
#endif
    }
    if (valueType != fieldType && value.canConvert(fieldType)) {
      QVariant converted(value);
      if (converted.convert(fieldType)) {
        return setField(frame, id, converted);
      }
    }
    return setField(frame, id, value);
  }
  return false;
}

/**
 * Get value of a field.
 *
 * @param frame frame to get
 * @param id    field ID
 *
 * @return field value, invalid if not found.
 */
QVariant Frame::getField(const Frame& frame, FieldId id)
{
  QVariant result;
  if (!frame.getFieldList().empty()) {
    for (auto it = frame.getFieldList().constBegin();
         it != frame.getFieldList().constEnd();
         ++it) {
      if ((*it).m_id == id) {
        result = (*it).m_value;
        break;
      }
    }
  }
  return result;
}

/**
 * Get value of a field.
 *
 * @param frame frame to get
 * @param fieldName name of field, can be English or translated
 *
 * @return field value, invalid if not found.
 */
QVariant Frame::getField(const Frame& frame, const QString& fieldName)
{
  const FieldId id = Field::getFieldId(fieldName);
  return id != ID_NoField ? getField(frame, id) : QVariant();
}

/**
 * Get type of frame from English name.
 *
 * @param name name, spaces and case are ignored
 *
 * @return type.
 */
Frame::Type Frame::getTypeFromName(const QString& name)
{
  static QMap<QString, int> strNumMap;
  if (strNumMap.empty()) {
    // first time initialization
    for (int i = 0; i <= Frame::FT_LastFrame; ++i) {
      auto type = static_cast<Frame::Type>(i);
      strNumMap.insert(QString::fromLatin1(getNameFromType(type))
                       .remove(QLatin1Char(' ')).toUpper(), type);
    }
  }
  QString ucName(name.toUpper());
  ucName.remove(QLatin1Char(' '));
  auto it = strNumMap.constFind(ucName);
  if (it != strNumMap.constEnd()) {
    return static_cast<Frame::Type>(*it);
  }
  return Frame::FT_Other;
}

/**
 * Get a translated string for a frame type.
 *
 * @param type frame type
 *
 * @return frame type, null string if unknown.
 */
QString Frame::getFrameTypeName(Type type)
{
  return QCoreApplication::translate("@default", getNameFromType(type));
}

/**
 * Get a display name for a frame name.
 * @param name frame name as returned by getName()
 * @return display name, transformed if necessary and translated.
 */
QString Frame::getDisplayName(const QString& name)
{
  QMap<QByteArray, QByteArray> idStrMap = getDisplayNamesOfIds();
  if (name.isEmpty())
    return name;

  Type type = getTypeFromName(name);
  if (type != FT_Other)
    return QCoreApplication::translate("@default",
                                       name.toLatin1().constData());

  QString nameStr(name);
  int nlPos = nameStr.indexOf(QLatin1Char('\n'));
  if (nlPos > 0)
    // probably "TXXX - User defined text information\nDescription" or
    // "WXXX - User defined URL link\nDescription"
    nameStr = nameStr.mid(nlPos + 1);

  QByteArray id;
  if (nameStr.mid(4, 3) == QLatin1String(" - ")) {
    id = nameStr.left(4).toLatin1();
  } else {
    id = nameStr.toLatin1();
  }

  auto it = idStrMap.constFind(id);
  if (it != idStrMap.constEnd()) {
    return QCoreApplication::translate("@default", it->constData());
  }
  return nameStr;
}

/**
 * Get a map with display names as keys and frame names as values.
 * @param names frame names as returned by getName()
 * @return mapping of display names to frame names.
 */
QMap<QString, QString> Frame::getDisplayNameMap(const QStringList& names)
{
  QMap<QString, QString> map;
  for (const QString& name : names) {
    map.insert(getDisplayName(name), name);
  }
  return map;
}

/**
 * Get the frame name for a translated display name.
 * @param name translated display name
 * @return English frame name for @a name if found, else @a name.
 */
QString Frame::getNameForTranslatedFrameName(const QString& name)
{
  static QMap<QString, QString> nameMap;
  if (nameMap.isEmpty()) {
    // first time initialization
    for (int k = Frame::FT_FirstFrame; k <= Frame::FT_LastFrame; ++k) {
      QString name = Frame::ExtendedType(static_cast<Frame::Type>(k),
                                         QLatin1String("")).getName();
      nameMap.insert(QCoreApplication::translate("@default",
                         name.toLatin1().constData()), name);
    }
    QMap<QByteArray, QByteArray> idStrMap = getDisplayNamesOfIds();
    const auto names = idStrMap.values();
    for (const QByteArray& name : names) {
      nameMap.insert(QCoreApplication::translate("@default", name),
                     QString::fromLatin1(name));
    }
  }
  return nameMap.value(name, name);
}


/**
 * Get a translated string for a field ID.
 *
 * @param type field ID type
 *
 * @return field ID type, null string if unknown.
 */
QString Frame::Field::getFieldIdName(FieldId type)
{
  Q_STATIC_ASSERT(
      sizeof(fieldIdNames) / sizeof(fieldIdNames[0]) == ID_Seller + 2);
  if (static_cast<int>(type) >= 0 &&
      static_cast<int>(type) < static_cast<int>(
        sizeof(fieldIdNames) / sizeof(fieldIdNames[0]) - 1)) {
    return QCoreApplication::translate("@default", fieldIdNames[type]);
  }
  return QString();
}

/**
 * List of field ID strings, NULL terminated.
 */
const char* const* Frame::Field::getFieldIdNames()
{
  return fieldIdNames;
}

/**
 * Get field ID from field name.
 * @param fieldName name of field, can be English or translated
 * @return field ID, ID_NoField if not found.
 */
Frame::FieldId Frame::Field::getFieldId(const QString& fieldName)
{
  const char* const* fn;
  int id;
  // First try to find an exact English match.
  for (fn = fieldIdNames, id = 0; *fn != nullptr; ++fn, ++id) {
    if (fieldName == QLatin1String(*fn)) {
      return static_cast<FieldId>(id);
    }
  }
  // Then try to find a lowercase match ignoring spaces.
  QString lcName = fieldName.toLower().remove(QLatin1Char(' '));
  for (fn = fieldIdNames, id = 0; *fn != nullptr; ++fn, ++id) {
    if (lcName ==  QString::fromLatin1(*fn).toLower().remove(QLatin1Char(' '))) {
      return static_cast<FieldId>(id);
    }
  }
  // Finally try to find a translated name.
  for (fn = fieldIdNames, id = 0; *fn != nullptr; ++fn, ++id) {
    if (fieldName == QCoreApplication::translate("@default", *fn)) {
      return static_cast<FieldId>(id);
    }
  }
  return ID_NoField;
}

/**
 * Get a translated string for a text encoding.
 *
 * @param type text encoding type
 *
 * @return text encoding type, null string if unknown.
 */
QString Frame::Field::getTextEncodingName(TextEncoding type)
{
  if (static_cast<int>(type) >= 0 &&
      static_cast<int>(type) < static_cast<int>(
        sizeof(textEncodingNames) / sizeof(textEncodingNames[0]) - 1)) {
    return QCoreApplication::translate("@default", textEncodingNames[type]);
  }
  return QString();
}

/**
 * List of text encoding strings, NULL terminated.
 */
const char* const* Frame::Field::getTextEncodingNames()
{
  return textEncodingNames;
}

/**
 * Get a translated string for a timestamp format.
 *
 * @param type timestamp format type
 *
 * @return timestamp format type, null string if unknown.
 */
QString Frame::Field::getTimestampFormatName(int type)
{
  if (type >= 0 &&
      static_cast<unsigned int>(type) <
      sizeof(timestampFormatNames) / sizeof(timestampFormatNames[0]) - 1) {
    return QCoreApplication::translate("@default", timestampFormatNames[type]);
  }
  return QString();
}

/**
 * List of timestamp format strings, NULL terminated.
 */
const char* const* Frame::Field::getTimestampFormatNames()
{
  return timestampFormatNames;
}

/**
 * Get a translated string for a content type.
 *
 * @param type content type
 *
 * @return content type, null string if unknown.
 */
QString Frame::Field::getContentTypeName(int type)
{
  if (type >= 0 &&
      static_cast<unsigned int>(type) <
      sizeof(contentTypeNames) / sizeof(contentTypeNames[0]) - 1) {
    return QCoreApplication::translate("@default", contentTypeNames[type]);
  }
  return QString();
}

/**
 * List of content type strings, NULL terminated.
 */
const char* const* Frame::Field::getContentTypeNames()
{
  return contentTypeNames;
}

/**
 * Compare two field lists in a tolerant way.
 * This function can be used instead of the standard QList equality
 * operator if the field lists can be from different tag formats, which
 * may not all support the same field types.
 * @param fl1 first field list
 * @param fl2 second field list
 * @return true if they are similar enough.
 */
bool Frame::Field::fuzzyCompareFieldLists(const QList<Field>& fl1,
                                          const QList<Field>& fl2)
{
  return reducedFieldList(fl1) == reducedFieldList(fl2);
}

/**
 * Get list of available tag versions with translated description.
 * @return tag version/description pairs.
 */
const QList<QPair<Frame::TagVersion, QString> > Frame::availableTagVersions()
{
  QList<QPair<TagVersion, QString> > result;
  FOR_ALL_TAGS(tagNr) {
    const char* const tagStr = QT_TRANSLATE_NOOP("@default", "Tag %1");
    result << qMakePair(Frame::tagVersionCast(1 << tagNr),
                        QCoreApplication::translate("@default", tagStr)
                        .arg(Frame::tagNumberToString(tagNr)));
  }
  const char* const tag12Str = QT_TRANSLATE_NOOP("@default", "Tag 1 and Tag 2");
  result << qMakePair(TagV2V1, QCoreApplication::translate("@default",
                                                           tag12Str));
  if (TagVAll != TagV2V1) {
    const char* const allTagsStr = QT_TRANSLATE_NOOP("@default", "All Tags");
    result << qMakePair(TagVAll, QCoreApplication::translate("@default",
                                                             allTagsStr));
  }
  return result;
}

/**
 * Get string representation for tag number.
 * @param tagNr tag number
 * @return "1" for Tag_1, "2" for Tag_2, ..., null if invalid.
 */
QString Frame::tagNumberToString(TagNumber tagNr)
{
  return tagNr < Tag_NumValues ? QString::number(tagNr + 1) : QString();
}

/**
 * Get tag number from string representation.
 * @param str string representation
 * @return Tag_1 for "1", Tag_2 for "2", ..., Tag_NumValues if invalid.
 */
Frame::TagNumber Frame::tagNumberFromString(const QString& str)
{
  bool ok;
  int nr = str.toInt(&ok);
  return ok && --nr >= Frame::Tag_1 && nr < Frame::Tag_NumValues
      ? static_cast<Frame::TagNumber>(nr) : Tag_NumValues;
}

#ifndef QT_NO_DEBUG

namespace {

/**
 * Get string representation of variant.
 * @param val variant value
 * @return string representation.
 */
QString variantToString(const QVariant& val)
{
#if QT_VERSION >= 0x060000
  if (val.typeId() == QMetaType::QByteArray) {
#else
  if (val.type() == QVariant::ByteArray) {
#endif
    return QString(QLatin1String("ByteArray of %1 bytes"))
        .arg(val.toByteArray().size());
  } else {
    return val.toString();
  }
}

}

/**
 * Dump contents of frame to debug console.
 */
void Frame::dump() const
{
  qDebug("Frame: name=%s, value=%s, type=%s, index=%d, valueChanged=%u, marked=%s",
         getInternalName().toLatin1().data(), m_value.toLatin1().data(),
         getNameFromType(getType()), m_index,
         m_valueChanged, qPrintable(m_marked.getDescription()));
  qDebug("  fields=");
  for (auto it = m_fieldList.constBegin(); it != m_fieldList.constEnd(); ++it) {
    qDebug("  Field: id=%s, value=%s",
           qPrintable(
             Field::getFieldIdName(static_cast<FieldId>((*it).m_id))),
           variantToString((*it).m_value).toLatin1().data());
  }
}
#endif


/**
 * Bit mask containing the bits of all frame types which shall be used as
 * quick access frames.
 * This mask has to be handled like FrameFilter::m_enabledFrames.
 */
quint64 FrameCollection::s_quickAccessFrames =
    FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES;

/**
 * Set values which are different inactive.
 *
 * @param others frames to compare, will be modified
 * @param differentValues optional storage for the different values
 */
void FrameCollection::filterDifferent(
    FrameCollection& others,
    QHash<Frame::ExtendedType, QSet<QString>>* differentValues)
{
  const int ALREADY_HANDLED_INDEX = INT_MIN;
  QByteArray frameData, othersData;
  auto it = begin();
  while (it != end()) {
    auto& frame = const_cast<Frame&>(*it);
    // This frame list is not tied to a specific file, so the
    // index is not valid.
    frame.setIndex(-1);
    auto othersIt = others.find(frame);
    if (othersIt == others.end()) {
      frame.setDifferent();
      ++it;
    } else {
      while (it != end() && othersIt != others.end() &&
             !(frame < *it) && !(frame < *othersIt)) {
        if ((it->getType() != Frame::FT_Picture &&
             it->getValue() != othersIt->getValue()) ||
            (it->getType() == Frame::FT_Picture &&
             !(PictureFrame::getData(*it, frameData) &&
               PictureFrame::getData(*othersIt, othersData) &&
               frameData == othersData))) {
          if (differentValues && it->getType() != Frame::FT_Picture &&
              it->getType() != Frame::FT_Genre) {
            auto& valueSet = (*differentValues)[it->getExtendedType()];
            if (it->getValue() != Frame::differentRepresentation()) {
              valueSet.insert(it->getValue());
            }
            valueSet.insert(othersIt->getValue());
          }
          const_cast<Frame&>(*it).setDifferent();
        }
        // Mark as already handled.
        const_cast<Frame&>(*othersIt).setIndex(ALREADY_HANDLED_INDEX);
        ++it;
        ++othersIt;
      }
    }
  }

  // Insert frames which are in others but not in this (not marked as already
  // handled by index ALREADY_HANDLED_INDEX) as different frames.
  for (auto othersIt = others.begin();
       othersIt != others.end();
       ++othersIt) {
    if (othersIt->getIndex() != ALREADY_HANDLED_INDEX) {
      auto& frame = const_cast<Frame&>(*othersIt);
      frame.setIndex(-1);
      frame.setDifferent();
      insert(frame);
    }
  }
}

/**
 * Add standard frames which are missing.
 */
void FrameCollection::addMissingStandardFrames()
{
  quint64 mask;
  int i;
  for (i = Frame::FT_FirstFrame, mask = 1ULL;
       i <= Frame::FT_LastFrame;
       ++i, mask <<= 1) {
    if (s_quickAccessFrames & mask) {
      Frame frame(static_cast<Frame::Type>(i), QString(), QString(), -1);
      auto it = find(frame);
      if (it == end()) {
        insert(frame);
      }
    }
  }
}

/**
 * Copy enabled frames.
 *
 * @param flt filter with enabled frames
 *
 * @return copy with enabled frames.
 */
FrameCollection FrameCollection::copyEnabledFrames(const FrameFilter& flt) const
{
  FrameCollection frames;
  for (auto it = cbegin(); it != cend(); ++it) {
    if (flt.isEnabled(it->getType(), it->getName())) {
      Frame frame = *it;
      frame.setIndex(-1);
      frames.insert(frame);
    }
  }
  return frames;
}

/**
 * Remove all frames which are not enabled from the collection.
 *
 * @param flt filter with enabled frames
 */
void FrameCollection::removeDisabledFrames(const FrameFilter& flt)
{
  for (auto it = begin(); it != end();) {
    if (!flt.isEnabled(it->getType(), it->getName())) {
      erase(it++);
    } else {
      ++it;
    }
  }
}

/**
 * Set the index of all frames to -1.
 */
void FrameCollection::setIndexesInvalid()
{
  for (auto it = begin(); it != end(); ++it) {
    auto& frame = const_cast<Frame&>(*it);
    frame.setIndex(-1);
  }
}

/**
 * Copy frames which are empty or inactive from other frames.
 * This can be used to merge two frame collections.
 *
 * @param frames other frames
 */
void FrameCollection::merge(const FrameCollection& frames)
{
  for (auto otherIt = frames.cbegin(); otherIt != frames.cend(); ++otherIt) {
    auto it = find(*otherIt);
    if (it != end()) {
      QString value(otherIt->getValue());
      auto& frameFound = const_cast<Frame&>(*it);
      if (frameFound.getValue().isEmpty() && !value.isEmpty()) {
        frameFound.setValueIfChanged(value);
      }
    } else {
      Frame frame(*otherIt);
      frame.setIndex(-1);
      frame.setValueChanged(true);
      insert(frame);
    }
  }
}

/**
 * Check if the standard tags are empty or inactive.
 *
 * @return true if empty or inactive.
 */
bool FrameCollection::isEmptyOrInactive() const
{
  return
    getTitle().isEmpty() &&
    getArtist().isEmpty() &&
    getAlbum().isEmpty() &&
    getComment().isEmpty() &&
    getYear() <= 0 &&
    getTrack() <= 0 &&
    getGenre().isEmpty();
}

/**
 * Search for a frame only by name.
 *
 * @param name the name of the frame to find, a case-insensitive search for
 *             the first name starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::searchByName(
    const QString& name) const
{
  if (name.isEmpty())
    return cend();

  const_iterator it;
  QString ucName = name.toUpper().remove(QLatin1Char('/'));
  int len = ucName.length();
  for (it = cbegin(); it != cend(); ++it) {
    const QStringList names{it->getName(), it->getInternalName()};
    for (const QString& frameName : names) {
      QString ucFrameName(frameName.toUpper().remove(QLatin1Char('/')));
#if QT_VERSION >= 0x060000
      if (ucName == ucFrameName.left(len))
#else
      if (ucName == ucFrameName.leftRef(len))
#endif
      {
        return it;
      }
      int nlPos = ucFrameName.indexOf(QLatin1Char('\n'));
#if QT_VERSION >= 0x060000
      if (nlPos > 0 && ucName == ucFrameName.mid(nlPos + 1, len))
#else
      if (nlPos > 0 && ucName == ucFrameName.midRef(nlPos + 1, len))
#endif
      {
        // Description in TXXX, WXXX, COMM, PRIV matches
        return it;
      }
    }
  }
  return it;
}

/**
 * Find a frame by name.
 *
 * @param name  the name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 * @param index 0 for first frame with @a name, 1 for second, etc.
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByName(
    const QString& name, int index) const
{
  Frame frame(Frame::ExtendedType(name), QLatin1String(""), -1);
  auto it = find(frame);
  if (it == cend()) {
    it = searchByName(name);
    if (it == cend()) {
      const auto ids = getDisplayNamesOfIds().keys(name.toLatin1());
      for (const QByteArray& id : ids) {
        if (!id.isEmpty()) {
          it = searchByName(QString::fromLatin1(id));
          if (it != cend()) {
            break;
          }
        }
      }
    }
  }
  if (index > 0 && it != cend()) {
    const Frame::ExtendedType extendedType = it->getExtendedType();
    for (int i = 0; i < index && it != cend(); ++i, ++it) {}
    if (it != cend() && !(it->getExtendedType() == extendedType)) {
      it = cend();
    }
  }
  return it;
}

/**
 * Find a frame by type or name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByExtendedType(
    const Frame::ExtendedType& type, int index) const
{
  Frame frame(type, QLatin1String(""), -1);
  auto it = find(frame);
  if (it == cend()) {
    it = searchByName(frame.getInternalName());
  }
  if (index > 0 && it != cend()) {
    const Frame::ExtendedType extendedType = it->getExtendedType();
    for (int i = 0; i < index && it != cend(); ++i, ++it) {}
    if (it != cend() && !(it->getExtendedType() == extendedType)) {
      it = cend();
    }
  }
  return it;
}

/**
 * Find a frame by index.
 *
 * @param index the index in the frame, see \ref Frame::getIndex()
 *
 * @return iterator or end() if not found.
 */
FrameCollection::const_iterator FrameCollection::findByIndex(int index) const
{
  const_iterator it;
  for (it = cbegin(); it != cend(); ++it) {
    if (it->getIndex() == index) {
      break;
    }
  }
  return it;
}

/**
 * Get value by type.
 *
 * @param type type
 *
 * @return value, QString::null if not found.
 */
QString FrameCollection::getValue(Frame::Type type) const
{
  Frame frame(type, QLatin1String(""), QLatin1String(""), -1);
  auto it = find(frame);
  return it != cend() ? it->getValue() : QString();
}

/**
 * Get value by type and name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 *
 * @return value, QString::null if not found.
 */
QString FrameCollection::getValue(const Frame::ExtendedType& type) const
{
  auto it = findByExtendedType(type);
  return it != cend() ? it->getValue() : QString();
}

/**
 * Set value by type.
 *
 * @param type type
 * @param value value, nothing is done if QString::null
 */
void FrameCollection::setValue(Frame::Type type, const QString& value)
{
  if (!value.isNull()) {
    Frame frame(type, QLatin1String(""), QLatin1String(""), -1);
    auto it = find(frame);
    if (it != end()) {
      auto& frameFound = const_cast<Frame&>(*it);
      frameFound.setValueIfChanged(value);
    } else {
      frame.setValueIfChanged(value);
      insert(frame);
    }
  }
}

/**
 * Set value by type and name.
 *
 * @param type  type and name of the frame to find, if the exact name is not
 *              found, a case-insensitive search for the first name
 *              starting with this string is performed
 * @param value value, nothing is done if QString::null
 */
void FrameCollection::setValue(const Frame::ExtendedType& type,
                               const QString& value)
{
  if (!value.isNull()) {
    Frame frame(type, QLatin1String(""), -1);
    auto it = find(frame);
    if (it == end()) {
      it = searchByName(type.getInternalName());
    }
    if (it != end()) {
      auto& frameFound = const_cast<Frame&>(*it);
      frameFound.setValueIfChanged(value);
    } else {
      frame.setValueIfChanged(value);
      insert(frame);
    }
  }
}

/**
 * Get integer value by type.
 *
 * @param type type
 *
 * @return value, 0 if empty, -1 if not found.
 */
int FrameCollection::getIntValue(Frame::Type type) const
{
  QString str = getValue(type);
  return str.isNull() ? -1 : str.toInt();
}

/**
 * Set integer value by type.
 *
 * @param type type
 * @param value value, 0 to set empty, nothing is done if -1
 */
void FrameCollection::setIntValue(Frame::Type type, int value)
{
  if (value != -1) {
    QString str = value != 0 ? QString::number(value) : QLatin1String("");
    setValue(type, str);
  }
}

/**
 * Compare the frames with another frame collection and mark the value as
 * changed on frames which are different.
 *
 * @param other other frame collection
 */
void FrameCollection::markChangedFrames(const FrameCollection& other)
{
  for (auto it = begin(); it != end(); ++it) {
    auto otherIt = it->getIndex() != -1
        ? other.findByIndex(it->getIndex())
        : other.find(*it);
    auto& frame = const_cast<Frame&>(*it);
    frame.setValueChanged(!(otherIt != other.cend() && otherIt->isEqual(*it)));
  }
}

#ifndef QT_NO_DEBUG
/**
 * Dump contents of frame collection to debug console.
 */
void FrameCollection::dump() const
{
  qDebug("FrameCollection:");
  for (const_iterator it = cbegin(); it != cend(); ++it) {
    (*it).dump();
  }
}
#endif

/**
 * Create a frame collection from a list of subframe fields.
 *
 * The given subframe fields must start with a Frame::ID_Subframe field with
 * the frame name as its value, followed by the fields of the frame. More
 * subframes may follow.
 *
 * @param begin iterator to begin of subframes
 * @param end iterator after end of subframes
 *
 * @return frames constructed from subframe fields.
 */
FrameCollection FrameCollection::fromSubframes(
    Frame::FieldList::const_iterator begin, // clazy:exclude=function-args-by-ref
    Frame::FieldList::const_iterator end) // clazy:exclude=function-args-by-ref
{
  FrameCollection frames;
  Frame frame;
  int index = 0;
  for (auto it = begin; it != end; ++it) {
    const Frame::Field& fld = *it;
    if (fld.m_id == Frame::ID_Subframe) {
      if (frame.getType() != Frame::FT_UnknownFrame) {
        frame.setValueFromFieldList();
        frames.insert(frame);
        frame = Frame();
      }
      QString name = fld.m_value.toString();
      if (!name.isEmpty()) {
        frame.setExtendedType(Frame::ExtendedType(name));
        frame.setIndex(index++);
      }
    } else {
      if (frame.getType() != Frame::FT_UnknownFrame) {
        frame.fieldList().append(fld);
      }
    }
  }
  if (frame.getType() != Frame::FT_UnknownFrame) {
    frame.setValueFromFieldList();
    frames.insert(frame);
  }
  return frames;
}


/**
 * Constructor.
 * All frames are disabled
 */
FrameFilter::FrameFilter() : m_enabledFrames(0) {}

/**
 * Enable all frames.
 */
void FrameFilter::enableAll()
{
  m_enabledFrames = FTM_AllFrames;
  m_disabledOtherFrames.clear();
}

/**
 * Check if all fields are true.
 *
 * @return true if all fields are true.
 */
bool FrameFilter::areAllEnabled() const
{
  return (m_enabledFrames & FTM_AllFrames) == FTM_AllFrames &&
    m_disabledOtherFrames.empty();
}

/**
 * Check if frame is enabled.
 *
 * @param type frame type
 * @param name frame name
 *
 * @return true if frame is enabled.
 */
bool FrameFilter::isEnabled(Frame::Type type, const QString& name) const
{
  if (type <= Frame::FT_LastFrame) {
    return (m_enabledFrames & (1ULL << type)) != 0;
  } else if (!name.isEmpty()) {
    auto it = m_disabledOtherFrames.find(name);
    return it == m_disabledOtherFrames.end();
  } else {
    return true;
  }
}

/**
 * Enable or disable frame.
 *
 * @param type frame type
 * @param name frame name
 * @param en true to enable
 */
void FrameFilter::enable(Frame::Type type, const QString& name, bool en)
{
  if (type <= Frame::FT_LastFrame) {
    if (en) {
      m_enabledFrames |= (1ULL << type);
    } else {
      m_enabledFrames &= ~(1ULL << type);
    }
  } else if (!name.isEmpty()) {
    if (en) {
      auto it = m_disabledOtherFrames.find(name);
      if (it != m_disabledOtherFrames.end()) {
        m_disabledOtherFrames.erase(it);
      }
    } else {
      m_disabledOtherFrames.insert(name);
    }
  }
}


/**
 * Constructor.
 *
 * @param frames frame collection
 * @param str    string with format codes
 */
FrameFormatReplacer::FrameFormatReplacer(
  const FrameCollection& frames, const QString& str)
  : FormatReplacer(str), m_frames(frames) {}

/**
 * Replace a format code (one character %c or multiple characters %{chars}).
 * Supported format fields:
 * %s title (song)
 * %l album
 * %a artist
 * %c comment
 * %y year
 * %t track, two digits, i.e. leading zero if < 10
 * %T track, without leading zeroes
 * %g genre
 *
 * @param code format code
 *
 * @return replacement string,
 *         QString::null if code not found.
 */
QString FrameFormatReplacer::getReplacement(const QString& code) const
{
  QString result;
  QString name;

  if (code.length() == 1) {
    static const struct {
      const char* longCode;
      char shortCode;
    } shortToLong[] = {
      { "title", 's' },
      { "album", 'l' },
      { "artist", 'a' },
      { "comment", 'c' },
      { "year", 'y' },
      { "track", 't' },
      { "tracknumber", 'T' },
      { "genre", 'g' }
    };
    const char c = code[0].toLatin1();
    for (const auto& s2l : shortToLong) {
      if (s2l.shortCode == c) {
        name = QString::fromLatin1(s2l.longCode);
        break;
      }
    }
  } else if (code.length() > 1) {
    name = code;
  }

  if (!name.isNull()) {
    QString lcName(name.toLower());
    QString fieldName;
    int fieldWidth = lcName == QLatin1String("track") ? 2 : -1;
    if (lcName == QLatin1String("year")) {
      name = QLatin1String("date");
    } else if (lcName == QLatin1String("tracknumber")) {
      name = QLatin1String("track number");
    }
    int len = lcName.length();
    if (len > 2 && lcName.at(len - 2) == QLatin1Char('.') &&
        lcName.at(len - 1) >= QLatin1Char('0') &&
        lcName.at(len - 1) <= QLatin1Char('9')) {
      fieldWidth = lcName.at(len - 1).toLatin1() - '0';
      lcName.truncate(len - 2);
      name.truncate(len - 2);
    }
    const int dotIndex = name.indexOf(QLatin1Char('.'));
    if (dotIndex != -1) {
      fieldName = name.mid(dotIndex + 1);
      name.truncate(dotIndex);
    }

    if (name == QLatin1String("disk")) {
      name = QLatin1String("disc number");
    }

    auto it = m_frames.findByName(name);
    if (it != m_frames.cend()) {
      if (fieldName.isEmpty()) {
        result = it->getValue().trimmed();
      } else {
        result = Frame::getField(*it, fieldName).toString().trimmed();
      }
      if (result.isNull()) {
        // code was found, but value is empty
        result = QLatin1String("");
      }
      if (it->getType() == Frame::FT_Picture && result.isEmpty()) {
        QVariant fieldValue = it->getFieldValue(Frame::ID_Data);
        if (fieldValue.isValid() && fieldValue.toByteArray().size() > 0) {
          // If there is a picture without description, return "1", so that
          // an empty value indicates "no picture"
          result = QLatin1String("1");
        }
      }
    }

    if (lcName == QLatin1String("year")) {
      QRegularExpression yearRe(QLatin1String("^\\d{4}-\\d{2}"));
      auto match = yearRe.match(result);
      if (match.hasMatch()) {
        result.truncate(4);
      }
    }

    if (fieldWidth > 0) {
      bool ok;
      int nr = Frame::numberWithoutTotal(result, &ok);
      if (ok) {
        result = QString(QLatin1String("%1"))
            .arg(nr, fieldWidth, 10, QLatin1Char('0'));
      }
    }
  }

  return result;
}

/**
 * Get help text for supported format codes.
 *
 * @param onlyRows if true only the tr elements are returned,
 *                 not the surrounding table
 *
 * @return help text.
 */
QString FrameFormatReplacer::getToolTip(bool onlyRows)
{
  QString str;
  if (!onlyRows) str += QLatin1String("<table>\n");

  str += QLatin1String("<tr><td>%s</td><td>%{title}</td><td>");
  str += QCoreApplication::translate("@default", "Title");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%l</td><td>%{album}</td><td>");
  str += QCoreApplication::translate("@default", "Album");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%a</td><td>%{artist}</td><td>");
  str += QCoreApplication::translate("@default", "Artist");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%c</td><td>%{comment}</td><td>");
  str += QCoreApplication::translate("@default", "Comment");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%y</td><td>%{year}</td><td>");
  const char* const yearStr = QT_TRANSLATE_NOOP("@default", "Year");
  str += QCoreApplication::translate("@default", yearStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("<tr><td>%t</td><td>%{track}</td><td>");
  const char* const trackStr = QT_TRANSLATE_NOOP("@default", "Track");
  str += QCoreApplication::translate("@default", trackStr);
  str += QLatin1String(" &quot;01&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%t</td><td>%{track.3}</td><td>");
  str += QCoreApplication::translate("@default", trackStr);
  str += QLatin1String(" &quot;001&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%T</td><td>%{tracknumber}</td><td>");
  str += QCoreApplication::translate("@default", trackStr);
  str += QLatin1String(" &quot;1&quot;</td></tr>\n");

  str += QLatin1String("<tr><td>%g</td><td>%{genre}</td><td>");
  str += QCoreApplication::translate("@default", "Genre");
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String(R"(<tr><td></td><td>%{"t1"title"t2"}...</td><td>)");
  const char* const prependAppendStr =
      QT_TRANSLATE_NOOP("@default", "Prepend t1/append t2 if not empty");
  str += QCoreApplication::translate("@default", prependAppendStr);
  str += QLatin1String("</td></tr>\n");

  if (!onlyRows) str += QLatin1String("</table>\n");
  return str;
}
