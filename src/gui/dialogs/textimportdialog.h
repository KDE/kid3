/**
 * \file textimportdialog.h
 * Dialog to import from a text (file or clipboard).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef TEXTIMPORTDIALOG_H
#define TEXTIMPORTDIALOG_H

#include <QDialog>

class TextImporter;
class TrackDataModel;
class FormatListEdit;
class IPlatformTools;

/**
 * Dialog to import from a text (file or clipboard).
 */
class TextImportDialog : public QDialog {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param parent  parent widget
   * @param trackDataModel track data to be filled with imported values
   */
  TextImportDialog(IPlatformTools* platformTools, QWidget* parent,
                   TrackDataModel* trackDataModel);

  /**
   * Destructor.
   */
  virtual ~TextImportDialog();

  /**
   * Clear dialog data.
   */
  void clear();

private slots:
  /**
   * Let user select file, assign file contents to text and preview in
   * table.
   */
  void fromFile();

  /**
   * Assign clipboard contents to text and preview in table.
   */
  void fromClipboard();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

signals:
  /**
   * Emitted when the m_trackDataVector was updated with new imported data.
   */
  void trackDataUpdated();

private:
  /**
   * Import from a file.
   *
   * @param fn file name
   *
   * @return true if ok.
   */
  bool importFromFile(const QString& fn);

  /**
   * Set the format combo box and line edits from the configuration.
   */
  void setFormatFromConfig();

  IPlatformTools* m_platformTools;
  /** format editor */
  FormatListEdit* m_formatListEdit;
  /** text importer */
  TextImporter* m_textImporter;
};

#endif
