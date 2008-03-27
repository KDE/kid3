/**
 * \file rendirdialog.h
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 *
 * Copyright (C) 2004-2007  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RENDIRDIALOG_H
#define RENDIRDIALOG_H

#include <qdialog.h>
#include <qstring.h>
/** Base class for main window. */
#if QT_VERSION >= 0x040000
#include <QList>
#if QT_VERSION >= 0x040300
#include <QWizard>
typedef QWizard RenDirDialogBaseClass;
#else
typedef QDialog RenDirDialogBaseClass;
#endif
#else
#include <qvaluelist.h>
#include <qwizard.h>
typedef QWizard RenDirDialogBaseClass;
#endif

class QComboBox;
class QLabel;
class TaggedFile;
class QVBoxLayout;
class QTextEdit;

/**
 * Rename directory dialog.
 */
class RenDirDialog : public RenDirDialogBaseClass {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent     parent widget
	 */
	RenDirDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~RenDirDialog();

	/**
	 * Start dialog.
	 *
	 * @param taggedFile file to use for rename preview
	 * @param dirName    if taggedFile is 0, the directory can be set here
	 */
	void startDialog(TaggedFile* taggedFile, const QString& dirName = QString());

	/**
	 * Set new directory name.
	 *
	 * @param dir new directory name
	 */
	void setNewDirname(const QString& dir);

	/**
	 * Get new directory name.
	 *
	 * @return new directory name.
	 */
	QString getNewDirname() const;

	/**
	 * Generate new directory name according to current settings.
	 *
	 * @param taggedFile file to get information from
	 * @param olddir pointer to QString to place old directory name into,
	 *               NULL if not used
	 *
	 * @return new directory name.
	 */
	QString generateNewDirname(TaggedFile* taggedFile, QString* olddir);

	/**
	 * Clear the rename actions.
	 * This method has to be called before scheduling new actions.
	 */
	void clearActions();

	/**
	 * Schedule the actions necessary to rename the directory containing a file.
	 *
	 * @param taggedFile file in directory
	 */
	void scheduleAction(TaggedFile* taggedFile);

	/**
	 * Perform the scheduled rename actions.
	 *
	 * @param errorMsg if not 0 and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 */
	void performActions(QString* errorMsg);

	/**
	 * Set directory format string.
	 *
	 * @param fmt directory format
	 */
	void setDirectoryFormat(const QString& fmt);

	/**
	 * Set action.
	 *
	 * @param create true to create, false to rename
	 */ 
	void setAction(bool create);

	/**
	 * Set tag source
	 *
	 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
	 */
	void setTagSource(int tagMask);

	/**
	 * Check if dialog was aborted.
	 * @return true if aborted.
	 */
	bool getAbortFlag() { return m_aborted; }

protected:
	/**
	 * Called when the wizard is canceled.
	 */
	virtual void reject();

#if QT_VERSION < 0x040000
	/**
	 * Called when the wizard is finished.
	 */
	virtual void accept();
#endif

signals:
	/**
	 * Emitted when scheduling of actions using clearActions() followed by
	 * one or multiple scheduleAction() calls is requested.
	 */
	void actionSchedulingRequested();

private slots:
	/**
	 * Set new directory name according to current settings.
	 */
	void slotUpdateNewDirname();

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

	/**
	 * Request action scheduling and then accept dialog.
	 */
	void requestActionSchedulingAndAccept();

	/**
	 * Wizard page changed.
	 */
	void pageChanged();

private:
	/**
	 * An action performed while renaming a directory.
	 */
	class RenameAction {
	public:
		/** Action type. */
		enum Type {
			CreateDirectory,
			RenameDirectory,
			RenameFile,
			ReportError,
			NumTypes
		};

		/**
		 * Constructor.
		 * @param type type of action
		 * @param src  source file or directory name
		 * @param dest destination file or directory name
		 */
		RenameAction(Type type, const QString& src, const QString& dest) :
			m_type(type), m_src(src), m_dest(dest) {}

