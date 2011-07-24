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
#include <QStringList>

class QLineEdit;
class QLabel;
class QComboBox;
class QPushButton;
class TextImporter;
class TrackDataModel;

/**
 * Dialog to import from a text (file or clipboard).
 */
class TextImportDialog : public QDialog {
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent  parent widget
   * @param trackDataModel track data to be filled with imported values
   */
  explicit TextImportDialog(QWidget* parent,
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
   * Set the format lineedits to the format selected in the combo box.
   *
   * @param index current index of the combo box
   */
  void setFormatLineEdit(int index);

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

  /** combobox with import formats */
  QComboBox* m_formatComboBox;
  /** LineEdit for header regexp */
  QLineEdit* m_headerLineEdit;
  /** LineEdit for track regexp */
  QLineEdit* m_trackLineEdit;
  /** header format regexps */
  QStringList m_formatHeaders;
  /** track format regexps */
  QStringList m_formatTracks;
  /** text importer */
  TextImporter* m_textImporter;
};

#endif
