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

#include <qstringlist.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3ValueList>
#else
#include <qvaluelist.h>
#endif
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
	/** true to rename directory from ID3v1 tags, else ID3v2 tags */
	bool m_renDirSrcV1;
	/** size of splitter in main window */
	Q3ValueList<int> splitterSizes;
	/** size of file/dirlist splitter */
	Q3ValueList<int> m_vSplitterSizes;
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
	/** true if proxy is used */
	bool m_useProxy;
	/** proxy used for access */
	QString m_proxy;
#ifndef CONFIG_USE_KDE
	/** mainwindow width */
	int windowWidth;
	/** mainwindow height */
	int windowHeight;
#endif

	/** Default name filter */
	static const char* const defaultNameFilter;
	/** Default value for comment name */
	static const char* const defaultCommentName;
	/** Default filename format list */
	static const char** defaultFnFmtList;
	/** Default directory format list */
	static const char** defaultDirFmtList;
};

#endif
