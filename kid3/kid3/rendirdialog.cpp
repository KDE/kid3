/**
 * \file rendirdialog.cpp
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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qdir.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#endif

#include "taggedfile.h"
#include "standardtags.h"
#include "kid3.h"
#include "miscconfig.h"
#include "rendirdialog.h"

/**
 * Constructor.
 *
 * @param parent     parent widget
 * @param taggedFile file to use for rename preview
 */
RenDirDialog::RenDirDialog(QWidget* parent, TaggedFile* taggedFile) :
	QDialog(parent), m_taggedFile(taggedFile)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Rename Directory"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout* actionLayout = new QHBoxLayout;
	m_actionComboBox = new QComboBox(this);
	m_tagversionComboBox = new QComboBox(this);
	if (m_actionComboBox && m_tagversionComboBox) {
		m_actionComboBox->QCM_insertItem(ActionRename, i18n("Rename Directory"));
		m_actionComboBox->QCM_insertItem(ActionCreate, i18n("Create Directory"));
		actionLayout->addWidget(m_actionComboBox);
		connect(m_actionComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		m_tagversionComboBox->QCM_insertItem(TagV2V1, i18n("From Tag 2 and Tag 1"));
		m_tagversionComboBox->QCM_insertItem(TagV1, i18n("From Tag 1"));
		m_tagversionComboBox->QCM_insertItem(TagV2, i18n("From Tag 2"));
		actionLayout->addWidget(m_tagversionComboBox);
		connect(m_tagversionComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		vlayout->addLayout(actionLayout);
	}
	QHBoxLayout* formatLayout = new QHBoxLayout;
	QLabel* formatLabel = new QLabel(i18n("&Format:"), this);
	m_formatComboBox = new QComboBox(this);
	if (formatLayout && formatLabel && m_formatComboBox) {
		QStringList strList;
		for (const char** sl = MiscConfig::s_defaultDirFmtList; *sl != 0; ++sl) {
			strList += *sl;
		}
		m_formatComboBox->QCM_addItems(strList);
		m_formatComboBox->setEditable(true);
#if QT_VERSION >= 0x040000
		m_formatComboBox->setItemText(Kid3App::s_miscCfg.m_dirFormatItem,
																	Kid3App::s_miscCfg.m_dirFormatText);
		m_formatComboBox->setCurrentIndex(Kid3App::s_miscCfg.m_dirFormatItem);
#else
		m_formatComboBox->setCurrentItem(Kid3App::s_miscCfg.m_dirFormatItem);
		m_formatComboBox->setCurrentText(Kid3App::s_miscCfg.m_dirFormatText);
#endif
		m_tagversionComboBox->QCM_setCurrentIndex(Kid3App::s_miscCfg.m_renDirSrc);
		formatLabel->setBuddy(m_formatComboBox);
		formatLayout->addWidget(formatLabel);
		formatLayout->addWidget(m_formatComboBox);
		connect(m_formatComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		connect(m_formatComboBox, QCM_SIGNAL_editTextChanged, this, SLOT(slotUpdateNewDirname()));
		vlayout->addLayout(formatLayout);
	}

#if QT_VERSION >= 0x040000
	QGridLayout* fromToLayout = new QGridLayout;
	vlayout->addLayout(fromToLayout);
#else
	QGridLayout* fromToLayout = new QGridLayout(vlayout, 2, 2);
#endif
	QLabel* fromLabel = new QLabel(i18n("From:"), this);
	m_currentDirLabel = new QLabel(this);
	QLabel* toLabel = new QLabel(i18n("To:"), this);
	m_newDirLabel = new QLabel(this);
	if (fromToLayout && fromLabel && m_currentDirLabel &&
		toLabel && m_newDirLabel) {
		fromToLayout->addWidget(fromLabel, 0, 0);
		fromToLayout->addWidget(m_currentDirLabel, 0, 1);
		fromToLayout->addWidget(toLabel, 1, 0);
		fromToLayout->addWidget(m_newDirLabel, 1, 1);
	}

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && saveButton && okButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
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
static QString parentDirectory(const QString& dir)
{
	QString parent(dir);
	int slashPos = parent.QCM_lastIndexOf('/');
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
bool RenDirDialog::createDirectory(const QString& dir,
								   QString* errorMsg) const
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
	const QString& olddir, const QString& newdir, QString* errorMsg) const
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
bool RenDirDialog::renameFile(const QString& oldfn, const QString& newfn,
							  QString* errorMsg) const
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
 * @param olddir pointer to QString to place old directory name into,
 *               NULL if not used
 *
 * @return new directory name.
 */
QString RenDirDialog::generateNewDirname(TaggedFile* taggedFile, QString* olddir)
{
	StandardTags st;
	taggedFile->readTags(false);
	switch (m_tagversionComboBox->QCM_currentIndex()) {
		case TagV1:
			taggedFile->getStandardTagsV1(&st);
			break;
		case TagV2:
			taggedFile->getStandardTagsV2(&st);
			break;
		case TagV2V1:
		default:
		{
			// use merged tags 1 and 2
			StandardTags st1;
			taggedFile->getStandardTagsV1(&st1);
			taggedFile->getStandardTagsV2(&st);
			st.merge(st1);
		}
	}
	QString newdir(taggedFile->getDirname());
	if (newdir.endsWith(QChar('/'))) {
		// remove trailing separator
		newdir.truncate(newdir.length() - 1);
	}
	if (olddir) {
		*olddir = newdir;
	}
	if (!st.isEmptyOrInactive()) {
		if (m_actionComboBox->QCM_currentIndex() == ActionRename) {
			newdir = parentDirectory(newdir);
		} else if (!newdir.isEmpty()) {
			newdir.append('/');
		}
		newdir.append(taggedFile->formatWithTags(&st, m_formatComboBox->currentText(), true));
	}
	return newdir;
}

/**
 * Get new directory name.
 *
 * @return new directory name.
 */
void RenDirDialog::setNewDirname(const QString& dir)
{
	m_newDirLabel->setText(dir);
}

/**
 * Get new directory name.
 *
 * @return new directory name.
 */
QString RenDirDialog::getNewDirname() const
{
	return m_newDirLabel->text();
}

/**
 * Set new directory name according to current settings.
 */
void RenDirDialog::slotUpdateNewDirname()
{
	if (m_taggedFile) {
		QString currentDirname;
		QString newDirname(generateNewDirname(m_taggedFile, &currentDirname));
		m_currentDirLabel->setText(currentDirname);
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
bool RenDirDialog::performAction(TaggedFile* taggedFile, bool& again, QString* errorMsg)
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
				int slashPos = newPart.QCM_indexOf('/', 1);
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
				int slashPos = newPart.QCM_indexOf('/');
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
	Kid3App::s_miscCfg.m_dirFormatItem = m_formatComboBox->QCM_currentIndex();
	Kid3App::s_miscCfg.m_dirFormatText = m_formatComboBox->currentText();
	Kid3App::s_miscCfg.m_renDirSrc = m_tagversionComboBox->QCM_currentIndex();
}

/**
 * Show help.
 */
void RenDirDialog::showHelp()
{
	Kid3App::displayHelp("rename-directory");
}

/**
 * Set directory format string.
 *
 * @param fmt directory format
 */
void RenDirDialog::setDirectoryFormat(const QString& fmt)
{
	m_formatComboBox->setEditText(fmt);
}

/**
 * Set action.
 *
 * @param create true to create, false to rename
 */ 
void RenDirDialog::setAction(bool create)
{
	m_actionComboBox->QCM_setCurrentIndex(create ? ActionCreate : ActionRename);
}

/**
 * Set tag source
 *
 * @param tagMask tag mask (bit 0 for tag 1, bit 1 for tag 2)
 */
void RenDirDialog::setTagSource(int tagMask)
{
	TagVersion index;
	switch (tagMask & 3) {
		case 1:
			index = TagV1;
			break;
		case 2:
			index = TagV2;
			break;
		case 3:
		default:
			index = TagV2V1;
	}
	m_tagversionComboBox->QCM_setCurrentIndex(index);
}
