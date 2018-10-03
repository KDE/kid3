/**
 * \file tagconfig.h
 * Tag related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#ifndef TAGCONFIG_H
#define TAGCONFIG_H

#include <QStringList>
#include <QScopedPointer>
#include "generalconfig.h"
#include "kid3api.h"

class StarRatingMapping;

/**
 * Tag related configuration.
 */
class KID3_CORE_EXPORT TagConfig : public StoredConfig<TagConfig> {
  Q_OBJECT
  /** features provided by metadata plugins */
  Q_PROPERTY(int taggedFileFeatures READ taggedFileFeatures WRITE setTaggedFileFeatures NOTIFY taggedFileFeaturesChanged)
  /** true to mark truncated ID3v1.1 fields */
  Q_PROPERTY(bool markTruncations READ markTruncations WRITE setMarkTruncations NOTIFY markTruncationsChanged)
  /** true to mark oversized pictures */
  Q_PROPERTY(bool markOversizedPictures READ markOversizedPictures WRITE setMarkOversizedPictures NOTIFY markOversizedPicturesChanged)
  /** Maximum size of picture in bytes */
  Q_PROPERTY(int maximumPictureSize READ maximumPictureSize WRITE setMaximumPictureSize NOTIFY maximumPictureSizeChanged)
  /** true to mark standard violations */
  Q_PROPERTY(bool markStandardViolations READ markStandardViolations WRITE setMarkStandardViolations NOTIFY markStandardViolationsChanged)
  /** true to write total number of tracks into track fields */
  Q_PROPERTY(bool enableTotalNumberOfTracks READ enableTotalNumberOfTracks WRITE setEnableTotalNumberOfTracks NOTIFY enableTotalNumberOfTracksChanged)
  /** true to write genres as text instead of numeric string */
  Q_PROPERTY(bool genreNotNumeric READ genreNotNumeric WRITE setGenreNotNumeric NOTIFY genreNotNumericChanged)
  /** true to use "id3 " instead of "ID3 " chunk names in WAV files */
  Q_PROPERTY(bool lowercaseId3RiffChunk READ lowercaseId3RiffChunk WRITE setLowercaseId3RiffChunk NOTIFY lowercaseId3RiffChunkChanged)
  /** field name used for Vorbis comment entries */
  Q_PROPERTY(QString commentName READ commentName WRITE setCommentName NOTIFY commentNameChanged)
  /** index of field name used for Vorbis picture entries */
  Q_PROPERTY(int pictureNameIndex READ pictureNameIndex WRITE setPictureNameIndex NOTIFY pictureNameIndexChanged)
  /** field name used for RIFF track entries */
  Q_PROPERTY(QString riffTrackName READ riffTrackName WRITE setRiffTrackName NOTIFY riffTrackNameChanged)
  /** custom genres for ID3v2.3 */
  Q_PROPERTY(QStringList customGenres READ customGenres WRITE setCustomGenres NOTIFY customGenresChanged)
  /** version used for new ID3v2 tags */
  Q_PROPERTY(int id3v2Version READ id3v2Version WRITE setId3v2Version NOTIFY id3v2VersionChanged)
  /** text encoding used for new ID3v1 tags */
  Q_PROPERTY(QString textEncodingV1 READ textEncodingV1 WRITE setTextEncodingV1 NOTIFY textEncodingV1Changed)
  /** text encoding used for new ID3v1 tags */
  Q_PROPERTY(int textEncodingV1Index READ textEncodingV1Index WRITE setTextEncodingV1Index NOTIFY textEncodingV1Changed)
  /** text encoding used for new ID3v2 tags */
  Q_PROPERTY(int textEncoding READ textEncoding WRITE setTextEncoding NOTIFY textEncodingChanged)
  /** frames which are displayed for Tag 2 even if not present */
  Q_PROPERTY(quint64 quickAccessFrames READ quickAccessFrames WRITE setQuickAccessFrames NOTIFY quickAccessFramesChanged)
  /** order of frames which are displayed for Tag 2 even if not present */
  Q_PROPERTY(QList<int> quickAccessFrameOrder READ quickAccessFrameOrder WRITE setQuickAccessFrameOrder NOTIFY quickAccessFrameOrderChanged)
  /** number of digits in track number */
  Q_PROPERTY(int trackNumberDigits READ trackNumberDigits WRITE setTrackNumberDigits NOTIFY trackNumberDigitsChanged)
  /** true to show only custom genres in combo boxes */
  Q_PROPERTY(bool onlyCustomGenres READ onlyCustomGenres WRITE setOnlyCustomGenres NOTIFY onlyCustomGenresChanged)
  /** the order in which meta data plugins are tried when opening a file */
  Q_PROPERTY(QStringList pluginOrder READ pluginOrder WRITE setPluginOrder NOTIFY pluginOrderChanged)
  /** disabled plugins */
  Q_PROPERTY(QStringList disabledPlugins READ disabledPlugins WRITE setDisabledPlugins NOTIFY disabledPluginsChanged)
  /** list of available plugins. */
  Q_PROPERTY(QStringList availablePlugins READ availablePlugins WRITE setAvailablePlugins NOTIFY availablePluginsChanged)
  /** mapping between star count and rating values. */
  Q_PROPERTY(QStringList starRatingMappingStrings READ starRatingMappingStrings WRITE setStarRatingMappingStrings NOTIFY starRatingMappingsChanged)
  /** default value for Email field in POPM frame. */
  Q_PROPERTY(QString defaultPopmEmail READ defaultPopmEmail NOTIFY starRatingMappingsChanged)
  Q_ENUMS(Id3v2Version)
  Q_ENUMS(TextEncoding)
  Q_ENUMS(VorbisPictureName)
public:
  /** The ID3v2 version used for new tags. */
  enum Id3v2Version {
    ID3v2_3_0 = 0,
    ID3v2_4_0 = 1
  };

