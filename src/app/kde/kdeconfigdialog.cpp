/**
 * \file kdeconfigdialog.cpp
 * KDE configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "kdeconfigdialog.h"
#include "contexthelp.h"
#include "configdialogpages.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param configSkeleton configuration skeleton
 */
KdeConfigDialog::KdeConfigDialog(QWidget* parent, QString& caption,
                                 KConfigSkeleton* configSkeleton) :
  KConfigDialog(parent, QLatin1String("configure"), configSkeleton),
  m_pages(new ConfigDialogPages(this))
{
  setObjectName(QLatin1String("ConfigDialog"));
  setWindowTitle(caption);
  setSizeGripEnabled(true);

  addPage(m_pages->createTagsPage(), tr("Tags"), QLatin1String("applications-multimedia"));
  addPage(m_pages->createFilesPage(), tr("Files"), QLatin1String("document-save"));
  addPage(m_pages->createActionsPage(), tr("User Actions"), QLatin1String("preferences-other"));
  addPage(m_pages->createNetworkPage(), tr("Network"), QLatin1String("preferences-system-network"));

  setButtons(Ok | Cancel | Help);
  setHelp(QLatin1String("configure-kid3"));
}

/**
 * Destructor.
 */
KdeConfigDialog::~KdeConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 */
void KdeConfigDialog::setConfig()
{
  m_pages->setConfig();
}

/**
 * Get values from dialog and store them in the current configuration.
 */
void KdeConfigDialog::getConfig() const
{
  m_pages->getConfig();
}

/**
 * Show help.
 */
void KdeConfigDialog::slotHelp()
{
  ContextHelp::displayHelp(QLatin1String("configure-kid3"));
}
