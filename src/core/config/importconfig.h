/**
 * \file importconfig.h
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#pragma once

#include <QStringList>
#include "config.h"
#include "generalconfig.h"
#include "frame.h"

/**
 * Import configuration.
 */
class KID3_CORE_EXPORT ImportConfig : public StoredConfig<ImportConfig> {
  Q_OBJECT
  /** available plugins, this information is not stored in the configuration */
  Q_PROPERTY(QStringList availablePlugins READ availablePlugins
             WRITE setAvailablePlugins NOTIFY availablePluginsChanged)
  /** import server */
  Q_PROPERTY(int importServer READ importServer WRITE setImportServer
             NOTIFY importServerChanged)
  /** tag version to import */
  Q_PROPERTY(int importDest READ importDest WRITE setImportDestInt
             NOTIFY importDestChanged)
  /** Names of import formats */
  Q_PROPERTY(QStringList importFormatNames READ importFormatNames
             WRITE setImportFormatNames NOTIFY importFormatNamesChanged)
  /** regexp describing header import format */
  Q_PROPERTY(QStringList importFormatHeaders READ importFormatHeaders
             WRITE setImportFormatHeaders NOTIFY importFormatHeadersChanged)
  /** regexp describing track import format */
  Q_PROPERTY(QStringList importFormatTracks READ importFormatTracks
             WRITE setImportFormatTracks NOTIFY importFormatTracksChanged)
  /** selected import format */
  Q_PROPERTY(int importFormatIndex READ importFormatIndex
             WRITE setImportFormatIndex NOTIFY importFormatIndexChanged)
  /** maximum allowable time difference */
  Q_PROPERTY(int maxTimeDifference READ maxTimeDifference
             WRITE setMaxTimeDifference NOTIFY maxTimeDifferenceChanged)
  /** visible optional columns in import table */
  Q_PROPERTY(quint64 importVisibleColumns READ importVisibleColumns
             WRITE setImportVisibleColumns NOTIFY importVisibleColumnsChanged)
  /** import window geometry */
  Q_PROPERTY(QByteArray importWindowGeometry READ importWindowGeometry
             WRITE setImportWindowGeometry NOTIFY importWindowGeometryChanged)
  /** Names of import tags formats */
  Q_PROPERTY(QStringList importTagsNames READ importTagsNames
             WRITE setImportTagsNames NOTIFY importTagsNamesChanged)
  /** Expressions for tag import sources */
  Q_PROPERTY(QStringList importTagsSources READ importTagsSources
             WRITE setImportTagsSources NOTIFY importTagsSourcesChanged)
  /** regexp describing extraction from import tag sources */
  Q_PROPERTY(QStringList importTagsExtractions READ importTagsExtractions
             WRITE setImportTagsExtractions NOTIFY importTagsExtractionsChanged)
  /** selected import tags format */
  Q_PROPERTY(int importTagsIndex READ importTagsIndex WRITE setImportTagsIndex
             NOTIFY importTagsIndexChanged)
  /** names of picture sources */
  Q_PROPERTY(QStringList pictureSourceNames READ pictureSourceNames
             WRITE setPictureSourceNames NOTIFY pictureSourceNamesChanged)
  /** picture source URLs */
  Q_PROPERTY(QStringList pictureSourceUrls READ pictureSourceUrls
             WRITE setPictureSourceUrls NOTIFY pictureSourceUrlsChanged)
  /** selected picture source */
  Q_PROPERTY(int pictureSourceIndex READ pictureSourceIndex
             WRITE setPictureSourceIndex NOTIFY pictureSourceIndexChanged)
  /** Browse cover art window geometry */
  Q_PROPERTY(QByteArray browseCoverArtWindowGeometry READ browseCoverArtWindowGeometry
             WRITE setBrowseCoverArtWindowGeometry NOTIFY browseCoverArtWindowGeometryChanged)
  /** Mapping for picture URL matching */
  Q_PROPERTY(QStringList matchPictureUrlMap READ matchPictureUrlStringList
             WRITE setMatchPictureUrlStringList NOTIFY matchPictureUrlMapChanged)
  /** Last directory used for import or export. */
  Q_PROPERTY(QString importDir READ importDir WRITE setImportDir
             NOTIFY importDirChanged)
  /** Disabled plugins */
  Q_PROPERTY(QStringList disabledPlugins READ disabledPlugins
             WRITE setDisabledPlugins NOTIFY disabledPluginsChanged)
  /** check maximum allowable time difference */
  Q_PROPERTY(bool enableTimeDifferenceCheck READ enableTimeDifferenceCheck
             WRITE setEnableTimeDifferenceCheck NOTIFY enableTimeDifferenceCheckChanged)

public:
  /**
   * Constructor.
   */
  ImportConfig();

