/**
 * \file findreplacedialog.h
 * Find and replace dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Feb 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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
#include "tagsearcher.h"

class QLabel;
class QComboBox;
class QCheckBox;
class QStatusBar;
class CheckableStringListModel;

/**
 * Find and replace dialog.
 */
class FindReplaceDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit FindReplaceDialog(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FindReplaceDialog() override = default;

  /**
   * Initialize dialog before it is displayed.
   * @param findOnly true to display only find part of dialog
   */
  void init(bool findOnly);

  /**
   * Get search parameters from GUI controls.
   * @param params the search parameters are returned here
   */
  void getParameters(TagSearcher::Parameters& params) const;

  /**
   * Set search parameters in GUI controls.
   * @param params search parameters
   */
  void setParameters(const TagSearcher::Parameters& params);

public slots:
  /**
   * Show progress message.
   * @param msg message
   */
  void showProgress(const QString& msg);

signals:
  /**
   * Emitted to request search for a string
   * @param params search parameters
   */
  void findRequested(const TagSearcher::Parameters& params);

  /**
   * Emitted to request replacing of a string
   * @param params search parameters
   */
  void replaceRequested(const TagSearcher::Parameters& params);

  /**
   * Emitted to request replacing all occurrences.
   * @param params search parameters
   */
  void replaceAllRequested(const TagSearcher::Parameters& params);

private slots:
  /**
   * Find next occurrence.
   */
  void find();

  /**
   * Replace found text.
   */
  void replace();

  /**
   * Replace all occurrences.
   */
  void replaceAll();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Called when Return is pressed in the Find combo box.
   */
  void onReturnPressedInFind();

  /**
   * Called when Return is pressed in the Replace combo box.
   */
  void onReturnPressedInReplace();

private:
  void readConfig();

  QPushButton* m_findButton;
  QPushButton* m_replaceButton;
  QPushButton* m_replaceAllButton;
  QLabel* m_replaceLabel;
  QComboBox* m_findEdit;
  QComboBox* m_replaceEdit;
  QCheckBox* m_matchCaseCheckBox;
  QCheckBox* m_backwardsCheckBox;
  QCheckBox* m_regExpCheckBox;
  QCheckBox* m_allFramesCheckBox;
  QStatusBar* m_statusBar;
  CheckableStringListModel* m_tagsModel;
};