  /** Encoding used for ID3v2 frames. */
  enum TextEncoding {
    TE_ISO8859_1,
    TE_UTF16,
    TE_UTF8
  };

  /** Name for Vorbis picture. */
  enum VorbisPictureName {
    VP_METADATA_BLOCK_PICTURE,
    VP_COVERART
  };

  /**
   * Constructor.
   */
  TagConfig();

  /**
   * Destructor.
   */
  virtual ~TagConfig() override;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /**
   * Get features provided by metadata plugins.
   * @return bit mask with TaggedFile::Feature flags set.
   * @remark This information is not stored in the configuration, it is
   * registered at initialization time using setTaggedFileFeatures().
   */
  int taggedFileFeatures() const {
    return m_taggedFileFeatures;
  }

  /**
   * Set features provided by metadata plugins.
   * @param taggedFileFeatures bit mask with TaggedFile::Feature flags set
   */
  void setTaggedFileFeatures(int taggedFileFeatures);

  /** true to mark truncated ID3v1.1 fields */
  bool markTruncations() const { return m_markTruncations; }

  /** Set true to mark truncated ID3v1.1 fields. */
  void setMarkTruncations(bool markTruncations);

  /** true to mark oversized pictures */
  bool markOversizedPictures() const { return m_markOversizedPictures; }

  /** Set true to mark oversized pictures. */
  void setMarkOversizedPictures(bool markOversizedPictures);

  /** Maximum size of picture in bytes */
  int maximumPictureSize() const { return m_maximumPictureSize; }

  /** Set maximum size of picture in bytes. */
  void setMaximumPictureSize(int maximumPictureSize);

  /** true to mark standard violations */
  bool markStandardViolations() const { return m_markStandardViolations; }

  /** Set true to mark standard violations. */
  void setMarkStandardViolations(bool markStandardViolations);

  /** true to write total number of tracks into track fields */
  bool enableTotalNumberOfTracks() const { return m_enableTotalNumberOfTracks; }

  /** Set true to write total number of tracks into track fields. */
  void setEnableTotalNumberOfTracks(bool enableTotalNumberOfTracks);

  /** true to write genres as text instead of numeric string */
  bool genreNotNumeric() const { return m_genreNotNumeric; }

  /** Set true to write genres as text instead of numeric string. */
  void setGenreNotNumeric(bool genreNotNumeric);

  /** true to use "id3 " instead of "ID3 " chunk names in WAV files */
  bool lowercaseId3RiffChunk() const { return m_lowercaseId3RiffChunk; }

  /** Set true to use "id3 " instead of "ID3 " chunk names in WAV files */
  void setLowercaseId3RiffChunk(bool lowercaseId3RiffChunk);

  /** field name used for Vorbis comment entries */
  QString commentName() const { return m_commentName; }

  /** Set field name used for Vorbis comment entries. */
  void setCommentName(const QString& commentName);

  /** index of field name used for Vorbis picture entries */
  int pictureNameIndex() const { return m_pictureNameItem; }

  /** Set index of field name used for Vorbis picture entries. */
  void setPictureNameIndex(int pictureNameIndex);

  /** field name used for RIFF track entries */
  QString riffTrackName() const { return m_riffTrackName; }

