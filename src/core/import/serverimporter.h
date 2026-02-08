/**
 * \file serverimporter.h
 * Generic baseclass to import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2024  Urs Fleisch
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

#include <QString>
#include "standardtablemodel.h"
#include "importclient.h"

class ServerImporterConfig;
class ImportClient;
class TrackDataModel;

/**
 * Model containing list of albums which can be imported.
 */
class KID3_CORE_EXPORT AlbumListModel : public StandardTableModel {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit AlbumListModel(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  ~AlbumListModel() override;

  /**
   * Get an album item.
   * @param row model row
   * @param text the text is returned here
   * @param category the category is returned here
   * @param id the internal ID is returned here
   */
  void getItem(int row, QString& text, QString& category, QString& id) const;

  /**
   * Append an album item.
   * @param text display test
   * @param category category, e.g. "release"
   * @param id internal ID
   */
  void appendItem(const QString& text,
                  const QString& category, const QString& id);
};

/**
 * Generic baseclass to import from an external source.
 */
class KID3_CORE_EXPORT ServerImporter : public ImportClient {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param netMgr network access manager
   * @param trackDataModel track data to be filled with imported values
   */
  ServerImporter(QNetworkAccessManager* netMgr,
                 TrackDataModel *trackDataModel);

  /**
   * Destructor.
   */
  ~ServerImporter() override = default;

  /**
   * Name of import source.
   * @return name.
   */
  virtual const char* name() const = 0;

  /** NULL-terminated array of server strings, 0 if not used */
  virtual const char** serverList() const;

  /** default server, 0 to disable */
  virtual const char* defaultServer() const;

  /** default CGI path, 0 to disable */
  virtual const char* defaultCgiPath() const;

  /** anchor to online help, 0 to disable */
  virtual const char* helpAnchor() const;

  /** configuration, 0 if not used */
  virtual ServerImporterConfig* config() const;

  /** additional tags option, false if not used */
  virtual bool additionalTags() const;

  /**
   * Parse result of find request and populate m_albumListBox with results.
   * This method has to be reimplemented for the specific result data.
   *
   * @param searchStr search data received
   */
  virtual void parseFindResults(const QByteArray& searchStr) = 0;

  /**
   * Parse result of album request and populate m_trackDataModel with results.
   * This method has to be reimplemented for the specific result data.
   *
   * @param albumStr album data received
   */
  virtual void parseAlbumResults(const QByteArray& albumStr) = 0;

  /**
   * Get model with album list.
   *
   * @return album list item model.
   */
  AlbumListModel* getAlbumListModel() const { return m_albumListModel; }

  /**
   * Clear model data.
   */
  void clear();

  /**
   * Get standard tags option.
   *
   * @return true if standard tags are enabled.
   */
  bool getStandardTags() const { return m_standardTagsEnabled; }

  /**
   * Set standard tags option.
   *
   * @param enable true if standard tags are enabled
   */
  void setStandardTags(bool enable) { m_standardTagsEnabled = enable; }

  /**
   * Get additional tags option.
   *
   * @return true if additional tags are enabled.
   */
  bool getAdditionalTags() const { return m_additionalTagsEnabled; }

  /**
   * Set additional tags option.
   *
   * @param enable true if additional tags are enabled
   */
  void setAdditionalTags(bool enable) { m_additionalTagsEnabled = enable; }

  /**
   * Get cover art option.
   *
   * @return true if cover art are enabled.
   */
  bool getCoverArt() const { return m_coverArtEnabled; }

  /**
   * Set cover art option.
   *
   * @param enable true if cover art are enabled
   */
  void setCoverArt(bool enable) { m_coverArtEnabled = enable; }

  /**
   * Replace HTML entities in a string.
   *
   * @param str string with HTML entities (e.g. &quot;)
   *
   * @return string with replaced HTML entities.
   */
  static QString replaceHtmlEntities(QString str);

  /**
   * Replace HTML entities and remove HTML tags.
   *
   * @param str string containing HTML
   *
   * @return clean up string
   */
  static QString removeHtml(QString str);

protected:
  AlbumListModel* m_albumListModel; /**< albums to select */
  TrackDataModel* m_trackDataModel; /**< model with tracks to import */

private:
  bool m_standardTagsEnabled;
  bool m_additionalTagsEnabled;
  bool m_coverArtEnabled;
};
