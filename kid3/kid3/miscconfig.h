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
	 * External command in context menu.
	 */
	class MenuCommand {
	public:
		/**
		 * Constructor.
		 *
		 * @param name display name
		 * @param cmd  command string with argument codes
		 * @param config true if confirmation required
		 * @param showOutput true if output of command shall be shown
		 */
		explicit MenuCommand(const QString& name = QString::null,
												 const QString& cmd = QString::null,
												 bool confirm = false, bool showOutput = false);

		/**
		 * Constructor.
		 *
		 * @param strList string list with encoded command
		 */
		explicit MenuCommand(const QStringList& strList);

		/**
		 * Encode into string list.
		 *
		 * @return string list with encoded command.
		 */
		QStringList toStringList() const;

		/**
		 * Get the display name.
		 * @return name.
		 */
		const QString& getName() const { return m_name; }

		/**
		 * Get the command string.
		 * @return command string.
		 */
		const QString& getCommand() const { return m_cmd; }

		/**
		 * Check if command must be confirmed.
		 * @return true if command has to be confirmed.
		 */
		bool mustBeConfirmed() const { return m_confirm; }

		/**
		 * Check if command output has to be shown.
		 * @return true if command output has to be shown.
		 */
		bool outputShown() const { return m_showOutput; }

	private:
		QString m_name;
		QString m_cmd;
		bool m_confirm;
		bool m_showOutput;
	};

	/**
	 * Constructor.
	 */
	MiscConfig(const QString& group);

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
	void readFromConfig(
#ifdef CONFIG_USE_KDE
		KConfig* config
#else
		Kid3Settings* config
#endif
		);

	/** true to mark truncated ID3v1.1 fields */
	bool m_markTruncations;
	/** true to write total number of tracks into track fields */
	bool m_enableTotalNumberOfTracks;
	/** true to preserve file time stamps */
	bool m_preserveTime;
	/** field name used for Vorbis comment entries */
	QString m_commentName;
	/** filter of file names to be opened */
	QString m_nameFilter;
	/** filename format */
	QString m_formatText;
	/** index of filename format selected */
	int m_formatItem;
	/** directory name format */
	QString m_dirFormatText;
	/** index of directory name format selected */
	int m_dirFormatItem;
	/** true to rename directory from ID3v1 tags, else ID3v2 tags */
	bool m_renDirSrcV1;
	/** size of splitter in main window */
	Q3ValueList<int> m_splitterSizes;
	/** size of file/dirlist splitter */
	Q3ValueList<int> m_vSplitterSizes;
	/** commands available in context menu */
	Q3ValueList<MenuCommand> m_contextMenuCommands;
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
	/** web browser substituted for %b */
	QString m_browser;
	/** true to show only custom genres in combo boxes */
	bool m_onlyCustomGenres;
#ifndef CONFIG_USE_KDE
	/** mainwindow width */
	int m_windowWidth;
	/** mainwindow height */
	int m_windowHeight;
#endif

	/** Default name filter */
	static const char* const s_defaultNameFilter;
	/** Default value for comment name */
	static const char* const s_defaultCommentName;
	/** Default value for web browser */
	static const char* const s_defaultBrowser;
	/** Default filename format list */
	static const char** s_defaultFnFmtList;
	/** Default directory format list */
	static const char** s_defaultDirFmtList;
};

#endif
