/**
 * \file formatconfig.h
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#ifndef FORMATCONFIG_H
#define FORMATCONFIG_H

#include "config.h"
#include "generalconfig.h"
#include <qmap.h>

#ifdef CONFIG_USE_KDE
class KConfig;
#else
class QSettings;
#endif
class QString;
class StandardTags;

class FormatConfig : public GeneralConfig
{
public:
	/** Case conversion options. */
	enum CaseConversion {
	    NoChanges,
	    AllLowercase,
	    AllUppercase,
	    FirstLetterUppercase,
	    AllFirstLettersUppercase,
	    NumCaseConversions
	};
	/**
	 * Constructor.
	 *
	 * @param grp configuration group
	 */
	FormatConfig(const QString &grp);
	/**
	 * Destructor.
	 */
	virtual ~FormatConfig();
	/**
	 * Set specific properties for a filename format.
	 * This will set default string conversions and not touch the file
	 * extension when formatting.
	 */
	void setAsFilenameFormatter();
	/**
	 * Format a string using this configuration.
	 *
	 * @param str string to format
	 */
	void formatString(QString& str) const;
	/**
	 * Format title, artist and album using this configuration.
	 *
	 * @param st standard tags
	 */
	void formatStandardTags(StandardTags& st) const;
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
	/** Case conversion option */
	CaseConversion caseConversion;
	/** true if string replacement enabled */
	bool strRepEnabled;
	/** Mapping for string replacement */
	QMap<QString, QString> strRepMap;
private:
	/** true if it is a file formatter */
	bool filenameFormatter;
};

#endif
