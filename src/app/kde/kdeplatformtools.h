/**
 * \file kdeplatformtools.h
 * KDE platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

#ifndef KDEPLATFORMTOOLS_H
#define KDEPLATFORMTOOLS_H

#include "iplatformtools.h"

/**
 * KDE platform specific tools.
 */
class KdePlatformTools : public IPlatformTools {
public:
  /**
   * Constructor.
   */
  KdePlatformTools();

  /**
   * Destructor.
   */
  virtual ~KdePlatformTools();

  /**
   * Move file or directory to trash.
   *
   * @param path path to file or directory
   *
   * @return true if ok.
   */
  virtual bool moveToTrash(const QString& path) const;

  /**
   * Display help for a topic.
   *
   * @param anchor anchor in help document
   */
  virtual void displayHelp(const QString& anchor);

  /**
   * Display error dialog with item list.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   */
  virtual void errorList(QWidget* parent, const QString& text,
      const QStringList& strlist, const QString& caption);

  /**
   * Display warning dialog with yes, no, cancel buttons.
   * @param parent parent widget
   * @param text text
   * @param caption caption
   * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
   */
  virtual int warningYesNoCancel(QWidget* parent, const QString& text,
      const QString& caption);

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
      QString* selectedFilter);

  /**
   * Display dialog to select an existing directory.
   * @param parent parent widget
   * @param caption caption
   * @param startDir start directory
   * @return selected directory, empty if canceled.
   */
  virtual QString getExistingDirectory(QWidget* parent,
      const QString& caption, const QString& startDir);

  /**
   * Display warning dialog.
   * @param parent parent widget
   * @param text text
   * @param details detailed message
   * @param caption caption
   */
  virtual void warningDialog(QWidget* parent,
      const QString& text, const QString& details, const QString& caption);

  /**
   * Display warning dialog with options to continue or cancel.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   * @return true if continue was selected.
   */
  virtual bool warningContinueCancelList(QWidget* parent,
      const QString& text, const QStringList& strlist, const QString& caption);
};

#endif // KDEPLATFORMTOOLS_H