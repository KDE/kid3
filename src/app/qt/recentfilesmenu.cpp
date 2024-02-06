/**
 * \file recentfilesmenu.cpp
 * Menu to open recent files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Aug 2010
 *
 * Copyright (C) 2010-2024  Urs Fleisch
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

#include "recentfilesmenu.h"
#include <QAction>
#include <QDir>
#include "isettings.h"

namespace {

constexpr int MAX_RECENT_FILES = 10;

}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
RecentFilesMenu::RecentFilesMenu(QWidget* parent) : QMenu(parent)
{
  setObjectName(QLatin1String("RecentFilesMenu"));
}

/**
 * Add directory to list of recent files.
 *
 * @param dir path to directory
 */
void RecentFilesMenu::addDirectory(const QString& dir)
{
  QString path = QDir(dir).canonicalPath();
  if (path.isNull())
    return;

  // first remove the path if it already exists
  if (int pathIdx = m_files.indexOf(path); pathIdx != -1) {
    m_files.removeAt(pathIdx);
  }

  m_files.prepend(path);
  if (m_files.size() > MAX_RECENT_FILES) {
    m_files.removeLast();
  }

  updateRecentFileActions();
}

/**
 * Saves the current recent files entries to a given configuration.
 *
 * @param config configuration settings
 */
void RecentFilesMenu::saveEntries(ISettings* config)
{
  config->beginGroup(QLatin1String("RecentFiles"), true);
  config->setValue(QLatin1String("Files"), QVariant(m_files));
  config->endGroup();
}

/**
 * Loads the recent files entries from a given configuration.
 *
 * @param config configuration settings
 */
void RecentFilesMenu::loadEntries(ISettings* config)
{
  config->beginGroup(QLatin1String("RecentFiles"), true);
  m_files = config->value(QLatin1String("Files"), m_files).toStringList();
  config->endGroup();

  while (m_files.size() > MAX_RECENT_FILES) {
    m_files.removeLast();
  }

  updateRecentFileActions();
}

/**
 * Update the recent file actions.
 */
void RecentFilesMenu::updateRecentFileActions()
{
  int i = 0;
  clear();
  for (auto it = m_files.constBegin(); it != m_files.constEnd(); ++it) {
    auto act = new QAction(this);
    act->setText(QString(QLatin1String("&%1 %2")).arg(++i).arg(*it));
    act->setData(*it);
    connect(act, &QAction::triggered, this, &RecentFilesMenu::openRecentFile);
    this->addAction(act);
  }
  if (i > 0) {
    addSeparator();
    auto clearListAction = new QAction(this);
    clearListAction->setText(tr("&Clear List"));
    connect(clearListAction, &QAction::triggered, this, &RecentFilesMenu::clearList);
    this->addAction(clearListAction);
    setEnabled(true);
  } else {
    setEnabled(false);
  }
}

/**
 * Emit a load file signal when a recent file has to be loaded.
 */
void RecentFilesMenu::openRecentFile()
{
  auto action = qobject_cast<QAction*>(sender());
  if (action) {
    emit loadFile(action->data().toString());
  }
}

/**
 * Clear the list of recent files.
 */
void RecentFilesMenu::clearList()
{
  m_files.clear();
  updateRecentFileActions();
}
