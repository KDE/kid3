/**
 * \file exportdialog.h
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
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
#include <QStringList>

class IPlatformTools;
class QTextEdit;
class QTableView;
class QPushButton;
class QComboBox;
class TextExporter;
class TextTableModel;
class FormatListEdit;

/**
 * Export dialog.
 */
class ExportDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform tools
   * @param parent       parent widget
   * @param textExporter text exporter to use
   */
  ExportDialog(IPlatformTools* platformTools,
               QWidget* parent, TextExporter* textExporter);

  /**
   * Destructor.
   */
  virtual ~ExportDialog() override = default;

  /**
   * Read the local settings from the configuration.
   */
  void readConfig();

public slots:
  /**
   * Show exported text as preview in editor.
   */
  void showPreview();

private slots:
  /**
   * Export to a file.
   */
  void slotToFile();

  /**
   * Export to clipboard.
   */
  void slotToClipboard();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Called when the source combo box selection is changed.
   * @param index combo box index
   */
  void onSrcComboBoxActivated(int index);

private:
  /**
   * Set the format combo box and line edits from the configuration.
   */
  void setFormatFromConfig();

  IPlatformTools* m_platformTools;
  /** Text editor */
  QTextEdit* m_edit;
  /** Table view */
  QTableView* m_table;
  /** Format editor */
  FormatListEdit* m_formatListEdit;
  /** To File button */
  QPushButton* m_fileButton;
  /** To Clipboard button */
  QPushButton* m_clipButton;
  /** combobox with export sources */
  QComboBox* m_srcComboBox;

  /** text exporter */
  TextExporter* m_textExporter;
  /** text table model */
  TextTableModel* m_textTableModel;
};
