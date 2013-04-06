/**
 * \file importdialog.h
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include "config.h"
#include "importconfig.h"

class IPlatformTools;
class QString;
class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QTableView;
class QLabel;
class TrackDataModel;
class MusicBrainzDialog;
class ServerImporter;
class ServerImportDialog;
class TextImportDialog;
class TagImportDialog;
class ImportTrackDataVector;
class FrameCollection;
class FreedbConfig;
class MusicBrainzClient;
#ifdef HAVE_CHROMAPRINT
class MusicBrainzConfig;
#endif

/**
 * Import dialog.
 */
class ImportDialog : public QDialog {
Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param parent        parent widget
   * @param caption       dialog title
   * @param trackDataModel track data to be filled with imported values,
   *                      is passed with durations of files set
   * @param importers     server importers
   * @param mbClient      MusicBrainz client if supported, else 0
   */
  ImportDialog(IPlatformTools* platformTools,
               QWidget* parent, QString& caption,
               TrackDataModel* trackDataModel,
               const QList<ServerImporter*>& importers,
               MusicBrainzClient* mbClient);

  /**
   * Destructor.
   */
  ~ImportDialog();

  /**
   * Set dialog to be started automatically.
   *
   * @param importerIndex index of importer to use
   */
  void setAutoStartSubDialog(int importerIndex);

  /**
   * Clear dialog data.
   */
  void clear();

  /**
   * Get import destination.
   *
   * @return TagV1, TagV2 or TagV2V1 for ID3v1, ID3v2 or both.
   */
  TrackData::TagVersion getDestination() const;

public slots:
  /**
   * Shows the dialog as a modal dialog.
   */
  int exec();

private slots:
  /**
   * Show help.
   */
  void showHelp();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Called when the maximum time difference value is changed.
   */
  void maxDiffChanged();

  /**
   * Move a table row.
   *
   * The first parameter @a section is not used.
   * @param fromIndex index of position moved from
   * @param toIndex   index of position moved to
   */
  void moveTableRow(int, int fromIndex, int toIndex);

  /**
   * Import from server and preview in table.
   */
  void fromServer();

  /**
   * Import from text.
   */
  void fromText();

  /**
   * Import from tags.
   */
  void fromTags();

  /**
   * Show fields to import in text as preview in table.
   */
  void showPreview();

  /**
   * Match import data with length.
   */
  void matchWithLength();

  /**
   * Match import data with track number.
   */
  void matchWithTrack();

  /**
   * Match import data with title.
   */
  void matchWithTitle();

#ifdef HAVE_CHROMAPRINT
  /**
   * Import from MusicBrainz and preview in table.
   */
  void fromMusicBrainz();
#endif

  /**
   * Hide subdialogs.
   */
  void hideSubdialogs();

  /**
   * Called when the destination combo box value is changed.
   */
  void changeTagDestination();

  /**
   * Display custom context menu for horizontal table header.
   *
   * @param pos position where context menu is drawn on screen
   */
  void showTableHeaderContextMenu(const QPoint& pos);

  /**
   * Toggle visibility of table column.
   *
   * @param visible true to make column visible
   */
  void toggleTableColumnVisibility(bool visible);

private:
  /**
   * Get time difference check configuration.
   *
   * @param enable  true if check is enabled
   * @param maxDiff maximum allowed time difference
   */
  void getTimeDifferenceCheck(bool& enable, int& maxDiff) const;

  /**
   * Display server import dialog.
   *
   * @param importerIdx importer index, if invalid but not negative the
   *                    MusicBrainz Fingerprint dialog is displayed
   */
  void displayServerImportDialog(int importerIdx);

  /**
   * Display server import dialog.
   *
   * @param source import source
   */
  void displayServerImportDialog(ServerImporter* source);

  IPlatformTools* m_platformTools;
  /** Index of importer for subdialog to open when starting, -1 for none */
  int m_autoStartSubDialog;
  /** Mask for visibility of optional columns */
  quint64 m_columnVisibility;
  /** Preview table */
  QTableView* m_trackDataTable;
  /** Track data model */
  TrackDataModel* m_trackDataModel;
  /** Accuracy value */
  QLabel* m_accuracyPercentLabel;
  /** URL of cover art to be imported */
  QLabel* m_coverArtUrlLabel;
  /** combobox with import servers */
  QComboBox* m_serverComboBox;
  /** combobox with import destinations */
  QComboBox* m_destComboBox;
  QCheckBox* m_mismatchCheckBox;
  QSpinBox* m_maxDiffSpinBox;
  /** importers for different servers */
  QList<ServerImporter*> m_importers;
  /** MusicBrainz client */
  MusicBrainzClient* m_musicBrainzClient;
  /** MusicBrainz import dialog */
  MusicBrainzDialog* m_musicBrainzDialog;
  /** Server import dialog */
  ServerImportDialog* m_serverImportDialog;
  /** Text import dialog */
  TextImportDialog* m_textImportDialog;
  /** Tag import dialog */
  TagImportDialog* m_tagImportDialog;
};

#endif
