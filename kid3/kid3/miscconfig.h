/**
 * \file miscconfig.h
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 */

#ifndef MISCCONFIG_H
#define MISCCONFIG_H

#include <qvaluelist.h>
#include "config.h"
#include "generalconfig.h"

#ifdef CONFIG_USE_KDE
class KConfig;
#else
class QSettings;
#endif
class QString;
class StandardTags;

/**
 * Miscellaneous Configuration.
 */
class MiscConfig : public GeneralConfig
{
public:
	/**
	 * Constructor.
	 */
	MiscConfig(const QString &group);
	/**
	 * Destructor.
	 */
	virtual ~MiscConfig();
	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		) const;
	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		);

	/** true to enable formating in line edits */
	bool formatWhileEditing;
	/** filter of file names to be opened */
	QString nameFilter;
	/** filename format */
	QString formatText;
	/** index of filename format selected */
	int formatItem;
	/** directory name format */
	QString dirFormatText;
	/** index of directory name format selected */
	int dirFormatItem;
	/** size of splitter in main window */
	QValueList<int> splitterSizes;
#ifndef CONFIG_USE_KDE
	/** mainwindow width */
	int windowWidth;
	/** mainwindow height */
	int windowHeight;
#endif
};

#endif
