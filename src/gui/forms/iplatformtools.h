/**
 * \file iplatformtools.h
 * Interface for platform specific tools.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Mar 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#include <QStringList>
#include "icoreplatformtools.h"

class QWidget;
class QIcon;

/**
 * Interface for platform specific tools.
 */
class KID3_GUI_EXPORT IPlatformTools : public ICorePlatformTools {
public:
  /**
   * Destructor.
   */
  ~IPlatformTools() override;

  /**
   * Display help for a topic.
   *
   * @param anchor anchor in help document
   */
  virtual void displayHelp(const QString& anchor) = 0;

  /**
   * Get a themed icon by name.
   * @param name name of icon
   * @return icon.
   */
  virtual QIcon iconFromTheme(const QString& name) const = 0;

  /**
   * Display error dialog with item list.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   */
  virtual void errorList(QWidget* parent, const QString& text,
      const QStringList& strlist, const QString& caption) = 0;

  /**
   * Display warning dialog with yes, no, cancel buttons.
   * @param parent parent widget
   * @param text text
   * @param caption caption
   * @return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
   */
  virtual int warningYesNoCancel(QWidget* parent, const QString& text,
      const QString& caption) = 0;

  /**
   * Display warning dialog with item list.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   * @return QMessageBox::Yes or QMessageBox::No.
   */
  virtual int warningYesNoList(QWidget* parent, const QString& text,
      const QStringList& strlist, const QString& caption) = 0;

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
      QString* selectedFilter) = 0;

  /**
   * Display warning dialog.
   * @param parent parent widget
   * @param text text
   * @param details detailed message
   * @param caption caption
   */
  virtual void warningDialog(QWidget* parent,
      const QString& text, const QString& details, const QString& caption) = 0;

  /**
   * Display warning dialog with options to continue or cancel.
   * @param parent parent widget
   * @param text text
   * @param strlist list of items
   * @param caption caption
   * @return true if continue was selected.
   */
  virtual bool warningContinueCancelList(QWidget* parent,
   const QString& text, const QStringList& strlist, const QString& caption) = 0;
};
