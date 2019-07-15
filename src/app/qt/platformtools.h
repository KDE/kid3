/**
 * \file platformtools.h
 * Platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include <QScopedPointer>
#include "iplatformtools.h"
#include "guiplatformtools.h"

class QSettings;
class BrowserDialog;

/**
 * Platform specific tools.
 */
class PlatformTools : public IPlatformTools, private GuiPlatformTools {
public:
  /**
   * Constructor.
   */
  PlatformTools();

  /**
   * Destructor.
   */
  virtual ~PlatformTools() override;

  /**
   * Get application settings.
   * @return settings instance.
   */
  virtual ISettings* applicationSettings() override;

  /**
   * Get icon provider for tagged files.
   * @return icon provider.
   */
  virtual CoreTaggedFileIconProvider* iconProvider() override;

  /**
   * Write text to clipboard.
   * @param text text to write
   * @return true if operation is supported.
   */
  virtual bool writeToClipboard(const QString& text) const override;

  /**
   * Read text from clipboard.
   * @return text, null if operation not supported.
   */
  virtual QString readFromClipboard() const override;

  /**
   * Create an audio player instance.
   * @param app application context
   * @param dbusEnabled true to enable MPRIS D-Bus interface
   * @return audio player, nullptr if not supported.
   */
  virtual QObject* createAudioPlayer(Kid3Application* app,
                                     bool dbusEnabled) const override;

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  virtual bool moveToTrash(const QString& path) const override;

  /**
   * Display help for a topic.
   *
   * @param anchor anchor in help document
   */
  virtual void displayHelp(const QString& anchor) override;

  /**
   * Get a themed icon by name.
   * @param name name of icon
   * @return icon.
   */
  virtual QIcon iconFromTheme(const QString& name) const override;

  /**
   * Construct a name filter string suitable for file dialogs.
   * @param nameFilters list of description, filter pairs, e.g.
   * [("Images", "*.jpg *.jpeg *.png"), ("All Files", "*")].
   * @return name filter string.
   */
  virtual QString fileDialogNameFilter(
      const QList<QPair<QString, QString> >& nameFilters) const override;

  /**
   * Get file pattern part of m_nameFilter.
   * @param nameFilter name filter string
   * @return file patterns, e.g. "*.mp3".
   */
  virtual QString getNameFilterPatterns(const QString& nameFilter) const override;

  /**
   * Display error dialog with item list.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   */
  virtual void errorList(QWidget* parent, const QString& text,
      const QStringList& strlist, const QString& caption) override;

  /**
   * Display warning dialog with yes, no, cancel buttons.
   * @param parent parent widget
   * @param text text
   * @param caption caption
   * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
   */
  virtual int warningYesNoCancel(QWidget* parent, const QString& text,
      const QString& caption) override;

  /**
   * Display warning dialog with item list.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   * @return QMessageBox::Yes or QMessageBox::No.
   */
  virtual int warningYesNoList(QWidget* parent, const QString& text,
      const QStringList& strlist, const QString& caption) override;

  /**
   * Display dialog to select an existing file.
   * @param parent parent widget
   * @param caption caption
   * @param dir directory
   * @param filter filter
   * @param selectedFilter the selected filter is returned here
   * @return selected file, empty if canceled.
   */
  virtual QString getOpenFileName(QWidget* parent,
      const QString& caption, const QString& dir, const QString& filter,
      QString* selectedFilter) override;

  /**
   * Display dialog to select existing files.
   * @param parent parent widget
   * @param caption caption
   * @param dir directory
   * @param filter filter
   * @param selectedFilter the selected filter is returned here
   * @return selected files, empty if canceled.
   */
  virtual QStringList getOpenFileNames(QWidget* parent,
      const QString& caption, const QString& dir, const QString& filter,
      QString* selectedFilter) override;

  /**
   * Display dialog to select a file to save.
   * @param parent parent widget
   * @param caption caption
   * @param dir directory
   * @param filter filter
   * @param selectedFilter the selected filter is returned here
   * @return selected file, empty if canceled.
   */
  virtual QString getSaveFileName(QWidget* parent,
      const QString& caption, const QString& dir, const QString& filter,
      QString* selectedFilter) override;

  /**
   * Display dialog to select an existing directory.
   * @param parent parent widget
   * @param caption caption
   * @param startDir start directory
   * @return selected directory, empty if canceled.
   */
  virtual QString getExistingDirectory(QWidget* parent,
      const QString& caption, const QString& startDir) override;

  /**
   * Display warning dialog.
   * @param parent parent widget
   * @param text text
   * @param details detailed message
   * @param caption caption
   */
  virtual void warningDialog(QWidget* parent,
      const QString& text, const QString& details, const QString& caption) override;

  /**
   * Display warning dialog with options to continue or cancel.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   * @return true if continue was selected.
   */
  virtual bool warningContinueCancelList(QWidget* parent,
      const QString& text, const QStringList& strlist, const QString& caption) override;

private:
  QScopedPointer<BrowserDialog> m_helpBrowser;
};
