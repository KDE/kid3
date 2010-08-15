/**
 * \file recentfilesmenu.h
 * Menu to open a recent files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15-Aug-2010
 */

#ifndef RECENTFILESMENU_H
#define RECENTFILESMENU_H

#include "config.h"

#ifndef CONFIG_USE_KDE

#include <qstringlist.h>

#include "generalconfig.h"

/** Base class for main window. */
#if QT_VERSION >= 0x040000
#include <qmenu.h>
typedef QMenu RecentFilesMenuBaseClass;
#else
#include <qpopupmenu.h>
typedef QPopupMenu RecentFilesMenuBaseClass;
#endif

class RecentFilesMenu : public RecentFilesMenuBaseClass
{
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	explicit RecentFilesMenu(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~RecentFilesMenu();

	/**
	 * Add directory to list of recent files.
	 *
	 * @param path path to directory
	 */
	void addDirectory(const QString& dir);
	
  /**
   * Saves the current recent files entries to a given configuration.
   *
   * @param config configuration settings
   */
	void saveEntries(Kid3Settings* config);

  /**
   * Loads the recent files entries from a given configuration.
   *
   * @param config configuration settings
   */
	void loadEntries(Kid3Settings* config);

signals:
	/**
	 * Emitted when a recent file has to be loaded.
	 * Parameter: path to file or directory
	 */
	void loadFile(const QString&);

private slots:
	/**
	 * Emit a load file signal when a recent file has to be loaded.
	 */
	void openRecentFile();

	/**
	 * Clear the list of recent files.
	 */
	void clearList();

private:
	/**
	 * Update the recent file actions.
	 */
	void updateRecentFileActions();

	QStringList m_files;
};

#endif // !CONFIG_USE_KDE

#endif // RECENTFILESMENU_H
