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
class Mp3File;

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
	 * @param mp3        MP3 file to use for rename preview
	 * @param formatItem directory format item
	 * @param formatText directory format
	 */
	RenDirDialog(QWidget *parent, const QString &caption,
				 Mp3File *mp3,
				 int formatItem, const QString &formatText);
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
	QString getNewDirname(void) const;
	/**
	 * Generate new directory name according to current settings.
	 *
	 * @param mp3    MP3 file to get information from
	 * @param olddir pointer to QString to place old directory name into,
	 *               NULL if not used
	 *
	 * @return new directory name.
	 */
	QString generateNewDirname(Mp3File *mp3, QString *olddir);
	/**
	 * Perform renaming or creation of directory according to current settings.
	 *
	 * @param mp3      MP3 file to take directory from and to move
	 * @param again    is set true if the new directory (getNewDirname())
	 *                 should be read and processed again
	 * @param errorMsg if not NULL and an error occurred, a message is appended here,
	 *                 otherwise it is not touched
	 *
	 * @return true if other files can be processed,
	 *         false if operation is finished.
	 */
	bool performAction(Mp3File *mp3, bool& again, QString *errorMsg);
	/**
	 * Get currently selected directory format item.
	 *
	 * @return index of directory format.
	 */
	int getFormatItem(void) const;
	/**
	 * Get currently used directory format.
	 *
	 * @return directory format.
	 */
	QString getFormatText(void) const;
	/** List with directory name formats */
	static const char **dirFmtList;

private slots:
	/**
	 * Set new directory name according to current settings.
	 */
	void slotUpdateNewDirname();

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
	Mp3File *mp3file;
};

#endif
