/**
 * \file kdeconfigdialog.h
 * KDE configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#include <QtGlobal>
#include <KConfigDialog>

class QString;
class QWidget;
class QComboBox;
class KConfigSkeleton;
class ConfigDialogPages;
class IPlatformTools;

/**
 * KDE configuration dialog.
 */
class KdeConfigDialog : public KConfigDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param platformTools platform specific tools
   * @param parent  parent widget
   * @param caption dialog title
   * @param configSkeleton configuration skeleton
   */
  KdeConfigDialog(IPlatformTools* platformTools, QWidget* parent, QString& caption,
                  KConfigSkeleton* configSkeleton);

  /**
   * Destructor.
   */
  virtual ~KdeConfigDialog() override = default;

  /**
   * Set values in dialog from current configuration.
   */
  void setConfig();

  /**
   * Get values from dialog and store them in the current configuration.
   */
  void getConfig() const;

protected slots:
  /**
   * Show help.
   */
  void slotHelp();

protected:
  /**
   * Returns whether the current state of the dialog is
   * the same as the default configuration.
   * @return false
   */
  virtual bool isDefault() override;

private:
  ConfigDialogPages* m_pages;
};
