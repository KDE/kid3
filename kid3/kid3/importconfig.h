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
	ImportConfig(const QString& grp);

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
		KConfig* config
#else
		Kid3Settings* config
#endif
		) const;

	/**
	 * Read persisted configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		);

	/** true to import into ID3v1 tags, else into ID3v2 tags */
	bool m_importDestV1;
	/** Names of import formats */
	QStringList m_importFormatNames;
	/** regexp describing header import format */
	QStringList m_importFormatHeaders;
	/** regexp describing track import format */
	QStringList m_importFormatTracks;
	/** selected import format */
	int m_importFormatIdx;
	/** check maximum allowable time difference */
	bool m_enableTimeDifferenceCheck;
	/** maximum allowable time difference */
	int m_maxTimeDifference;

	/** true to export ID3v1 tags, else ID3v2 tags */
	bool m_exportSrcV1;
	/** Names of export formats */
	QStringList m_exportFormatNames;
	/** regexp describing header export format */
	QStringList m_exportFormatHeaders;
	/** regexp describing track export format */
	QStringList m_exportFormatTracks;
	/** regexp describing trailer export format */
	QStringList m_exportFormatTrailers;
	/** selected export format */
	int m_exportFormatIdx;
	/** export window width */
	int m_exportWindowWidth;
	/** export window height */
	int m_exportWindowHeight;
};

#endif