  /**
   * Destructor.
   */
  virtual ~ImportConfig() override = default;

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

  /** Get list of available plugins. */
  QStringList availablePlugins() const { return m_availablePlugins; }

  /** Set list of available plugins. */
  void setAvailablePlugins(const QStringList& availablePlugins);

  /** Clear list of available plugins. */
  void clearAvailablePlugins() { m_availablePlugins.clear(); }

  /** Get import server. */
  int importServer() const { return m_importServer; }

  /** Set import server. */
  void setImportServer(int importServer);

  /** Get tag version to import. */
  Frame::TagVersion importDest() const { return m_importDest; }

  /** Set tag version to import. */
  void setImportDest(Frame::TagVersion importDest);

  /** Get names of import formats. */
  QStringList importFormatNames() const { return m_importFormatNames; }

  /** Set names of import formats. */
  void setImportFormatNames(const QStringList& importFormatNames);

  /** Get regexp describing header import format. */
  QStringList importFormatHeaders() const { return m_importFormatHeaders; }

  /** Set regexp describing header import format. */
  void setImportFormatHeaders(const QStringList& importFormatHeaders);

  /** Get regexp describing track import format. */
  QStringList importFormatTracks() const { return m_importFormatTracks; }

  /** Set regexp describing track import format. */
  void setImportFormatTracks(const QStringList& importFormatTracks);

  /** Get selected import format. */
  int importFormatIndex() const { return m_importFormatIdx; }

  /** Set selected import format. */
  void setImportFormatIndex(int importFormatIndex);

  /** Get maximum allowable time difference. */
  int maxTimeDifference() const { return m_maxTimeDifference; }

  /** Set maximum allowable time difference. */
  void setMaxTimeDifference(int maxTimeDifference);

  /** Get visible optional columns in import table. */
  quint64 importVisibleColumns() const { return m_importVisibleColumns; }

  /** Set visible optional columns in import table. */
  void setImportVisibleColumns(quint64 importVisibleColumns);

  /** Get import window geometry. */
  QByteArray importWindowGeometry() const { return m_importWindowGeometry; }

  /** Set import window geometry. */
  void setImportWindowGeometry(const QByteArray& importWindowGeometry);

  /** Get names of import tags formats. */
  QStringList importTagsNames() const { return m_importTagsNames; }

  /** Set names of import tags formats. */
  void setImportTagsNames(const QStringList& importTagsNames);

  /** Get expressions for tag import sources. */
  QStringList importTagsSources() const { return m_importTagsSources; }

  /** Set expressions for tag import sources. */
  void setImportTagsSources(const QStringList& importTagsSources);

  /** Get regexp describing extraction from import tag sources. */
  QStringList importTagsExtractions() const { return m_importTagsExtractions; }

  /** Set regexp describing extraction from import tag sources. */
  void setImportTagsExtractions(const QStringList& importTagsExtractions);

  /** Get selected import tags format. */
  int importTagsIndex() const { return m_importTagsIdx; }

  /** Set selected import tags format. */
  void setImportTagsIndex(int importTagsIndex);

  /** Get names of picture sources. */
  QStringList pictureSourceNames() const { return m_pictureSourceNames; }

  /** Set names of picture sources. */
  void setPictureSourceNames(const QStringList& pictureSourceNames);

  /** Get picture source URLs. */
  QStringList pictureSourceUrls() const { return m_pictureSourceUrls; }

  /** Set picture source URLs. */
  void setPictureSourceUrls(const QStringList& pictureSourceUrls);

  /** Get selected picture source. */
  int pictureSourceIndex() const { return m_pictureSourceIdx; }

  /** Set selected picture source. */
  void setPictureSourceIndex(int pictureSourceIndex);

  /** Get browse cover art window geometry. */
  QByteArray browseCoverArtWindowGeometry() const {
    return m_browseCoverArtWindowGeometry;
  }

  /** Set browse cover art window geometry. */
  void setBrowseCoverArtWindowGeometry(const QByteArray& browseCoverArtWindowGeometry);

  /** Get mapping for picture URL matching. */
  QList<QPair<QString, QString>> matchPictureUrlMap() const {
    return m_matchPictureUrlMap;
  }

  /** Set mapping for picture URL matching. */
  void setMatchPictureUrlMap(const QList<QPair<QString, QString>>& matchPictureUrlMap);

  /** Get mapping for picture URL matching as list with alternating key, values. */
  QStringList matchPictureUrlStringList() const;

  /** Set mapping for picture URL matching from list with alternating key, values. */
  void setMatchPictureUrlStringList(const QStringList& lst);

