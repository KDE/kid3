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
	 * @param group  configuration group
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
	 * @param group  configuration group
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
	/** size of splitter in main window */
	QValueList<int> splitterSizes;
};

#endif
