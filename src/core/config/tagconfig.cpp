/**
 * \file tagconfig.cpp
 * Tag related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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
#include <QVector>
#include <QVariantMap>
#include "taggedfile.h"
#include "frame.h"
#include "isettings.h"

namespace {

/** Default value for comment name */
const char* const defaultCommentName = "COMMENT";

/** Default value for RIFF track name */
const char* const defaultRiffTrackName = "IPRT";

}


/**
 * Mapping between star count and rating values.
 */
class StarRatingMapping {
public:
  /** Maximum number of stars. */
  static constexpr int MAX_STAR_COUNT = 5;

  /** Constructor. */
  StarRatingMapping();

  /**
   * Get star count from rating value.
   * @param rating rating value stored in tag frame
   * @param type rating type containing frame name and optionally field value,
   * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
   * @return number of stars (1..5).
   */
  int starCountFromRating(int rating, const QString& type) const;

  /**
   * Get rating value from star count.
   * @param starCount number of stars (1..5)
   * @param type rating type containing frame name and optionally field value,
   * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
   * @return rating value stored in tag frame, usually a value between 1 and 255
   * or 1 and 100.
   */
  int starCountToRating(int starCount, const QString& type) const;

  /** Serialize to string list. */
  QStringList toStringList() const;

  /** Set from string list. */
  void fromStringList(const QStringList& strs);

  /** Get default value for Email field in POPM frame. */
  QString defaultPopmEmail() const;

  /** Get mappings. */
  const QList<QPair<QString, QVector<int> > >& getMappings() const {
    return m_maps;
  }

  /** Set mappings. */
  void setMappings(const QList<QPair<QString, QVector<int> > >& maps) {
    m_maps = maps;
  }

private:
  const QVector<int>& valuesForType(const QString& type) const;

  QVector<int> m_wmpValues;
  QList<QPair<QString, QVector<int> > > m_maps;
};

StarRatingMapping::StarRatingMapping()
{
  m_wmpValues << 1 << 64 << 128 << 196 << 255;
  QVector<int> traktorValues, wmaValues, percentValues;
  traktorValues << 51 << 102 << 153 << 204 << 255;
  wmaValues << 1<< 25<< 50 << 75 << 99;
  percentValues << 20 << 40 << 60 << 80 << 100;
  m_maps << qMakePair(QString(QLatin1String("POPM")), m_wmpValues);
  m_maps << qMakePair(QString(QLatin1String("POPM.Windows Media Player 9 Series")),
                      m_wmpValues);
  m_maps << qMakePair(QString(QLatin1String("POPM.traktor@native-instruments.de")),
                      traktorValues);
  m_maps << qMakePair(QString(QLatin1String("WM/SharedUserRating")), wmaValues);
  m_maps << qMakePair(QString(QLatin1String("IRTD")), percentValues);
  m_maps << qMakePair(QString(QLatin1String("rate")), percentValues);
  m_maps << qMakePair(QString(QLatin1String("RATING")), percentValues);
}

int StarRatingMapping::starCountFromRating(int rating, const QString& type) const
{
  if (rating < 1) {
    return 0;
  }
  const QVector<int>& vals = valuesForType(type);
  const bool useWmpHack = vals.at(3) == 196;
  for (int i = 1; i < MAX_STAR_COUNT; ++i) {
    // This is done the weird way in order to get the same thresholds as
    // apparently used in Windows Explorer:
    // 1:  1-31, 2: 32-95, 3: 96-159, 4:160-223, 5:224-255
    if (int threshold = useWmpHack
                          ? (((vals[i - 1] + 1) & ~7) + ((vals[i] + 1) & ~7)) / 2
                          : (vals[i - 1] + vals[i] + 1) / 2;
      rating < threshold) {
      return i;
    }
  }
  return MAX_STAR_COUNT;
}

int StarRatingMapping::starCountToRating(int starCount, const QString& type) const
{
  if (starCount < 1) {
    return 0;
  }
  if (starCount > MAX_STAR_COUNT) {
    starCount = MAX_STAR_COUNT;
  }
  return valuesForType(type).at(starCount - 1);
}