  /** Get last directory used for import or export. */
  QString importDir() const { return m_importDir; }

  /** Set last directory used for import or export. */
  void setImportDir(const QString& importDir);

  /** Get disabled plugins */
  QStringList disabledPlugins() const { return m_disabledPlugins; }

  /** Set disabled plugins */
  void setDisabledPlugins(const QStringList& disabledPlugins);

  /** Check if maximum allowable time difference check is enabled. */
  bool enableTimeDifferenceCheck() const { return m_enableTimeDifferenceCheck; }

  /** Set if maximum allowable time difference check is enabled. */
  void setEnableTimeDifferenceCheck(bool enableTimeDifferenceCheck);

signals:
  /** Emitted when @a availablePlugins changed. */
  void availablePluginsChanged(const QStringList& availablePlugins);

  /** Emitted when @a importServer changed. */
  void importServerChanged(int importServer);

  /** Emitted when @a importDest changed. */
  void importDestChanged(Frame::TagVersion importDest);

  /** Emitted when @a importFormatNames changed. */
  void importFormatNamesChanged(const QStringList& importFormatNames);

  /** Emitted when @a importFormatHeaders changed. */
  void importFormatHeadersChanged(const QStringList& importFormatHeaders);

  /** Emitted when @a importFormatTracks changed. */
  void importFormatTracksChanged(const QStringList& importFormatTracks);

  /** Emitted when @a importFormatIdx changed. */
  void importFormatIndexChanged(int importFormatIndex);

  /** Emitted when @a maxTimeDifference changed. */
  void maxTimeDifferenceChanged(int maxTimeDifference);

  /** Emitted when @a importVisibleColumns changed. */
  void importVisibleColumnsChanged(quint64 importVisibleColumns);

  /** Emitted when @a importWindowGeometry changed. */
  void importWindowGeometryChanged(const QByteArray& importWindowGeometry);

  /** Emitted when @a importTagsNames changed. */
  void importTagsNamesChanged(const QStringList& importTagsNames);

  /** Emitted when @a importTagsSources changed. */
  void importTagsSourcesChanged(const QStringList& importTagsSources);

  /** Emitted when @a importTagsExtractions changed. */
  void importTagsExtractionsChanged(const QStringList& importTagsExtractions);

  /** Emitted when @a importTagsIdx changed. */
  void importTagsIndexChanged(int importTagsIndex);

  /** Emitted when @a pictureSourceNames changed. */
  void pictureSourceNamesChanged(const QStringList& pictureSourceNames);

  /** Emitted when @a pictureSourceUrls changed. */
  void pictureSourceUrlsChanged(const QStringList& pictureSourceUrls);

  /** Emitted when @a pictureSourceIdx changed. */
  void pictureSourceIndexChanged(int pictureSourceIndex);

  /** Emitted when @a browseCoverArtWindowGeometry changed. */
  void browseCoverArtWindowGeometryChanged(const QByteArray& browseCoverArtWindowGeometry);

  /** Emitted when @a matchPictureUrlMap changed. */
  void matchPictureUrlMapChanged(const QList<QPair<QString, QString>>& matchPictureUrlMap);

  /** Emitted when @a importDir changed. */
  void importDirChanged(const QString& importDir);

  /** Emitted when @a disabledPlugins changed. */
  void disabledPluginsChanged(const QStringList& disabledPlugins);

  /** Emitted when @a enableTimeDifferenceCheck changed. */
  void enableTimeDifferenceCheckChanged(bool enableTimeDifferenceCheck);

private:
  friend ImportConfig& StoredConfig<ImportConfig>::instance();

  void setImportDestInt(int importDest) {
    setImportDest(Frame::tagVersionCast(importDest));
  }

  int m_importServer;
  Frame::TagVersion m_importDest;
  QStringList m_importFormatNames;
  QStringList m_importFormatHeaders;
  QStringList m_importFormatTracks;
  int m_importFormatIdx;
  int m_maxTimeDifference;
  quint64 m_importVisibleColumns;
  QByteArray m_importWindowGeometry;

  QStringList m_importTagsNames;
  QStringList m_importTagsSources;
  QStringList m_importTagsExtractions;
  int m_importTagsIdx;

  QStringList m_pictureSourceNames;
  QStringList m_pictureSourceUrls;
  int m_pictureSourceIdx;
  QByteArray m_browseCoverArtWindowGeometry;
  QList<QPair<QString, QString>> m_matchPictureUrlMap;

  QString m_importDir;

  QStringList m_disabledPlugins;

  QStringList m_availablePlugins;
  bool m_enableTimeDifferenceCheck;

  /** Index in configuration storage */
  static int s_index;
};
