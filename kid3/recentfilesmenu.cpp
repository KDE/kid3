/**
 * \file recentfilesmenu.cpp
 * Menu to open recent files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Aug 2010
 */

#include "recentfilesmenu.h"

#ifndef CONFIG_USE_KDE

#include <QAction>
#include <QDir>
#include "qtcompatmac.h"

static const int MAX_RECENT_FILES = 10;

/**
 * Constructor.
 *
 * @param parent parent widget
 */
RecentFilesMenu::RecentFilesMenu(QWidget* parent) : QMenu(parent)
{
  setObjectName("RecentFilesMenu");
}

/**
 * Destructor.
 */
RecentFilesMenu::~RecentFilesMenu()
{
}

/**
 * Add directory to list of recent files.
 *
 * @param path path to directory
 */
void RecentFilesMenu::addDirectory(const QString& dir)
{
  QString path = QDir(dir).canonicalPath();
  if (path.isNull())
    return;

  // first remove the path if it already exists
  int pathIdx = m_files.indexOf(path);
  if (pathIdx != -1) {
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
void RecentFilesMenu::saveEntries(Kid3Settings* config)
{
  config->beginGroup("/Recent Files");
  config->setValue("Files", QVariant(m_files));
  config->endGroup();
}

/**
 * Loads the recent files entries from a given configuration.
 *
 * @param config configuration settings
 */
void RecentFilesMenu::loadEntries(Kid3Settings* config)
{
  config->beginGroup("/Recent Files");
  m_files = config->value("Files").toStringList();
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
  for (QStringList::const_iterator it = m_files.begin();
       it != m_files.end();
       ++it) {
    QAction* act = new QAction(this);
    act->setText(QString("&%1 %2").arg(++i).arg(*it));
    act->setData(*it);
    connect(act, SIGNAL(triggered()), this, SLOT(openRecentFile()));
    this->addAction(act);
  }
  if (i > 0) {
    addSeparator();
    QAction* clearListAction = new QAction(this);
    clearListAction->setText(i18n("&Clear List"));
    connect(clearListAction, SIGNAL(triggered()), this, SLOT(clearList()));
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
  QAction* action = qobject_cast<QAction*>(sender());
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
#else // !CONFIG_USE_KDE

void RecentFilesMenu::openRecentFile() {}
void RecentFilesMenu::clearList(){}

#endif // !CONFIG_USE_KDE
