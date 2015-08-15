/**
 * \file tagconfig.cpp
 * Tag related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#include "tagconfig.h"
#include <QCoreApplication>
#include "taggedfile.h"
#include "frame.h"

namespace {

/** Default value for comment name */
const char* const defaultCommentName = "COMMENT";

/** Index of latin-1 entry in getTextEncodingV1Names(). */
enum { TextEncodingV1Latin1Index = 13 };

/**
 * Remove aliases in braces from text encoding name.
 *
 * @param comboEntry text encoding name
 *
 * @return codec name.
 */
QString getTextEncodingV1CodecName(const QString& comboEntry)
{
  int braceIdx = comboEntry.indexOf(QLatin1String(" ("));
  return braceIdx == -1 ? comboEntry : comboEntry.left(braceIdx);
}

}

int TagConfig::s_index = -1;

/**
 * Constructor.
 */
TagConfig::TagConfig() :
  StoredConfig<TagConfig>(QLatin1String("Tags")),
  m_commentName(QString::fromLatin1(defaultCommentName)),
  m_pictureNameItem(VP_METADATA_BLOCK_PICTURE),
  m_id3v2Version(ID3v2_3_0),
  m_textEncodingV1(QLatin1String("ISO-8859-1")),
  m_textEncoding(TE_ISO8859_1),
  m_quickAccessFrames(FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES),
  m_trackNumberDigits(1),
  m_taggedFileFeatures(0),
  m_maximumPictureSize(131072),
  m_markOversizedPictures(false),
  m_onlyCustomGenres(false),
  m_markTruncations(true),
  m_enableTotalNumberOfTracks(false),
  m_genreNotNumeric(false)
{
}

/**
 * Destructor.
 */
TagConfig::~TagConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void TagConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("MarkTruncations"), QVariant(m_markTruncations));
  config->setValue(QLatin1String("MarkOversizedPictures"), QVariant(m_markOversizedPictures));
  config->setValue(QLatin1String("MaximumPictureSize"), QVariant(m_maximumPictureSize));
  config->setValue(QLatin1String("EnableTotalNumberOfTracks"), QVariant(m_enableTotalNumberOfTracks));
  config->setValue(QLatin1String("GenreNotNumeric"), QVariant(m_genreNotNumeric));
  config->setValue(QLatin1String("CommentName"), QVariant(m_commentName));
  config->setValue(QLatin1String("PictureNameItem"), QVariant(m_pictureNameItem));
  config->setValue(QLatin1String("CustomGenres"), QVariant(m_customGenres));
  config->setValue(QLatin1String("ID3v2Version"), QVariant(m_id3v2Version));
  config->setValue(QLatin1String("TextEncodingV1"), QVariant(m_textEncodingV1));
  config->setValue(QLatin1String("TextEncoding"), QVariant(m_textEncoding));
#ifdef Q_OS_MAC
  // Convince Mac OS X to store a 64-bit value.
  config->setValue(QLatin1String("QuickAccessFrames"), QVariant(m_quickAccessFrames | (Q_UINT64_C(1) << 63)));
#else
  config->setValue(QLatin1String("QuickAccessFrames"), QVariant(m_quickAccessFrames));
#endif
  config->setValue(QLatin1String("QuickAccessFrameOrder"),
                   QVariant(intListToStringList(m_quickAccessFrameOrder)));
  config->setValue(QLatin1String("TrackNumberDigits"), QVariant(m_trackNumberDigits));
  config->setValue(QLatin1String("OnlyCustomGenres"), QVariant(m_onlyCustomGenres));
  config->setValue(QLatin1String("PluginOrder"), QVariant(m_pluginOrder));
  config->setValue(QLatin1String("DisabledPlugins"), QVariant(m_disabledPlugins));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void TagConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_markTruncations = config->value(QLatin1String("MarkTruncations"), m_markTruncations).toBool();
  m_markOversizedPictures = config->value(QLatin1String("MarkOversizedPictures"), m_markOversizedPictures).toBool();
  m_maximumPictureSize = config->value(QLatin1String("MaximumPictureSize"), m_maximumPictureSize).toInt();
  m_enableTotalNumberOfTracks = config->value(QLatin1String("EnableTotalNumberOfTracks"), m_enableTotalNumberOfTracks).toBool();
  m_genreNotNumeric = config->value(QLatin1String("GenreNotNumeric"), m_genreNotNumeric).toBool();
  m_commentName = config->value(QLatin1String("CommentName"), QString::fromLatin1(defaultCommentName)).toString();
  m_pictureNameItem = config->value(QLatin1String("PictureNameItem"), VP_METADATA_BLOCK_PICTURE).toInt();
  m_customGenres = config->value(QLatin1String("CustomGenres"),
                                 m_customGenres).toStringList();
  m_id3v2Version = config->value(QLatin1String("ID3v2Version"), ID3v2_3_0).toInt();
  m_textEncodingV1 = config->value(QLatin1String("TextEncodingV1"), QLatin1String("ISO-8859-1")).toString();
  m_textEncoding = config->value(QLatin1String("TextEncoding"), TE_ISO8859_1).toInt();
  m_quickAccessFrames = config->value(QLatin1String("QuickAccessFrames"),
                                     FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES).toULongLong();
