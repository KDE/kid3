/**
 * \file filterdialog.h
 * Filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include "filefilter.h"

class QPushButton;
class FormatListEdit;

/**
 * Filter dialog.
 */
class FilterDialog : public QDialog {
Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  FilterDialog(QWidget* parent);

  /**
   * Destructor.
   */
  ~FilterDialog();

  /**
   * Read the local settings from the configuration.
   */
  void readConfig();

  /**
   * Display information in text view.
   *
   * @param text text to display
   */
  void showInformation(const QString& text) { m_edit->append(text); }

signals:
  /**
   * Is triggered when the selected @a filter has to be applied.
   */
  void apply(FileFilter&);

public slots:
  /**
   * Show information about filter event.
   *
   * @param type filter event type
   * @param fileName name of filtered file
   */
  void showFilterEvent(FileFilter::FilterEventType type,
                       const QString& fileName);

private slots:
  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Apply or abort filter.
   */
  void applyOrAbortFilter();

private:
  /**
   * Set the filter combo box and line edit from the configuration.
   */
  void setFiltersFromConfig();

  /**
   * Set button to Apply or Abort.
   * @param enableAbort true to set Abort button
   */
  void setAbortButton(bool enableAbort);

  /** Text editor */
  QTextEdit* m_edit;
  /** format editor */
  FormatListEdit* m_formatListEdit;
  /** Apply button */
  QPushButton* m_applyButton;
  /** file filter used */
  FileFilter m_fileFilter;
  /** true if m_applyButton is an Abort button */
  bool m_isAbortButton;
};

#endif