		/**
		 * Constructor.
		 */
		RenameAction() : m_type(ReportError) {}

		/**
		 * Destructor.
		 */
		~RenameAction() {}

		/**
		 * Test for equality.
		 * @param rhs right hand side
		 * @return true if equal.
		 */
		bool operator==(const RenameAction& rhs) const {
			return m_type == rhs.m_type && m_src == rhs.m_src && m_dest == rhs.m_dest;
		}

		Type m_type;    /**< type of action */
		QString m_src;  /**< source file or directory name */
		QString m_dest; /**< destination file or directory name */
	};

	/** List of rename actions. */
#if QT_VERSION >= 0x040000
	typedef QList<RenameAction> RenameActionList;
#else
	typedef QValueList<RenameAction> RenameActionList;
#endif

	enum Action { ActionRename = 0, ActionCreate = 1 };
	enum TagVersion { TagV2V1 = 0, TagV1 = 1, TagV2 = 2 };

	/**
	 * Set up the main wizard page.
	 *
	 * @param page    widget
	 * @param vlayout layout
	 */
	void setupMainPage(QWidget* page, QVBoxLayout* vlayout);

	/**
	 * Set up the preview wizard page.
	 *
	 * @param page widget
	 */
	void setupPreviewPage(QWidget* page);

	/**
	 * Create a directory if it does not exist.
	 *
	 * @param dir      directory path
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if directory exists or was created successfully.
	 */
	bool createDirectory(const QString& dir,
						 QString* errorMsg) const;

	/**
	 * Rename a directory.
	 *
	 * @param olddir   old directory name
	 * @param newdir   new directory name
	 * @param errorMsg if not NULL and an error occurred, a message is
	 *                 appended here, otherwise it is not touched
	 *
	 * @return true if rename successful.
	 */
	bool renameDirectory(
		const QString& olddir, const QString& newdir, QString* errorMsg) const;

	/**
	 * Rename a file.
	 *
	 * @param oldfn    old file name
	 * @param newfn    new file name
	 * @param errorMsg if not NULL and an error occurred, a message is
	 *                 appended here, otherwise it is not touched
	 *
	 * @return true if rename successful or newfn already exists.
	 */
	bool renameFile(const QString& oldfn, const QString& newfn,
									QString* errorMsg) const;

  /**
	 * Add a rename action.
	 *
	 * @param type type of action
	 * @param src  source file or directory name
	 * @param dest destination file or directory name
	 */
	void addAction(RenameAction::Type type, const QString& src, const QString& dest);

  /**
	 * Add a rename action.
	 *
	 * @param type type of action
	 * @param dest destination file or directory name
	 */
	void addAction(RenameAction::Type type, const QString& dest);

	/**
	 * Check if there is already an action scheduled for this source.
	 *
	 * @return true if a rename action for the source exists.
	 */
	bool actionHasSource(const QString& src) const;

	/**
	 * Check if there is already an action scheduled for this destination.
	 *
	 * @return true if a rename or create action for the destination exists.
	 */
	bool actionHasDestination(const QString& dest) const;

	/**
	 * Replace directory name if there is already a rename action.
	 *
	 * @param src directory name, will be replaced if there is a rename action
	 */
	void replaceIfAlreadyRenamed(QString& src) const;

	/**
	 * Clear action preview.
	 */
	void clearPreview();

	/**
	 * Display action preview.
	 */
	void displayPreview();

	QComboBox* m_formatComboBox;
	QComboBox* m_actionComboBox;
	QComboBox* m_tagversionComboBox;
	QLabel* m_currentDirLabel;
	QLabel* m_newDirLabel;
  QTextEdit* m_edit;
	TaggedFile* m_taggedFile;
	RenameActionList m_actions;
	bool m_aborted;
};

#endif
