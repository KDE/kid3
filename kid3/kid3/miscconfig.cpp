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
#include "mp3file.h"
#include "rendirdialog.h"
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
	dirFormatItem = 0;
	dirFormatText = RenDirDialog::dirFmtList[0];
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
	config->writeEntry("DirFormatItem", dirFormatItem);
	config->writeEntry("DirFormatText", dirFormatText);
	config->writeEntry("FormatWhileEditing", formatWhileEditing);
	config->writeEntry("SplitterSizes", splitterSizes);
#else
	config->beginGroup("/" + group);
	config->writeEntry("/NameFilter", nameFilter);
	config->writeEntry("/FormatItem", formatItem);
	config->writeEntry("/FormatText", formatText);
	config->writeEntry("/DirFormatItem", dirFormatItem);
	config->writeEntry("/DirFormatText", dirFormatText);
	config->writeEntry("/FormatWhileEditing", formatWhileEditing);
#if QT_VERSION >= 300
	QValueList<int>::const_iterator it;
#else
	QValueListConstIterator<int> it;
#endif
	int i;
	for (it = splitterSizes.begin(), i = 0;
		 it != splitterSizes.end();
		 ++it, ++i) {
		config->writeEntry("/SplitterSize" + QString::number(i), *it);
	}
	config->writeEntry("/WindowWidth", windowWidth);
	config->writeEntry("/WindowHeight", windowHeight);
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
	dirFormatItem =
	    config->readNumEntry("DirFormatItem", 0);
	formatWhileEditing = config->readBoolEntry("FormatWhileEditing", formatWhileEditing);
#if QT_VERSION >= 300
	formatText =
	    config->readEntry("FormatText", Mp3File::fnFmtList[0]);
	dirFormatText =
	    config->readEntry("DirFormatText", RenDirDialog::dirFmtList[0]);
#endif
	splitterSizes = config->readIntListEntry("SplitterSizes");
#else
	config->beginGroup("/" + group);
	nameFilter =
	    config->readEntry("/NameFilter", FileList::defaultNameFilter);
	formatItem =
	    config->readNumEntry("/FormatItem", 0);
	dirFormatItem =
	    config->readNumEntry("/DirFormatItem", 0);
	formatWhileEditing = config->readBoolEntry("/FormatWhileEditing", formatWhileEditing);
#if QT_VERSION >= 300
	formatText =
	    config->readEntry("/FormatText", Mp3File::fnFmtList[0]);
	dirFormatText =
	    config->readEntry("/DirFormatText", RenDirDialog::dirFmtList[0]);
	QValueList<int>::iterator it;
#else
	QValueListIterator<int> it;
#endif
	splitterSizes.clear();
	for (int i = 0; i < 5; ++i) {
		int val = config->readNumEntry("/SplitterSize" + QString::number(i), -1);
		if (val != -1) {
#if QT_VERSION >= 300
			splitterSizes.push_back(val);
#else
			splitterSizes.append(val);
#endif
		} else {
			break;
		}
	}
	windowWidth = config->readNumEntry("/WindowWidth", -1);
	windowHeight = config->readNumEntry("/WindowHeight", -1);
	config->endGroup();
#endif
}