const QVector<int>& StarRatingMapping::valuesForType(const QString& type) const
{
  // First search in the maps for the given type.
  for (auto it = m_maps.constBegin(); it != m_maps.constEnd(); ++it) {
    if (type == it->first) {
      return it->second;
    }
  }
  // If not found, use the first map or the WMP map if no maps are available.
  return m_maps.isEmpty() ? m_wmpValues : m_maps.first().second;
}

QStringList StarRatingMapping::toStringList() const
{
  QStringList strs;
  for (auto it = m_maps.constBegin(); it != m_maps.constEnd(); ++it) {
    QString str = it->first;
    for (auto sit = it->second.constBegin(); sit != it->second.constEnd(); ++sit) {
      str += QLatin1Char(',');
      str += QString::number(*sit);
    }
    strs.append(str);
  }
  return strs;
}

void StarRatingMapping::fromStringList(const QStringList& strs)
{
  QList<QPair<QString, QVector<int> > > maps;
  QVector<int> values;
  for (auto it = strs.constBegin(); it != strs.constEnd(); ++it) {
    QStringList parts = it->split(QLatin1Char(','));
    if (const int numParts = parts.size(); numParts >= MAX_STAR_COUNT + 1) {
      bool ok = false;
      values.resize(0);
      int lastValue = -1;
      for (int i = numParts - MAX_STAR_COUNT; i < numParts; ++i) {
        int value = parts.at(i).toInt(&ok);
        if (value <= lastValue) {
          ok = false;
        }
        if (!ok) {
          break;
        }
        values.append(value);
      }
      if (ok) {
        const QStringList typeParts = parts.mid(0, numParts - MAX_STAR_COUNT);
        const QString type = typeParts.join(QLatin1String(","));
        maps.append(qMakePair(type, values));
      }
    }
  }
  if (!maps.isEmpty()) {
    m_maps.swap(maps);
  }
}

QString StarRatingMapping::defaultPopmEmail() const
{
  for (auto it = m_maps.constBegin(); it != m_maps.constEnd(); ++it) {
    if (QString type = it->first; type.startsWith(QLatin1String("POPM"))) {
      return type.length() > 4 && type.at(4) == QLatin1Char('.')
          ? type.mid(5) : QLatin1String("");
    }
  }
  return QString();
}


int TagConfig::s_index = -1;

/**
 * Constructor.
 */
TagConfig::TagConfig()
  : StoredConfig(QLatin1String("Tags")),
    m_starRatingMapping(new StarRatingMapping),
    m_commentName(QString::fromLatin1(defaultCommentName)),
    m_riffTrackName(QString::fromLatin1(defaultRiffTrackName)),
    m_pictureNameItem(VP_METADATA_BLOCK_PICTURE),
    m_id3v2Version(ID3v2_3_0),
    m_textEncodingV1(QLatin1String("ISO-8859-1")),
    m_textEncoding(TE_ISO8859_1),
    m_quickAccessFrames(FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES),
    m_trackNumberDigits(1),
    m_taggedFileFeatures(0),
    m_maximumPictureSize(131072),
    m_markOversizedPictures(false),
    m_markStandardViolations(true),
    m_onlyCustomGenres(false),
    m_markTruncations(true),
    m_enableTotalNumberOfTracks(false),
    m_genreNotNumeric(true),
    m_lowercaseId3RiffChunk(false)
{
  m_disabledPlugins << QLatin1String("Id3libMetadata")
                    << QLatin1String("Mp4v2Metadata");
}

/**
 * Destructor.
 */
