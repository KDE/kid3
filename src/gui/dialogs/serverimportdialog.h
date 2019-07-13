/**
 * \file serverimportdialog.h
 * Generic dialog to import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2018  Urs Fleisch
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

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QComboBox;
class QPushButton;
class QCheckBox;
class QStatusBar;
class QListView;
class ServerImporter;
class ServerImporterConfig;
class ImportTrackDataVector;

/**
 * Generic dialog to import from an external source.
 */
class ServerImportDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent  parent widget
   */
  explicit ServerImportDialog(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~ServerImportDialog() override = default;

  /**
   * Set importer to be used.
   *
   * @param source  import source to use
   */
  void setImportSource(ServerImporter* source);

  /**
   * Get string with server and port.
   *
   * @return "servername:port".
   */
  QString getServer() const;

  /**
   * Set string with server and port.
   *
   * @param srv "servername:port"
   */
  void setServer(const QString& srv);

  /**
   * Get string with CGI path.
   *
   * @return CGI path, e.g. "/~cddb/cddb.cgi".
   */
  QString getCgiPath() const;

  /**
   * Set string with CGI path.
   *
   * @param cgi CGI path, e.g. "/~cddb/cddb.cgi".
   */
  void setCgiPath(const QString& cgi);

  /**
   * Get standard tags option.
   *
   * @return true if standard tags are enabled.
   */
  bool getStandardTags() const;

  /**
   * Set standard tags option.
   *
   * @param enable true if standard tags are enabled
   */
  void setStandardTags(bool enable);

  /**
   * Get additional tags option.
   *
   * @return true if additional tags are enabled.
   */
  bool getAdditionalTags() const;

  /**
   * Set additional tags option.
   *
   * @param enable true if additional tags are enabled
   */
  void setAdditionalTags(bool enable);

  /**
   * Get cover art option.
   *
   * @return true if cover art are enabled.
   */
  bool getCoverArt() const;

  /**
   * Set cover art option.
   *
   * @param enable true if cover art are enabled
   */
  void setCoverArt(bool enable);

  /**
   * Set a find string from artist and album information.
   *
   * @param artist artist
   * @param album  album
   */
  void setArtistAlbum(const QString& artist, const QString& album);

private slots:
  /**
   * Query a search for a keyword from the server.
   */
  void slotFind();

  /**
   * Process finished find request.
   *
   * @param searchStr search data received
   */
  void slotFindFinished(const QByteArray& searchStr);

  /**
   * Process finished album data.
   *
   * @param albumStr album track data received
   */
  void slotAlbumFinished(const QByteArray& albumStr);

  /**
   * Request track list from server.
   *
   * @param category category, e.g. "release"
   * @param id internal ID
   */
  void requestTrackList(const QString& category, const QString& id);

  /**
   * Request track list from server.
   *
   * @param index model index of list containing an AlbumListItem
   */
  void requestTrackList(const QModelIndex& index);

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Display message in status bar.
   *
   * @param msg status message
   */
  void showStatusMessage(const QString& msg);

signals:
  /**
   * Emitted when the m_trackDataVector was updated with new imported data.
   */
  void trackDataUpdated();

protected:
  QListView* m_albumListBox; /**< list box with albums to select */

private:
  /**
   * Get the local configuration.
   *
   * @param cfg configuration
   */
  void getImportSourceConfig(ServerImporterConfig* cfg) const;

  QComboBox* m_artistLineEdit;
  QComboBox* m_albumLineEdit;
  QPushButton* m_findButton;
  QLabel* m_serverLabel;
  QComboBox* m_serverComboBox;
  QLabel* m_cgiLabel;
  QLineEdit* m_cgiLineEdit;
  QCheckBox* m_standardTagsCheckBox;
  QCheckBox* m_additionalTagsCheckBox;
  QCheckBox* m_coverArtCheckBox;
  QPushButton* m_helpButton;
  QPushButton* m_saveButton;
  QStatusBar* m_statusBar;
  ServerImporter* m_source;
};