  /** Set field name used for RIFF track entries. */
  void setRiffTrackName(const QString& riffTrackName);

  /** custom genres for ID3v2.3 */
  QStringList customGenres() const { return m_customGenres; }

  /** Set custom genres for ID3v2.3. */
  void setCustomGenres(const QStringList& customGenres);

  /** version used for new ID3v2 tags */
  int id3v2Version() const;

  /** Set version used for new ID3v2 tags. */
  void setId3v2Version(int id3v2Version);

  /** text encoding used for new ID3v1 tags */
  QString textEncodingV1() const { return m_textEncodingV1; }

  /** Set text encoding used for new ID3v1 tags. */
  void setTextEncodingV1(const QString& textEncodingV1);

  /** index of ID3v1 text encoding in getTextCodecNames() */
  int textEncodingV1Index() const;

  /** Set ID3v1 text encoding from index in getTextCodecNames(). */
  void setTextEncodingV1Index(int index);

  /** text encoding used for new ID3v2 tags */
  int textEncoding() const { return m_textEncoding; }

  /** Set text encoding used for new ID3v2 tags. */
  void setTextEncoding(int textEncoding);

  /** frames which are displayed for Tag 2 even if not present */
  quint64 quickAccessFrames() const {
    return m_quickAccessFrames;
  }

  /** Set frames which are displayed for Tag 2 even if not present. */
  void setQuickAccessFrames(quint64 quickAccessFrames);

  /** order of frames which are displayed for Tag 2 even if not present. */
  QList<int> quickAccessFrameOrder() const {
    return m_quickAccessFrameOrder;
  }

  /** Set order of frames which are displayed for Tag 2 even if not present. */
  void setQuickAccessFrameOrder(const QList<int>& frameTypes);

  /** number of digits in track number */
  int trackNumberDigits() const { return m_trackNumberDigits; }

  /** Set number of digits in track number. */
  void setTrackNumberDigits(int trackNumberDigits);

  /** true to show only custom genres in combo boxes */
  bool onlyCustomGenres() const { return m_onlyCustomGenres; }

  /** Set true to show only custom genres in combo boxes. */
  void setOnlyCustomGenres(bool onlyCustomGenres);

  /** The order in which meta data plugins are tried when opening a file */
  QStringList pluginOrder() const { return m_pluginOrder; }

  /** Set the order in which meta data plugins are tried when opening a file. */
  void setPluginOrder(const QStringList& pluginOrder);

  /** Disabled plugins */
  QStringList disabledPlugins() const { return m_disabledPlugins; }

  /** Set list of disabled plugins. */
  void setDisabledPlugins(const QStringList& disabledPlugins);

  /**
   * Get list of available plugins.
   * @return available plugins.
   */
  QStringList availablePlugins() const { return m_availablePlugins; }

  /**
   * Set list of available plugins.
   * @param availablePlugins available plugins
   */
  void setAvailablePlugins(const QStringList& availablePlugins);

  /**
   * Clear list of available plugins.
   */
  void clearAvailablePlugins() { m_availablePlugins.clear(); }

  /**
   * Get list of star count rating mappings.
   * @return star count rating mappings as a list of strings.
   */
  QStringList starRatingMappingStrings() const;

  /**
   * Set list of star count rating mappings.
   * @param mappings star count rating mappings
   */
  void setStarRatingMappingStrings(const QStringList& mappings);

  /**
   * Get list of star count rating mappings.
   * @return star count rating mappings.
   */
  const QList<QPair<QString, QVector<int> > >& starRatingMappings() const;

  /**
   * Set list of star count rating mappings.
   * @param maps star count rating mappings
   */
  void setStarRatingMappings(const QList<QPair<QString, QVector<int> > >& maps);

  /**
   * Get star count from rating value.
   * @param rating rating value stored in tag frame
   * @param type rating type containing frame name and optionally field value,
   * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
   * @return number of stars (1..5).
   */
  Q_INVOKABLE int starCountFromRating(int rating, const QString& type) const;

  /**
   * Get rating value from star count.
   * @param starCount number of stars (1..5)
   * @param type rating type containing frame name and optionally field value,
   * e.g. "POPM.Windows Media Player 9 Series" or "RATING"
   * @return rating value stored in tag frame, usually a value between 1 and 255
   * or 1 and 100.
   */
  Q_INVOKABLE int starCountToRating(int starCount, const QString& type) const;

  /**
   * Get default value for Email field in POPM frame.
   * @return value for Email field in first POPM entry of star rating mappings.
   */
  QString defaultPopmEmail() const;