TagConfig::~TagConfig()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void TagConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("MarkTruncations"),
                   QVariant(m_markTruncations));
  config->setValue(QLatin1String("MarkOversizedPictures"),
                   QVariant(m_markOversizedPictures));
  config->setValue(QLatin1String("MaximumPictureSize"),
                   QVariant(m_maximumPictureSize));
  config->setValue(QLatin1String("MarkStandardViolations"),
                   QVariant(m_markStandardViolations));
  config->setValue(QLatin1String("EnableTotalNumberOfTracks"),
                   QVariant(m_enableTotalNumberOfTracks));
  config->setValue(QLatin1String("GenreNotNumeric"),
                   QVariant(m_genreNotNumeric));
  config->setValue(QLatin1String("LowercaseId3RiffChunk"),
                   QVariant(m_lowercaseId3RiffChunk));
  config->setValue(QLatin1String("CommentName"),
                   QVariant(m_commentName));
  config->setValue(QLatin1String("PictureNameItem"),
                   QVariant(m_pictureNameItem));
  config->setValue(QLatin1String("RiffTrackName"),
                   QVariant(m_riffTrackName));
  config->setValue(QLatin1String("CustomGenres"),
                   QVariant(m_customGenres));
  config->setValue(QLatin1String("CustomFrames"),
                   QVariant(m_customFrames));
  config->setValue(QLatin1String("ID3v2Version"),
                   QVariant(m_id3v2Version));
  config->setValue(QLatin1String("TextEncodingV1"),
                   QVariant(m_textEncodingV1));
  config->setValue(QLatin1String("TextEncoding"),
                   QVariant(m_textEncoding));
#ifdef Q_OS_MAC
  // Convince Mac OS X to store a 64-bit value.
  config->setValue(QLatin1String("QuickAccessFrames"),
                   QVariant(m_quickAccessFrames | (Q_UINT64_C(1) << 63)));
#else
  config->setValue(QLatin1String("QuickAccessFrames"),
                   QVariant(m_quickAccessFrames));
#endif
  config->setValue(QLatin1String("QuickAccessFrameOrder"),
                   QVariant(intListToStringList(m_quickAccessFrameOrder)));
  config->setValue(QLatin1String("TrackNumberDigits"),
                   QVariant(m_trackNumberDigits));
  config->setValue(QLatin1String("OnlyCustomGenres"),
                   QVariant(m_onlyCustomGenres));
  config->setValue(QLatin1String("PluginOrder"),
                   QVariant(m_pluginOrder));
  config->setValue(QLatin1String("DisabledPlugins"),
                   QVariant(m_disabledPlugins));
  config->setValue(QLatin1String("StarRatingMapping"),
                   QVariant(m_starRatingMapping->toStringList()));
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
  m_markTruncations = config->value(QLatin1String("MarkTruncations"),
                                    m_markTruncations).toBool();
  m_markOversizedPictures = config->value(QLatin1String("MarkOversizedPictures"),
                                          m_markOversizedPictures).toBool();
  m_maximumPictureSize = config->value(QLatin1String("MaximumPictureSize"),
                                       m_maximumPictureSize).toInt();
  m_markStandardViolations =
      config->value(QLatin1String("MarkStandardViolations"),
                    m_markStandardViolations).toBool();
  m_enableTotalNumberOfTracks =
      config->value(QLatin1String("EnableTotalNumberOfTracks"),
                    m_enableTotalNumberOfTracks).toBool();
  m_genreNotNumeric = config->value(QLatin1String("GenreNotNumeric"),
                                    m_genreNotNumeric).toBool();
  m_lowercaseId3RiffChunk = config->value(QLatin1String("LowercaseId3RiffChunk"),
                                          m_lowercaseId3RiffChunk).toBool();
  m_commentName =
      config->value(QLatin1String("CommentName"),
                    QString::fromLatin1(defaultCommentName)).toString();
  m_pictureNameItem = config->value(QLatin1String("PictureNameItem"),
                                    VP_METADATA_BLOCK_PICTURE).toInt();
  m_riffTrackName =
      config->value(QLatin1String("RiffTrackName"),
                    QString::fromLatin1(defaultRiffTrackName)).toString();
  m_customGenres = config->value(QLatin1String("CustomGenres"),
                                 m_customGenres).toStringList();
  m_customFrames = config->value(QLatin1String("CustomFrames"),
                                 m_customFrames).toStringList();
  m_id3v2Version = config->value(QLatin1String("ID3v2Version"),
                                 ID3v2_3_0).toInt();
  m_textEncodingV1 = config->value(QLatin1String("TextEncodingV1"),
                                   QLatin1String("ISO-8859-1")).toString();
  m_textEncoding = config->value(QLatin1String("TextEncoding"),
                                 TE_ISO8859_1).toInt();
  m_quickAccessFrames =
      config->value(QLatin1String("QuickAccessFrames"),
                    FrameCollection::DEFAULT_QUICK_ACCESS_FRAMES).toULongLong();
