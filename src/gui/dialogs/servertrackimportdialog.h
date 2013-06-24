/**
 * \file servertrackimportdialog.h
 * Generic dialog for track based import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2012  Urs Fleisch
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

#ifndef SERVERTRACKIMPORTDIALOG_H
#define SERVERTRACKIMPORTDIALOG_H

#include <QDialog>
#include <QString>
#include <QVector>

class QTableView;
class QStandardItemModel;
class QLineEdit;
class QLabel;
class QComboBox;
class QPushButton;
class QCheckBox;
class QStatusBar;
class QModelIndex;
class TrackDataModel;
class ImportTrackData;
class ImportTrackDataVector;
class ServerTrackImporter;

/**
 * Generic dialog for track based import from a server.
 */
class ServerTrackImportDialog : public QDialog {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent          parent widget
   * @param trackDataModel track data to be filled with imported values,
   *                        is passed with filenames set
   */
  ServerTrackImportDialog(QWidget* parent,
                          TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~ServerTrackImportDialog();

  /**
   * Set importer to be used.
   *
   * @param source  import source to use
   */
  void setImportSource(ServerTrackImporter* source);

  /**
   * Initialize the table model.
   * Has to be called before reusing the dialog with new track data.
   */
  void initTable();

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

signals:
  /**
   * Emitted when the m_trackDataModel was updated with new imported data.
   */
  void trackDataUpdated();

public slots:
  /**
   * Shows the dialog as a modal dialog.
   */
  int exec();

protected slots:
  /**
   * Hides the dialog and sets the result to QDialog::Accepted.
   */
  virtual void accept();

  /**
   * Hides the dialog and sets the result to QDialog::Rejected.
   */
  virtual void reject();

private slots:
  /**
   * Apply imported data.
   */
  void apply();

  /**
   * Set the status of a file.
   *
   * @param index  index of file
   * @param status status string
   */
  void setFileStatus(int index, const QString& status);

  /**
   * Update the track data combo box of a file.
   *
   * @param index  index of file
   */
  void updateFileTrackData(int index);

  /**
   * Set result list for a file.
   *
   * @param index           index of file
   * @param trackDataVector result list
   */
  void setResults(int index, ImportTrackDataVector& trackDataVector);

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Show the name of the current track in the status bar.
   *
   * @param row table row
   */
  void showFilenameInStatusBar(const QModelIndex& index);

private:
  /**
   * Clear all results.
   */
  void clearResults();

 /**
  * Create and start the track import client.
  */
  void startClient();

  /**
   * Stop and destroy the track import client.
   */
  void stopClient();

  QLabel* m_serverLabel;
  QComboBox* m_serverComboBox;
  QTableView* m_albumTable;
  QPushButton* m_helpButton;
  QPushButton* m_saveButton;
  QStandardItemModel* m_albumTableModel;
  QStatusBar* m_statusBar;
  ServerTrackImporter* m_client;
  TrackDataModel* m_trackDataModel;
  QVector<ImportTrackDataVector> m_trackResults;
};

#endif // SERVERTRACKIMPORTDIALOG_H