#ifdef Q_OS_MAC
  m_quickAccessFrames &= ~(Q_UINT64_C(1) << 63);
#endif
  m_quickAccessFrameOrder = stringListToIntList(
        config->value(QLatin1String("QuickAccessFrameOrder"), QStringList()).
        toStringList());
  m_trackNumberDigits = config->value(QLatin1String("TrackNumberDigits"), 1).toInt();
  m_onlyCustomGenres = config->value(QLatin1String("OnlyCustomGenres"), m_onlyCustomGenres).toBool();
  m_pluginOrder = config->value(QLatin1String("PluginOrder"),
                                 m_pluginOrder).toStringList();
  m_disabledPlugins = config->value(QLatin1String("DisabledPlugins"),
                                 m_disabledPlugins).toStringList();
  config->endGroup();

  if (m_pluginOrder.isEmpty()) {
    setDefaultPluginOrder();
  }
}

/**
 * Set default plugin order.
 */
void TagConfig::setDefaultPluginOrder()
{
  /** Default to filename format list */
  static const char* const defaultPluginOrder[] = {
    "Id3libMetadata",
    "OggFlacMetadata",
    "Mp4v2Metadata",
    "TaglibMetadata",
    0
  };

  m_pluginOrder.clear();
  for (const char* const* pn = defaultPluginOrder; *pn != 0; ++pn) {
    m_pluginOrder += QString::fromLatin1(*pn);
  }
}

/** version used for new ID3v2 tags */
int TagConfig::id3v2Version() const
{
  if (m_id3v2Version == ID3v2_3_0 &&
      !(taggedFileFeatures() & TaggedFile::TF_ID3v23))
    return ID3v2_4_0;
  if (m_id3v2Version == ID3v2_4_0 &&
      !(taggedFileFeatures() & TaggedFile::TF_ID3v24))
    return ID3v2_3_0;
  return m_id3v2Version;
}

/**
 * Set features provided by metadata plugins.
 * @param taggedFileFeatures bit mask with TaggedFile::Feature flags set
 */
void TagConfig::setTaggedFileFeatures(int taggedFileFeatures)
{
  if (m_taggedFileFeatures != taggedFileFeatures) {
    m_taggedFileFeatures = taggedFileFeatures;
    emit taggedFileFeaturesChanged(m_taggedFileFeatures);
  }
}

/** Set true to mark truncated ID3v1.1 fields. */
void TagConfig::setMarkTruncations(bool markTruncations)
{
  if (m_markTruncations != markTruncations) {
    m_markTruncations = markTruncations;
    emit markTruncationsChanged(m_markTruncations);
  }
}

/** Set true to mark oversized pictures. */
void TagConfig::setMarkOversizedPictures(bool markOversizedPictures)
{
  if (m_markOversizedPictures != markOversizedPictures) {
    m_markOversizedPictures = markOversizedPictures;
    emit markOversizedPicturesChanged(m_markOversizedPictures);
  }
}

/** Set maximum size of picture in bytes. */
void TagConfig::setMaximumPictureSize(int maximumPictureSize)
{
  if (m_maximumPictureSize != maximumPictureSize) {
    m_maximumPictureSize = maximumPictureSize;
    emit maximumPictureSizeChanged(m_maximumPictureSize);
  }
}

/** Set true to write total number of tracks into track fields. */
void TagConfig::setEnableTotalNumberOfTracks(bool enableTotalNumberOfTracks)
{
  if (m_enableTotalNumberOfTracks != enableTotalNumberOfTracks) {
    m_enableTotalNumberOfTracks = enableTotalNumberOfTracks;
    emit enableTotalNumberOfTracksChanged(m_enableTotalNumberOfTracks);
  }
}