  /**
   * String list of encodings for ID3v2.
   */
  Q_INVOKABLE static QStringList getTextEncodingNames();

  /**
   * String list of possible versions used for new ID3v2 tags.
   */
  Q_INVOKABLE static QStringList getId3v2VersionNames();

  /**
   * String list with suggested field names used for Vorbis comment entries.
   */
  Q_INVOKABLE static QStringList getCommentNames();

  /**
   * String list with possible field names used for Vorbis picture entries.
   */
  Q_INVOKABLE static QStringList getPictureNames();

  /**
   * String list with suggested field names used for RIFF track entries.
   */
  Q_INVOKABLE static QStringList getRiffTrackNames();

  /**
   * Set default plugin order.
   */
  void setDefaultPluginOrder();

signals:
  /** Emitted when @a taggedFileFeatures changed. */
  void taggedFileFeaturesChanged(int taggedFileFeatures);

  /** Emitted when @a markOversizedPictures changed. */
  void markTruncationsChanged(bool markTruncations);

  /** Emitted when @a maximumPictureSize changed. */
  void maximumPictureSizeChanged(int maximumPictureSize);

  /** Emitted when @a markTruncations changed. */
  void markOversizedPicturesChanged(bool markOversizedPictures);

  /** Emitted when @a markStandardViolations changed. */
  void markStandardViolationsChanged(bool markStandardViolations);

  /** Emitted when @a enableTotalNumberOfTracks changed. */
  void enableTotalNumberOfTracksChanged(bool enableTotalNumberOfTracks);

  /** Emitted when @a genreNotNumeric changed. */
  void genreNotNumericChanged(bool genreNotNumeric);

  /** Emitted when @a lowercaseId3RiffChunk changed. */
  void lowercaseId3RiffChunkChanged(bool lowercaseId3RiffChunk);

  /** Emitted when @a commentName changed. */
  void commentNameChanged(const QString& commentName);

  /** Emitted when @a pictureNameIndex changed. */
  void pictureNameIndexChanged(int pictureNameIndex);

  /** Emitted when @a riffTrackName changed. */
  void riffTrackNameChanged(const QString& riffTrackName);

  /** Emitted when @a customGenres changed. */
  void customGenresChanged(const QStringList& customGenres);

  /** Emitted when @a id3v2Version changed. */
  void id3v2VersionChanged(int id3v2Version);

  /** Emitted when @a textEncodingV1 changed. */
  void textEncodingV1Changed(const QString& textEncodingV1);

  /** Emitted when @a textEncoding changed. */
  void textEncodingChanged(int textEncoding);

  /** Emitted when @a quickAccessFrames changed. */
  void quickAccessFramesChanged(quint64 quickAccessFrames);

  /** Emitted when @a quickAccessFrameOrder changed. */
  void quickAccessFrameOrderChanged(const QList<int>& frameTypes);

  /** Emitted when @a  changed. */
  void trackNumberDigitsChanged(int trackNumberDigits);

  /** Emitted when @a onlyCustomGenres changed. */
  void onlyCustomGenresChanged(bool onlyCustomGenres);

  /** Emitted when @a pluginOrder changed. */
  void pluginOrderChanged(const QStringList& pluginOrder);

  /** Emitted when @a disabledPlugins changed. */
  void disabledPluginsChanged(const QStringList& disabledPlugins);

  /** Emitted when @a availablePlugins changed. */
  void availablePluginsChanged(const QStringList& availablePlugins);

  /** Emitted when star count rating mappings changed. */
  void starRatingMappingsChanged();

private:
  friend TagConfig& StoredConfig<TagConfig>::instance();

  QScopedPointer<StarRatingMapping> m_starRatingMapping;

  QString m_commentName;
  QString m_riffTrackName;
  int m_pictureNameItem;
  QStringList m_customGenres;
  int m_id3v2Version;
  QString m_textEncodingV1;
  int m_textEncoding;
  quint64 m_quickAccessFrames;
  QList<int> m_quickAccessFrameOrder;
  int m_trackNumberDigits;
  QStringList m_pluginOrder;
  QStringList m_disabledPlugins;
  QStringList m_availablePlugins;
  int m_taggedFileFeatures;
  int m_maximumPictureSize;
  bool m_markOversizedPictures;
  bool m_markStandardViolations;
  bool m_onlyCustomGenres;
  bool m_markTruncations;
  bool m_enableTotalNumberOfTracks;
  bool m_genreNotNumeric;
  bool m_lowercaseId3RiffChunk;

  /** Index in configuration storage */
  static int s_index;
};

#endif
