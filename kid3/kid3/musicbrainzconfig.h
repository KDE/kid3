/**
 * \file musicbrainzconfig.h
 * MusicBrainz configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#ifndef MUSICBRAINZCONFIG_H
#define MUSICBRAINZCONFIG_H

#include "config.h"
#ifdef HAVE_TUNEPIMP
#include <qstringlist.h>
#include "generalconfig.h"

/**
 * MusicBrainz configuration.
 */
class MusicBrainzConfig : public GeneralConfig {
public:
	/**
	 * Constructor.
	 * Set default configuration.
	 *
	 * @param grp configuration group
	 */
	MusicBrainzConfig(const QString &grp);

	/**
	 * Constructor.
	 * Use to create temporary configuration.
	 */
	MusicBrainzConfig();

	/**
	 * Destructor.
	 */
	virtual ~MusicBrainzConfig();

	/**
	 * Persist configuration.
	 *
	 * @param config KDE configuration
	 */
	virtual void writeToConfig(
#ifdef CONFIG_USE_KDE
		KConfig *config
#else
		Kid3Settings *config
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
		Kid3Settings *config
#endif
		);

	/** true if musicBrainz proxy is used */
	bool m_useProxy;

	/** proxy used for musicBrainz.org access */
	QString m_proxy;

	/** MusicBrainz server */
	QString m_server;
};

#endif // HAVE_TUNEPIMP

#endif // MUSICBRAINZCONFIG_H