/** Set true to write genres as text instead of numeric string. */
void TagConfig::setGenreNotNumeric(bool genreNotNumeric)
{
  if (m_genreNotNumeric != genreNotNumeric) {
    m_genreNotNumeric = genreNotNumeric;
    emit genreNotNumericChanged(m_genreNotNumeric);
  }
}

/** Set field name used for Vorbis comment entries. */
void TagConfig::setCommentName(const QString& commentName)
{
  if (m_commentName != commentName) {
    m_commentName = commentName;
    emit commentNameChanged(m_commentName);
  }
}

/** Set index of field name used for Vorbis picture entries. */
void TagConfig::setPictureNameIndex(int pictureNameItem)
{
  if (m_pictureNameItem != pictureNameItem) {
    m_pictureNameItem = pictureNameItem;
    emit pictureNameIndexChanged(m_pictureNameItem);
  }
}

/** Set custom genres for ID3v2.3. */
void TagConfig::setCustomGenres(const QStringList& customGenres)
{
  if (m_customGenres != customGenres) {
    m_customGenres = customGenres;
    emit customGenresChanged(m_customGenres);
  }
}

/** Set version used for new ID3v2 tags. */
void TagConfig::setId3v2Version(int id3v2Version)
{
  if (m_id3v2Version != id3v2Version) {
    m_id3v2Version = id3v2Version;
    emit id3v2VersionChanged(m_id3v2Version);
  }
}

/** Set text encoding used for new ID3v1 tags. */
void TagConfig::setTextEncodingV1(const QString& textEncodingV1)
{
  if (m_textEncodingV1 != textEncodingV1) {
    m_textEncodingV1 = textEncodingV1;
    emit textEncodingV1Changed(m_textEncodingV1);
  }
}

/** index of ID3v1 text encoding in getTextEncodingV1Names() */
int TagConfig::textEncodingV1Index() const
{
  int index = 0;
  QStringList textEncodingV1List = getTextEncodingV1Names();
  for (QStringList::const_iterator it = textEncodingV1List.constBegin();
       it != textEncodingV1List.constEnd();
       ++it) {
    if (getTextEncodingV1CodecName(*it) == m_textEncodingV1) {
      return index;
    }
    ++index;
  }
  return TextEncodingV1Latin1Index;
}

/** Set ID3v1 text encoding from index in getTextEncodingV1Names(). */
void TagConfig::setTextEncodingV1Index(int index)
{
  QStringList textEncodingV1List = getTextEncodingV1Names();
  if (index >= 0 && index < textEncodingV1List.size()) {
    setTextEncodingV1(getTextEncodingV1CodecName(textEncodingV1List.at(index)));
  }
}

/** Set text encoding used for new ID3v2 tags. */
void TagConfig::setTextEncoding(int textEncoding)
{
  if (m_textEncoding != textEncoding) {
    m_textEncoding = textEncoding;
    emit textEncodingChanged(m_textEncoding);
  }
}

/** Set frames which are displayed for Tag 2 even if not present. */
void TagConfig::setQuickAccessFrames(quint64 quickAccessFrames)
{
  if (m_quickAccessFrames != quickAccessFrames) {
    m_quickAccessFrames = quickAccessFrames;
    emit quickAccessFramesChanged(m_quickAccessFrames);
  }
}

/** Set order of frames which are displayed for Tag 2 even if not present. */
void TagConfig::setQuickAccessFrameOrder(const QList<int>& frameTypes)
{
  if (m_quickAccessFrameOrder != frameTypes) {
    m_quickAccessFrameOrder = frameTypes;
    emit quickAccessFrameOrderChanged(m_quickAccessFrameOrder);
  }
}

/** Set number of digits in track number. */
void TagConfig::setTrackNumberDigits(int trackNumberDigits)
{
  if (m_trackNumberDigits != trackNumberDigits) {
    m_trackNumberDigits = trackNumberDigits;
    emit trackNumberDigitsChanged(m_trackNumberDigits);
  }
}

/** Set true to show only custom genres in combo boxes. */
void TagConfig::setOnlyCustomGenres(bool onlyCustomGenres)
{
  if (m_onlyCustomGenres != onlyCustomGenres) {
    m_onlyCustomGenres = onlyCustomGenres;
    emit onlyCustomGenresChanged(m_onlyCustomGenres);
  }
}

/** Set the order in which meta data plugins are tried when opening a file. */
void TagConfig::setPluginOrder(const QStringList& pluginOrder)
{
  if (m_pluginOrder != pluginOrder) {
    m_pluginOrder = pluginOrder;
    emit pluginOrderChanged(m_pluginOrder);
  }
}

