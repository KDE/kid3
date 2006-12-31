/**
 * \file rendirdialog.h
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 */

#ifndef RENDIRDIALOG_H
#define RENDIRDIALOG_H

#include <qdialog.h>
#include <qstring.h>

class QComboBox;
class QLabel;
class TaggedFile;

/**
 * Rename directory dialog.
 */
class RenDirDialog : public QDialog {
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent     parent widget
	 * @param caption    dialog title
	 * @param taggedFile file to use for rename preview
	 */
	RenDirDialog(QWidget *parent, const QString &caption,
				 TaggedFile *taggedFile);
	/**
	 * Destructor.
	 */
	~RenDirDialog();
	/**
	 * Get new directory name.
	 *
	 * @return new directory name.
	 */
	void setNewDirname(const QString &dir);
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
	QString generateNewDirname(TaggedFile *taggedFile, QString *olddir);
	/**
	 * Perform renaming or creation of directory according to current settings.
	 *
	 * @param taggedFile file to take directory from and to move
	 * @param again    is set true if the new directory (getNewDirname())
	 *                 should be read and processed again
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if other files can be processed,
	 *         false if operation is finished.
	 */
	bool performAction(TaggedFile *taggedFile, bool& again, QString *errorMsg);

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

private:
	/**
	 * Create a directory if it does not exist.
	 *
	 * @param dir      directory path
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if directory exists or was created successfully.
	 */
	bool createDirectory(const QString &dir,
						 QString *errorMsg) const;

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
		const QString &olddir, const QString &newdir, QString *errorMsg) const;

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
	bool renameFile(const QString &oldfn, const QString &newfn,
					QString *errorMsg) const;

	enum Action { ActionRename = 0, ActionCreate = 1 };
	enum TagVersion { TagV1 = 0, TagV2 = 1 };
	QComboBox *formatComboBox;
	QComboBox *actionComboBox;
	QComboBox *tagversionComboBox;
	QLabel *currentDirLabel;
	QLabel *newDirLabel;
	TaggedFile* m_taggedFile;
};

#endif
