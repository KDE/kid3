/**
 * \file miscconfig.cpp
 * Miscellaneous Configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Jan 2004
 */

#include <qstring.h>
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#else
#if QT_VERSION >= 300
#include <qsettings.h>
#else
#include "generalconfig.h"
#endif
#endif

#include "filelist.h"
#include "miscconfig.h"

/**
 * Constructor.
 *
 * @param group configuration group
 */
MiscConfig::MiscConfig(const QString &group) : GeneralConfig(group)
{
	formatWhileEditing = false;
	nameFilter = FileList::defaultNameFilter;
	formatItem = 0;
	formatText = Mp3File::fnFmtList[0];
}

/**
 * Destructor.
 */
MiscConfig::~MiscConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 * @param group  configuration group
 */
void MiscConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	QSettings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("NameFilter", nameFilter);
	config->writeEntry("FormatItem", formatItem);
	config->writeEntry("FormatText", formatText);
	config->writeEntry("FormatWhileEditing", formatWhileEditing);
	config->writeEntry("SplitterSizes", splitterSizes);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/NameFilter", nameFilter);
	config->writeEntry("/FormatItem", formatItem);
	config->writeEntry("/FormatText", formatText);
	config->writeEntry("/FormatWhileEditing", formatWhileEditing);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 * @param group  configuration group
 */
void MiscConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	QSettings *config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	nameFilter =
	    config->readEntry("NameFilter", FileList::defaultNameFilter);
	formatItem =
	    config->readNumEntry("FormatItem", 0);
	formatWhileEditing = config->readBoolEntry("FormatWhileEditing", formatWhileEditing);
#if QT_VERSION >= 300
	formatText =
	    config->readEntry("FormatText", Mp3File::fnFmtList[0]);
#endif
	splitterSizes = config->readIntListEntry("SplitterSizes");
#else
	config->beginGroup("/" + group);
	nameFilter =
	    config->readEntry("/NameFilter", FileList::defaultNameFilter);
	formatItem =
	    config->readNumEntry("/FormatItem", 0);
	formatWhileEditing = config->readBoolEntry("/FormatWhileEditing", formatWhileEditing);
#if QT_VERSION >= 300
	formatText =
	    config->readEntry("/FormatText", Mp3File::fnFmtList[0]);
#endif
	config->endGroup();
#endif
}