/** Set list of disabled plugins. */
void TagConfig::setDisabledPlugins(const QStringList& disabledPlugins)
{
  if (m_disabledPlugins != disabledPlugins) {
    m_disabledPlugins = disabledPlugins;
    emit disabledPluginsChanged(m_disabledPlugins);
  }
}

/**
 * Set list of available plugins.
 * @param availablePlugins available plugins
 */
void TagConfig::setAvailablePlugins(const QStringList& availablePlugins)
{
  if (m_availablePlugins != availablePlugins) {
    m_availablePlugins = availablePlugins;
    emit availablePluginsChanged(m_availablePlugins);
  }
}

/**
 * String list of encodings for ID3v1.
 */
QStringList TagConfig::getTextEncodingV1Names()
{
  static QStringList textEncodingV1List;
  if (textEncodingV1List.isEmpty()) {
    static const char* const codecs[] = {
      "Apple Roman (macintosh)",
      "Big5",
      "big5-0",
      "Big5-HKSCS",
      "big5hkscs-0",
      "EUC-JP",
      "EUC-KR",
      "GB18030",
      "GBK (windows-936)",
      "hp-roman8",
      "IBM850",
      "IBM866",
      "ISO-2022-JP (JIS7)",
      "ISO-8859-1 (latin1)",
      "ISO-8859-2 (latin2)",
      "ISO-8859-3 (latin3)",
      "ISO-8859-4 (latin4)",
      "ISO-8859-5 (cyrillic)",
      "ISO-8859-6 (arabic)",
      "ISO-8859-7 (greek)",
      "ISO-8859-8 (hebrew)",
      "ISO-8859-9 (latin5)",
      "ISO-8859-10 (latin6)",
      "ISO-8859-13 (baltic)",
      "ISO-8859-14 (latin8, iso-celtic)",
      "ISO-8859-15 (latin9)",
      "ISO-8859-16 (latin10)",
      "ISO-10646-UCS-2 (UTF-16)",
      "Iscii-Bng",
      "Iscii-Dev",
      "Iscii-Gjr",
      "Iscii-Knd",
      "Iscii-Mlm",
      "Iscii-Ori",
      "Iscii-Pnj",
      "Iscii-Tlg",
      "Iscii-Tml",
      "jisx0201*-0",
      "KOI8-R",
      "KOI8-U",
      "ksc5601.1987-0",
      "mulelao-1",
      "Shift_JIS (SJIS, MS_Kanji)",
      "TIS-620 (ISO 8859-11)",
      "TSCII",
      "UTF-8",
      "windows-1250",
      "windows-1251",
      "windows-1252",
      "windows-1253",
      "windows-1254",
      "windows-1255",
      "windows-1256",
      "windows-1257",
      "windows-1258",
      "WINSAMI2 (WS2)",
      0
    };
    Q_ASSERT(qstrcmp(codecs[TextEncodingV1Latin1Index], "ISO-8859-1 (latin1)") == 0);
    const char* const* str = codecs;
    while (*str) {
      textEncodingV1List += QString::fromLatin1(*str++);
    }
  }
  return textEncodingV1List;
}

/**
 * String list of encodings for ID3v2.
 */
QStringList TagConfig::getTextEncodingNames()
{
  static const int NUM_NAMES = 3;
  static const char* const names[NUM_NAMES] = {
    QT_TRANSLATE_NOOP("@default", "ISO-8859-1"),
    QT_TRANSLATE_NOOP("@default", "UTF16"),
    QT_TRANSLATE_NOOP("@default", "UTF8")
  };
  QStringList strs;
#if QT_VERSION >= 0x040700
  strs.reserve(NUM_NAMES);
#endif
  for (int i = 0; i < NUM_NAMES; ++i) {
    strs.append(QCoreApplication::translate("@default", names[i]));
  }
  return strs;
}

/**
 * String list of possible versions used for new ID3v2 tags.
 */
QStringList TagConfig::getId3v2VersionNames()
{
  return QStringList() << QLatin1String("ID3v2.3.0")
                       << QLatin1String("ID3v2.4.0");
}

/**
 * String list with suggested field names used for Vorbis comment entries.
 */
QStringList TagConfig::getCommentNames()
{
  return QStringList() << QLatin1String("COMMENT")
                       << QLatin1String("DESCRIPTION");
}

/**
 * String list with possible field names used for Vorbis picture entries.
 */
QStringList TagConfig::getPictureNames()
{
  return QStringList() << QLatin1String("METADATA_BLOCK_PICTURE")
                       << QLatin1String("COVERART");
}
