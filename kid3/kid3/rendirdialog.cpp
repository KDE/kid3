/**
 * \file rendirdialog.cpp
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qdir.h>

#include "taggedfile.h"
#include "standardtags.h"
#include "kid3.h"
#include "miscconfig.h"
#include "rendirdialog.h"

/**
 * Constructor.
 *
 * @param parent     parent widget
 * @param caption    dialog title
 * @param taggedFile file to use for rename preview
 */
RenDirDialog::RenDirDialog(QWidget *parent, const QString &caption,
						   TaggedFile *taggedFile) :
	QDialog(parent, "rendir", true), m_taggedFile(taggedFile)
{
	setCaption(caption);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout *actionLayout = new QHBoxLayout(vlayout);
	actionComboBox = new QComboBox(this);
	tagversionComboBox = new QComboBox(this);
	if (actionComboBox && tagversionComboBox) {
		actionComboBox->insertItem(i18n("Rename Directory"), ActionRename);
		actionComboBox->insertItem(i18n("Create Directory"), ActionCreate);
		actionLayout->addWidget(actionComboBox);
		connect(actionComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		tagversionComboBox->insertItem(i18n("From Tag 1"), TagV1);
		tagversionComboBox->insertItem(i18n("From Tag 2"), TagV2);
		actionLayout->addWidget(tagversionComboBox);
		connect(tagversionComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
	}
	QHBoxLayout *formatLayout = new QHBoxLayout(vlayout);
	QLabel *formatLabel = new QLabel(i18n("&Format:"), this);
	formatComboBox = new QComboBox(this);
	if (formatLayout && formatLabel && formatComboBox) {
		formatComboBox->insertStrList(MiscConfig::defaultDirFmtList);
		formatComboBox->setEditable(true);
		formatComboBox->setCurrentItem(Kid3App::s_miscCfg.dirFormatItem);
		formatComboBox->setCurrentText(Kid3App::s_miscCfg.dirFormatText);
		tagversionComboBox->setCurrentItem(
			Kid3App::s_miscCfg.m_renDirSrcV1 ? TagV1 : TagV2);
		formatLabel->setBuddy(formatComboBox);
		formatLayout->addWidget(formatLabel);
		formatLayout->addWidget(formatComboBox);
		connect(formatComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		connect(formatComboBox, SIGNAL(textChanged(const QString &)), this, SLOT(slotUpdateNewDirname()));
	}

	QGridLayout *fromToLayout = new QGridLayout(vlayout, 2, 2);
	QLabel *fromLabel = new QLabel(i18n("From:"), this);
	currentDirLabel = new QLabel(this);
	QLabel *toLabel = new QLabel(i18n("To:"), this);
	newDirLabel = new QLabel(this);
	if (fromToLayout && fromLabel && currentDirLabel &&
		toLabel && newDirLabel) {
		fromToLayout->addWidget(fromLabel, 0, 0);
		fromToLayout->addWidget(currentDirLabel, 0, 1);
		fromToLayout->addWidget(toLabel, 1, 0);
		fromToLayout->addWidget(newDirLabel, 1, 1);
	}

	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton *helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton *saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton *okButton = new QPushButton(i18n("&OK"), this);
	QPushButton *cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && saveButton && okButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(TRUE);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}
	slotUpdateNewDirname();
}

/**
 * Destructor.
 */
RenDirDialog::~RenDirDialog()
{}

/**
 * Get parent directory.
 *
 * @param dir directory
 *
 * @return parent directory (terminated by separator),
 *         empty string if no separator in dir.
 */
static QString parentDirectory(const QString &dir)
{
	QString parent(dir);
	int slashPos = parent.findRev('/');
	if (slashPos != -1) {
		parent.truncate(slashPos + 1);
	} else {
		parent = "";
	}
	return parent;
}

/**
 * Create a directory if it does not exist.
 *
 * @param dir      directory path
 * @param errorMsg if not NULL and an error occurred, a message is appended here,
 *                 otherwise it is not touched
 *
 * @return true if directory exists or was created successfully.
 */
bool RenDirDialog::createDirectory(const QString &dir,
								   QString *errorMsg) const
{
	if (QFileInfo(dir).isDir() ||
		(QDir().mkdir(dir) && QFileInfo(dir).isDir())) {
		return true;
	} else {
		if (errorMsg) {
			errorMsg->append(i18n("Create directory %1 failed\n").arg(dir));
		}
		return false;
	}
}

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
bool RenDirDialog::renameDirectory(
	const QString &olddir, const QString &newdir, QString *errorMsg) const
{
	if (QFileInfo(newdir).exists()) {
		if (errorMsg) {
			errorMsg->append(i18n("File %1 already exists\n").arg(newdir));
		}
		return false;
	}
	if (!QFileInfo(olddir).isDir()) {
		if (errorMsg) {
			errorMsg->append(i18n("%1 is not a directory\n").arg(olddir));
		}
		return false;
	}
	if (QDir().rename(olddir, newdir) && QFileInfo(newdir).isDir()) {
		return true;
	} else {
		if (errorMsg) {
			errorMsg->append(i18n("Rename %1 to %2 failed\n").arg(olddir).arg(newdir));
		}
		return false;
	}
}

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
bool RenDirDialog::renameFile(const QString &oldfn, const QString &newfn,
							  QString *errorMsg) const
{
	if (QFileInfo(newfn).isFile()) {
		return true;
	}
	if (QFileInfo(newfn).exists()) {
		if (errorMsg) {
			errorMsg->append(i18n("%1 already exists\n").arg(newfn));
		}
		return false;
	}
	if (!QFileInfo(oldfn).isFile()) {
		if (errorMsg) {
			errorMsg->append(i18n("%1 is not a file\n").arg(oldfn));
		}
		return false;
	}
	if (QDir().rename(oldfn, newfn) && QFileInfo(newfn).isFile()) {
		return true;
	} else {
		if (errorMsg) {
			errorMsg->append(i18n("Rename %1 to %2 failed\n").arg(oldfn).arg(newfn));
		}
		return false;
	}
}

/**
 * Generate new directory name according to current settings.
 *
 * @param taggedFile file to get information from
 * @param newdir pointer to QString to place old directory name into,
 *               NULL if not used
 *
 * @return new directory name.
 */
QString RenDirDialog::generateNewDirname(TaggedFile *taggedFile, QString *olddir)
{
	StandardTags st;
	taggedFile->readTags(false);
	if (tagversionComboBox->currentItem() == TagV1) {
		taggedFile->getStandardTagsV1(&st);
	} else {
		taggedFile->getStandardTagsV2(&st);
	}
	QString newdir(taggedFile->getDirname());
	if (newdir.endsWith(QChar('/'))) {
		// remove trailing separator
		newdir.truncate(newdir.length() - 1);
	}
	if (olddir) {
		*olddir = newdir;
	}
	if (actionComboBox->currentItem() == ActionRename) {
		newdir = parentDirectory(newdir);
	} else if (!newdir.isEmpty()) {
		newdir.append('/');
	}
	newdir.append(taggedFile->formatWithTags(&st, formatComboBox->currentText(), true));
	return newdir;
}

/**
 * Get new directory name.
 *
 * @return new directory name.
 */
void RenDirDialog::setNewDirname(const QString &dir)
{
	newDirLabel->setText(dir);
}

/**
 * Get new directory name.
 *
 * @return new directory name.
 */
QString RenDirDialog::getNewDirname(void) const
{
	return newDirLabel->text();
}

/**
 * Set new directory name according to current settings.
 */
void RenDirDialog::slotUpdateNewDirname()
{
	if (m_taggedFile) {
		QString currentDirname;
		QString newDirname(generateNewDirname(m_taggedFile, &currentDirname));
		currentDirLabel->setText(currentDirname);
		setNewDirname(newDirname);
	}
}

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
bool RenDirDialog::performAction(TaggedFile *taggedFile, bool& again, QString *errorMsg)
{
	QString currentDirname;
	QString newDirname(generateNewDirname(taggedFile, &currentDirname));
	bool result = true;
	again = false;
	if (newDirname != currentDirname) {
		if (newDirname.startsWith(currentDirname)) {
			// A new directory is created in the current directory.
			bool createDir = true;
			QString dirWithFiles(currentDirname);
			for (int i = 0;
				 createDir && newDirname.startsWith(currentDirname) && i < 5;
				 i++) {
				QString newPart(newDirname.mid(currentDirname.length()));
				// currentDirname does not end with a separator, so newPart
				// starts with a separator and the search starts with the
				// second character.
				int slashPos = newPart.find('/', 1);
				if (slashPos != -1 && slashPos != (int)newPart.length() - 1) {
					newPart.truncate(slashPos);
					// the new part has multiple directories
					// => create one directory
				} else {
					createDir = false;
				}
				// Create a directory for each file and move it.
				if (createDirectory(currentDirname + newPart, errorMsg)) {
					if (!createDir) {
						renameFile(dirWithFiles + '/' +
								   taggedFile->getFilename(),
								   currentDirname + newPart +
								   '/' + taggedFile->getFilename(),
								   errorMsg);
					}
					currentDirname = currentDirname + newPart;
				}
			}
		} else {
			QString parent(parentDirectory(currentDirname));
			if (newDirname.startsWith(parent)) {
				QString newPart(newDirname.mid(parent.length()));
				int slashPos = newPart.find('/');
				if (slashPos != -1 && slashPos != (int)newPart.length() - 1) {
					newPart.truncate(slashPos);
					// the new part has multiple directories
					// => rename current directory, then create additional
					// directories.
					again = true;
				}
				if (QFileInfo(parent + newPart).isDir()) {
					// directory already exists => move files
					if (renameFile(currentDirname + '/' +
								   taggedFile->getFilename(),
								   parent + newPart +
								   '/' + taggedFile->getFilename(),
								   errorMsg)) {
						currentDirname = parent + newPart;
					}
				} else {
					if (renameDirectory(currentDirname, parent + newPart, errorMsg)) {
						currentDirname = parent + newPart;
						// Current directory is changed => finish operation
						result = false;
					}
				}
			} else {
				// new directory name is too different
				if (errorMsg) {
					errorMsg->append(i18n("New directory name is too different\n"));
				}
			}
		}
	}
	setNewDirname(currentDirname);
	return result;
}

/**
 * Save the local settings to the configuration.
 */
void RenDirDialog::saveConfig()
{
	Kid3App::s_miscCfg.dirFormatItem = formatComboBox->currentItem();
	Kid3App::s_miscCfg.dirFormatText = formatComboBox->currentText();
	Kid3App::s_miscCfg.m_renDirSrcV1 =
		(tagversionComboBox->currentItem() == TagV1);
}

/**
 * Show help.
 */
void RenDirDialog::showHelp()
{
	Kid3App::displayHelp("rename-directory");
}
