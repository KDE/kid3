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
#include <qstringlist.h>
#include "config.h"
#include "generalconfig.h"

class QString;
class StandardTags;

/**
 * Miscellaneous Configuration.
 */
class MiscConfig : public GeneralConfig
{
public:
	/** The ID3v2 version used for new tags. */
	enum Id3v2Version {
		ID3v2_3_0 = 0,
		ID3v2_4_0 = 1
	};

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
		Kid3Settings *config
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
		Kid3Settings *config
#endif
		);

	/** true to write total number of tracks into track fields */
	bool m_enableTotalNumberOfTracks;
	/** true to preserve file time stamps */
	bool m_preserveTime;
	/** field name used for Vorbis comment entries */
	QString m_commentName;
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
	/** size of file/dirlist splitter */
	QValueList<int> m_vSplitterSizes;
	/** commands available in context menu */
	QStringList m_contextMenuCommands;
	/** custom genres for ID3v2.3 */
	QStringList m_customGenres;
	/** true to hide ID3v1.1 controls */
	bool m_hideV1;
	/** true to hide ID3v2.3 controls */
	bool m_hideV2;
	/** version used for new ID3v2 tags */
	int m_id3v2Version;
#ifndef CONFIG_USE_KDE
	/** mainwindow width */
	int windowWidth;
	/** mainwindow height */
	int windowHeight;
#endif

	/** Default name filter */
	static const QString defaultNameFilter;
	/** Default value for comment name */
	static const QString defaultCommentName;
	/** Default filename format list */
	static const char** defaultFnFmtList;
	/** Default directory format list */
	static const char** defaultDirFmtList;
};

#endif
