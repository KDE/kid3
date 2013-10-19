/**
 * \file browsecoverartdialog.h
 * Browse cover art dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11-Jan-2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#ifndef BROWSECOVERARTDIALOG_H
#define BROWSECOVERARTDIALOG_H

#include <QDialog>
#include "frame.h"

class QTextEdit;
class QLineEdit;
class ExternalProcess;
class ConfigTable;
class ConfigTableModel;
class FormatListEdit;

/**
 * Browse cover art dialog.
 */
class BrowseCoverArtDialog : public QDialog {
Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit BrowseCoverArtDialog(QWidget* parent);

  /**
   * Destructor.
   */
  virtual ~BrowseCoverArtDialog();

  /**
   * Read the local settings from the configuration.
   */
  void readConfig();

  /**
   * Set frames for which picture has to be found.
   *
   * @param frames track data
   */
  void setFrames(const FrameCollection& frames);

public slots:
  /**
   * Hide modal dialog, start browse command.
   */
  virtual void accept();

private slots:
  /**
   * Show browse command as preview.
   */
  void showPreview();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

private:
  /**
   * Set the source combo box and line edits from the configuration.
   */
  void setSourceFromConfig();

  /** Text editor with command preview */
  QTextEdit* m_edit;
  /** Combobox with artist */
  QLineEdit* m_artistLineEdit;
  /** Combobox with album */
  QLineEdit* m_albumLineEdit;
  /** format editor */
  FormatListEdit* m_formatListEdit;
  /** Table to extract picture URL */
  ConfigTable* m_matchUrlTable;
  /** Table model to extract picture URL */
  ConfigTableModel* m_matchUrlTableModel;
  /** Formatted URL */
  QString m_url;

  /** Track data */
  FrameCollection m_frames;
  /** Process for browser command */
  ExternalProcess* m_process;
};

#endif