#ifdef Q_OS_MAC
  m_quickAccessFrames &= ~(Q_UINT64_C(1) << 63);
#endif
  m_quickAccessFrameOrder = stringListToIntList(
        config->value(QLatin1String("QuickAccessFrameOrder"), QStringList())
        .toStringList());
  m_trackNumberDigits = config->value(QLatin1String("TrackNumberDigits"),
                                      1).toInt();
  m_onlyCustomGenres = config->value(QLatin1String("OnlyCustomGenres"),
                                     m_onlyCustomGenres).toBool();
  m_pluginOrder = config->value(QLatin1String("PluginOrder"),
                                 m_pluginOrder).toStringList();
  m_disabledPlugins = config->value(QLatin1String("DisabledPlugins"),
                                 m_disabledPlugins).toStringList();
  m_starRatingMapping->fromStringList(
        config->value(QLatin1String("StarRatingMapping"),
                      QStringList()).toStringList());
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
    nullptr
  };

  m_pluginOrder.clear();
  for (const char* const* pn = defaultPluginOrder; *pn != nullptr; ++pn) {
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

/** Set true to mark standard violations. */
void TagConfig::setMarkStandardViolations(bool markStandardViolations)
{
  if (m_markStandardViolations != markStandardViolations) {
    m_markStandardViolations = markStandardViolations;
    emit markStandardViolationsChanged(m_markStandardViolations);
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

/** Set true to use "id3 " instead of "ID3 " chunk names in WAV files */
void TagConfig::setLowercaseId3RiffChunk(bool lowercaseId3RiffChunk)
{
  if (m_lowercaseId3RiffChunk != lowercaseId3RiffChunk) {
    m_lowercaseId3RiffChunk = lowercaseId3RiffChunk;
    emit lowercaseId3RiffChunkChanged(m_lowercaseId3RiffChunk);
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
void TagConfig::setPictureNameIndex(int pictureNameIndex)
{
  if (m_pictureNameItem != pictureNameIndex) {
    m_pictureNameItem = pictureNameIndex;
    emit pictureNameIndexChanged(m_pictureNameItem);
  }
}

/** Set field name used for RIFF track entries. */
void TagConfig::setRiffTrackName(const QString& riffTrackName)
{
  if (m_riffTrackName != riffTrackName) {
    m_riffTrackName = riffTrackName;
    emit riffTrackNameChanged(m_riffTrackName);
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

/** Set custom frame names. */
void TagConfig::setCustomFrames(const QStringList& customFrames)
{
  if (m_customFrames != customFrames) {
    m_customFrames = customFrames;
    emit customFramesChanged(m_customFrames);
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

/** index of ID3v1 text encoding in getTextCodecNames() */
int TagConfig::textEncodingV1Index() const
{
  return indexFromTextCodecName(m_textEncodingV1);
}

/** Set ID3v1 text encoding from index in getTextCodecNames(). */
void TagConfig::setTextEncodingV1Index(int index)
{
  if (QString encoding = indexToTextCodecName(index); !encoding.isNull()) {
    setTextEncodingV1(encoding);
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
 * Get list of star count rating mappings.
 * @return star count rating mappings as a list of strings.
 */
QStringList TagConfig::starRatingMappingStrings() const
{
  return m_starRatingMapping->toStringList();
}

/**
 * Set list of star count rating mappings.
 * @param mappings star count rating mappings
 */
void TagConfig::setStarRatingMappingStrings(const QStringList& mappings)
{
  if (m_starRatingMapping->toStringList() != mappings) {
    m_starRatingMapping->fromStringList(mappings);
    emit starRatingMappingsChanged();
  }
}

/**
 * Get list of star count rating mappings.
 * @return star count rating mappings.
 */
const QList<QPair<QString, QVector<int> > >& TagConfig::starRatingMappings() const
{
  return m_starRatingMapping->getMappings();
}

/**
 * Set list of star count rating mappings.
 * @param maps star count rating mappings
 */
void TagConfig::setStarRatingMappings(
    const QList<QPair<QString, QVector<int> > >& maps)
{
  if (m_starRatingMapping->getMappings() != maps) {
    m_starRatingMapping->setMappings(maps);
    emit starRatingMappingsChanged();
  }
}

/**
 * Get star count from rating value.
 * @param rating rating value stored in tag frame
 * @param type rating type containing frame name and optionally field value,
 * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
 * @return number of stars (1..5).
 */
int TagConfig::starCountFromRating(int rating, const QString& type) const
{
  return m_starRatingMapping->starCountFromRating(rating, type);
}

/**
 * Get rating value from star count.
 * @param starCount number of stars (1..5)
 * @param type rating type containing frame name and optionally field value,
 * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
 * @return rating value stored in tag frame, usually a value between 1 and 255
 * or 1 and 100.
 */
int TagConfig::starCountToRating(int starCount, const QString& type) const
{
  return m_starRatingMapping->starCountToRating(starCount, type);
}

/**
 * Get default value for Email field in POPM frame.
 * @return value for Email field in first POPM entry of star rating mappings.
 */
QString TagConfig::defaultPopmEmail() const
{
  return m_starRatingMapping->defaultPopmEmail();
}

/**
 * String list of encodings for ID3v2.
 */
QStringList TagConfig::getTextEncodingNames()
{
  static constexpr int NUM_NAMES = 3;
  static const char* const names[NUM_NAMES] = {
    QT_TRANSLATE_NOOP("@default", "ISO-8859-1"),
    QT_TRANSLATE_NOOP("@default", "UTF16"),
    QT_TRANSLATE_NOOP("@default", "UTF8")
  };
  QStringList strs;
  strs.reserve(NUM_NAMES);
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
  return {QLatin1String("ID3v2.3.0"), QLatin1String("ID3v2.4.0")};
}

/**
 * String list with suggested field names used for Vorbis comment entries.
 */
QStringList TagConfig::getCommentNames()
{
  return {QLatin1String("COMMENT"), QLatin1String("DESCRIPTION")};
}

/**
 * String list with possible field names used for Vorbis picture entries.
 */
QStringList TagConfig::getPictureNames()
{
  return {QLatin1String("METADATA_BLOCK_PICTURE"), QLatin1String("COVERART")};
}

/**
 * String list with suggested field names used for RIFF track entries.
 */
QStringList TagConfig::getRiffTrackNames()
{
  return {QLatin1String("IPRT"), QLatin1String("ITRK"), QLatin1String("TRCK")};
}

/**
 * Available and selected quick access frames.
 */
QVariantList TagConfig::selectedQuickAccessFrames() const {
  return getQuickAccessFrameSelection(
        quickAccessFrameOrder(), quickAccessFrames(),
        customFrameNamesToDisplayNames(customFrames()));
}

/**
 * Set selected quick access frames.
 * @param namesSelected list of maps with name, selected and type fields
 */
void TagConfig::setSelectedQuickAccessFrames(
    const QVariantList& namesSelected) {
  QList<int> frameTypes;
  quint64 frameMask = 0;
  setQuickAccessFrameSelection(namesSelected, frameTypes, frameMask);
  setQuickAccessFrameOrder(frameTypes);
  setQuickAccessFrames(frameMask);
}

/**
 * Get the available and selected quick access frames.
 * @param types ordered frame types as in quickAccessFrameOrder()
 * @param frameMask quick access frame selection as in quickAccessFrames()
 * @param customFrameNames list of custom frame names as in customFrames()
 * @return list of name/type/selected maps.
 */
QVariantList TagConfig::getQuickAccessFrameSelection(
    const QList<int>& types, quint64 frameMask,
    const QStringList& customFrameNames)
{
  QList frameTypes(types);
  if (frameTypes.size() < Frame::FT_Custom1) {
    frameTypes.clear();
    frameTypes.reserve(Frame::FT_LastFrame - Frame::FT_FirstFrame + 1);
    for (int i = Frame::FT_FirstFrame; i <= Frame::FT_LastFrame; ++i) {
      frameTypes.append(i);
    }
  } else {
    for (int i = frameTypes.size(); i <= Frame::FT_LastFrame; ++i) {
      frameTypes.append(i);
    }
  }
  QVariantList namesSelected;
  const auto constFrameTypes = frameTypes;
  for (int frameType : constFrameTypes) {
    auto name = Frame::ExtendedType(static_cast<Frame::Type>(frameType))
        .getTranslatedName();
    if (Frame::isCustomFrameType(static_cast<Frame::Type>(frameType))) {
      if (int idx = frameType - Frame::FT_Custom1;
          idx >= 0 && idx < customFrameNames.size()) {
        name = customFrameNames.at(idx);
      } else {
        name.clear();
      }
    }
    if (!name.isEmpty()) {
      const bool selected = (frameMask & (1ULL << frameType)) != 0ULL;
      namesSelected.append(
            QVariantMap{{QLatin1String("name"), name},
                        {QLatin1String("type"), frameType},
                        {QLatin1String("selected"), selected}});
    }
  }
  return namesSelected;
}

/**
 * Set the selected quick access frames.
 * @param namesSelected list of name/type/selected maps
 * @param frameTypes ordered frame types are returned here,
 *        suitable for setQuickAccessFrameOrder()
 * @param frameMask the quick access frame selection is returned here,
 *        suitable for setQuickAccessFrames()
 */
void TagConfig::setQuickAccessFrameSelection(
    const QVariantList& namesSelected,
    QList<int>& frameTypes, quint64& frameMask)
{
  bool isStandardFrameOrder = true;
  const int numQuickAccessTags = namesSelected.size();
  frameTypes.clear();
  frameTypes.reserve(numQuickAccessTags);
  frameMask = 0;
  for (int row = 0; row < numQuickAccessTags; ++row) {
    auto map = namesSelected.at(row).toMap();
    auto frameType = map.value(QLatin1String("type")).toInt();
    auto selected = map.value(QLatin1String("selected")).toBool();
    if (frameType != row) {
      isStandardFrameOrder = false;
    }
    frameTypes.append(frameType);
    if (selected) {
      frameMask |= 1ULL << frameType;
    }
  }
  if (isStandardFrameOrder) {
    frameTypes.clear();
  }
}

/**
 * Convert list of custom frame names to display names.
 * @param names custom frame names
 * @return possibly translated display representations of @a names.
 */
QStringList TagConfig::customFrameNamesToDisplayNames(const QStringList& names)
{
  QStringList displayNames;
  for (const QString& name : names) {
    displayNames.append(Frame::getDisplayName(name));
  }
  return displayNames;
}

/**
 * Convert list of display names to custom frame names.
 * @param displayNames displayed frame names
 * @return internal representations of @a displayNames.
 */
QStringList TagConfig::customFrameNamesFromDisplayNames(
    const QStringList& displayNames)
{
  QStringList names;
  for (const QString& displayName : displayNames) {
    QByteArray frameId = Frame::getFrameIdForTranslatedFrameName(displayName);
    names.append(frameId.isNull()
                 ? Frame::getNameForTranslatedFrameName(displayName)
                 : QString::fromLatin1(frameId));
  }
  return names;
}
