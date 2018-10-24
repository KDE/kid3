/**
 * \file rendirdialog.h
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 *
 * Copyright (C) 2004-2018  Urs Fleisch
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
#include <QList>
#include <QWizard>

class QComboBox;
class QLabel;
class TaggedFile;
class QVBoxLayout;
class QTextEdit;
class DirRenamer;

/**
 * Rename directory dialog.
 */
class RenDirDialog : public QWizard {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent     parent widget
   * @param dirRenamer directory renamer
   */
  RenDirDialog(QWidget* parent, DirRenamer* dirRenamer);

  /**
   * Destructor.
   */
  virtual ~RenDirDialog() override = default;

  /**
   * Start dialog.
   *
   * @param taggedFile file to use for rename preview
   * @param dirName    if taggedFile is 0, the directory can be set here
   */
  void startDialog(TaggedFile* taggedFile, const QString& dirName = QString());

  /**
   * Set new directory name.
   *
   * @param dir new directory name
   */
  void setNewDirname(const QString& dir);

  /**
   * Get new directory name.
   *
   * @return new directory name.
   */
  QString getNewDirname() const;

public slots:
  /**
   * Display action preview.
   *
   * @param actionStrs description of action
   */
  void displayActionPreview(const QStringList& actionStrs);

protected:
  /**
   * Called when the wizard is canceled.
   */
  virtual void reject() override;

signals:
  /**
   * Emitted when scheduling of actions using clearActions() followed by
   * one or multiple scheduleAction() calls is requested.
   */
  void actionSchedulingRequested();

private slots:
  /**
   * Set new directory name according to current settings.
   */
  void slotUpdateNewDirname();

  /**
   * Save the local settings to the configuration.
   */
  void saveConfig();

  /**
   * Show help.
   */
  void showHelp();

  /**
   * Request action scheduling and then accept dialog.
   */
  void requestActionSchedulingAndAccept();

  /**
   * Wizard page changed.
   */
  void pageChanged();

private:
  /** Action to be performed. */
  enum Action { ActionRename = 0, ActionCreate = 1 };

  /**
   * Set up the main wizard page.
   *
   * @param page    widget
   * @param vlayout layout
   */
  void setupMainPage(QWidget* page, QVBoxLayout* vlayout);

  /**
   * Set up the preview wizard page.
   *
   * @param page widget
   */
  void setupPreviewPage(QWidget* page);

  /**
   * Clear action preview.
   */
  void clearPreview();

  /**
   * Set configuration from dialog in directory renamer.
   */
  void setDirRenamerConfiguration();

  QComboBox* m_formatComboBox;
  QComboBox* m_actionComboBox;
  QComboBox* m_tagversionComboBox;
  QLabel* m_currentDirLabel;
  QLabel* m_newDirLabel;
  QTextEdit* m_edit;
  TaggedFile* m_taggedFile;
  DirRenamer* m_dirRenamer;
};
