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

#include <qaction.h>
#include <qdir.h>
#include "qtcompatmac.h"

static const int MAX_RECENT_FILES = 10;

/**
 * Constructor.
 *
 * @param parent parent widget
 */
RecentFilesMenu::RecentFilesMenu(QWidget* parent) :
	RecentFilesMenuBaseClass(parent)
{
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
#if QT_VERSION >= 0x040000
	int pathIdx = m_files.indexOf(path);
	if (pathIdx != -1) {
		m_files.removeAt(pathIdx);
	}

	m_files.prepend(path);
	if (m_files.size() > MAX_RECENT_FILES) {
		m_files.removeLast();
	}
#else
	m_files.remove(path);

	m_files.prepend(path);
	if (static_cast<int>(m_files.size()) > MAX_RECENT_FILES) {
		m_files.pop_back();
	}
#endif

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
	config->QCM_writeEntry("Files", m_files);
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
	m_files = config->QCM_readListEntry("Files");
	config->endGroup();

#if QT_VERSION >= 0x040000
	while (m_files.size() > MAX_RECENT_FILES) {
		m_files.removeLast();
	}
#else
	while (static_cast<int>(m_files.size()) > MAX_RECENT_FILES) {
		m_files.pop_back();
	}
#endif

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
#if QT_VERSION >= 0x040000
		act->setText(QString("&%1 %2").arg(++i).arg(*it));
		act->setData(*it);
#else
		act->setText(*it);
		++i;
#endif
		connect(act, QCM_SIGNAL_triggered, this, SLOT(openRecentFile()));
		QCM_addAction(this, act);
	}
	if (i > 0) {
#if QT_VERSION >= 0x040000
		addSeparator();
#else
		insertSeparator();
#endif
		QAction* clearListAction = new QAction(this);
		clearListAction->QCM_setMenuText(i18n("&Clear List"));
		connect(clearListAction, QCM_SIGNAL_triggered, this, SLOT(clearList()));
		QCM_addAction(this, clearListAction);
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
#if QT_VERSION >= 0x040000
	QAction* action = qobject_cast<QAction*>(sender());
	if (action) {
		emit loadFile(action->data().toString());
	}
#else
	const QAction* action = dynamic_cast<const QAction*>(sender());
	if (action) {
		emit loadFile(action->text());
	}
#endif
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
