/**
 * \file importconfig.h
 * Configuration for import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef IMPORTCONFIG_H
#define IMPORTCONFIG_H

#include <qstringlist.h>
#include "config.h"
#include "generalconfig.h"

#ifdef CONFIG_USE_KDE
class KConfig;
#else
class QSettings;
#endif

/**
 * Import configuration.
 */
class ImportConfig : public GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	ImportConfig(const QString &grp);
	/**
	 * Destructor.
	 */
	virtual ~ImportConfig();
	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
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
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		QSettings *config
#endif
		);
	/** true to import into ID3v1 tags, else into ID3v2 tags */
	bool importDestV1;
	/** Names of import formats */
	QStringList importFormatNames;
	/** regexp describing header import format */
	QStringList importFormatHeaders;
	/** regexp describing track import format */
	QStringList importFormatTracks;
	/** selected import format */
	int importFormatIdx;
	/** check maximum allowable time difference */
	bool enableTimeDifferenceCheck;
	/** maximum allowable time difference */
	int maxTimeDifference;
};

#endif
