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
#include <qapplication.h>
#include <qtextedit.h>
#include <qcursor.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#endif

#include "taggedfile.h"
#include "frame.h"
#include "kid3.h"
#include "miscconfig.h"
#include "rendirdialog.h"

/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_1 I18N_NOOP("Create directory %1 failed\n")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_2 I18N_NOOP("File %1 already exists\n")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_3 I18N_NOOP("%1 is not a directory\n")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_4 I18N_NOOP("Rename %1 to %2 failed\n")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_5 I18N_NOOP("%1 already exists\n")
/** Only defined for generation of KDE3 translation files */
#define FOR_KDE3_PO_6 I18N_NOOP("%1 is not a file\n")

/**
 * Constructor.
 *
 * @param parent parent widget
 */
#if QT_VERSION >= 0x040000
#if QT_VERSION >= 0x040300
RenDirDialog::RenDirDialog(QWidget* parent) :
	QWizard(parent), m_taggedFile(0), m_aborted(false)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Rename Directory"));

	QWizardPage* mainPage = new QWizardPage;

	QVBoxLayout* mainLayout = new QVBoxLayout(mainPage);
	setupMainPage(mainPage, mainLayout);
	mainPage->setTitle(i18n("Format"));
	addPage(mainPage);

	QWizardPage* previewPage = new QWizardPage;
	setupPreviewPage(previewPage);
	previewPage->setTitle(i18n("Preview"));
	addPage(previewPage);

	setOptions(HaveHelpButton | HaveCustomButton1);
	setButtonText(CustomButton1, i18n("&Save Settings"));
	connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));
	connect(this, SIGNAL(customButtonClicked(int)), this, SLOT(saveConfig()));
	connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged()));
}
#else
RenDirDialog::RenDirDialog(QWidget* parent) :
	QDialog(parent), m_edit(0), m_taggedFile(0), m_aborted(false)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Rename Directory"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	setupMainPage(this, vlayout);

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (vlayout && hlayout && helpButton && saveButton && okButton && cancelButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(cancelButton);
		okButton->setDefault(true);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()),
						this, SLOT(requestActionSchedulingAndAccept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
}
#endif
#else
RenDirDialog::RenDirDialog(QWidget* parent) :
	QWizard(parent), m_taggedFile(0), m_aborted(false)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Rename Directory"));

	QWidget* mainPage = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainPage);
	setupMainPage(mainPage, mainLayout);
	addPage(mainPage, i18n("Format"));

	QWidget* previewPage = new QWidget(this);
	setupPreviewPage(previewPage);
	addPage(previewPage, i18n("Preview"));
	setFinishEnabled(previewPage, true);

	connect(this, SIGNAL(helpClicked()), this, SLOT(showHelp()));
	connect(this, SIGNAL(selected(const QString&)), this, SLOT(pageChanged()));
}
#endif

/**
 * Destructor.
 */
RenDirDialog::~RenDirDialog()
{}

/**
 * Set up the main wizard page.
 *
 * @param page    widget
 * @param vlayout layout
 */
void RenDirDialog::setupMainPage(QWidget* page, QVBoxLayout* vlayout)
{
	if (!page || !vlayout) {
		return;
	}

	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout* actionLayout = new QHBoxLayout;
	m_actionComboBox = new QComboBox(page);
	m_tagversionComboBox = new QComboBox(page);
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
	QLabel* formatLabel = new QLabel(i18n("&Format:"), page);
	m_formatComboBox = new QComboBox(page);
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
	QLabel* fromLabel = new QLabel(i18n("From:"), page);
	m_currentDirLabel = new QLabel(page);
	QLabel* toLabel = new QLabel(i18n("To:"), page);
	m_newDirLabel = new QLabel(page);
	if (fromToLayout && fromLabel && m_currentDirLabel &&
		toLabel && m_newDirLabel) {
		fromToLayout->addWidget(fromLabel, 0, 0);
		fromToLayout->addWidget(m_currentDirLabel, 0, 1);
		fromToLayout->addWidget(toLabel, 1, 0);
		fromToLayout->addWidget(m_newDirLabel, 1, 1);
	}
}

/**
 * Set up the preview wizard page.
 *
 * @param page widget
 */
void RenDirDialog::setupPreviewPage(QWidget* page)
{
	QVBoxLayout* vlayout = new QVBoxLayout(page);
	if (vlayout) {
		m_edit = new QTextEdit(page);
		if (m_edit) {
			m_edit->setReadOnly(true);
			m_edit->QCM_setTextFormat_PlainText();
			vlayout->addWidget(m_edit);
		}
	}
}

/**
 * Start dialog.
 *
 * @param taggedFile file to use for rename preview
 * @param dirName    if taggedFile is 0, the directory can be set here
 */
void RenDirDialog::startDialog(TaggedFile* taggedFile, const QString& dirName)
{
	m_taggedFile = taggedFile;
	if (m_taggedFile) {
		slotUpdateNewDirname();
	} else {
		m_currentDirLabel->setText(dirName);
		m_newDirLabel->clear();
	}
#if QT_VERSION >= 0x040000
#if QT_VERSION >= 0x040300
	restart();
#endif
#else
	showPage(page(0));
#endif
}

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
			errorMsg->append(KCM_i18n1("Create directory %1 failed\n", dir));
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
			errorMsg->append(KCM_i18n1("File %1 already exists\n", newdir));
		}
		return false;
	}
	if (!QFileInfo(olddir).isDir()) {
		if (errorMsg) {
			errorMsg->append(KCM_i18n1("%1 is not a directory\n", olddir));
		}
		return false;
	}
	if (QDir().rename(olddir, newdir) && QFileInfo(newdir).isDir()) {
		return true;
	} else {
		if (errorMsg) {
			errorMsg->append(KCM_i18n2("Rename %1 to %2 failed\n", olddir, newdir));
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
			errorMsg->append(KCM_i18n1("%1 already exists\n", newfn));
		}
		return false;
	}
	if (!QFileInfo(oldfn).isFile()) {
		if (errorMsg) {
			errorMsg->append(KCM_i18n1("%1 is not a file\n", oldfn));
		}
		return false;
	}
	if (QDir().rename(oldfn, newfn) && QFileInfo(newfn).isFile()) {
		return true;
	} else {
		if (errorMsg) {
			errorMsg->append(KCM_i18n2("Rename %1 to %2 failed\n", oldfn, newfn));
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
	FrameCollection frames;
	taggedFile->readTags(false);
	switch (m_tagversionComboBox->QCM_currentIndex()) {
		case TagV1:
			taggedFile->getAllFramesV1(frames);
			break;
		case TagV2:
			taggedFile->getAllFramesV2(frames);
			break;
		case TagV2V1:
		default:
		{
			// use merged tags 1 and 2
			FrameCollection frames1;
			taggedFile->getAllFramesV1(frames1);
			taggedFile->getAllFramesV2(frames);
			frames.merge(frames1);
		}
	}
	QString newdir(taggedFile->getDirname());
#ifdef WIN32
	newdir.replace('\\', '/');
#endif
	if (newdir.endsWith(QChar('/'))) {
		// remove trailing separator
		newdir.truncate(newdir.length() - 1);
	}
	if (olddir) {
		*olddir = newdir;
	}
	if (!frames.isEmptyOrInactive()) {
		if (m_actionComboBox->QCM_currentIndex() == ActionRename) {
			newdir = parentDirectory(newdir);
		} else if (!newdir.isEmpty()) {
			newdir.append('/');
		}
		newdir.append(taggedFile->formatWithTags(
										frames, m_formatComboBox->currentText(), true));
	}
	return newdir;
}

/**
 * Set new directory name.
 *
 * @param dir new directory name
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
 * Clear the rename actions.
 * This method has to be called before scheduling new actions.
 */
void RenDirDialog::clearActions()
{
	m_actions.clear();
	m_aborted = false;
}
 
 /**
	* Add a rename action.
	*
	* @param type type of action
	* @param src  source file or directory name
	* @param dest destination file or directory name
	*/
void RenDirDialog::addAction(RenameAction::Type type, const QString& src, const QString& dest)
{
	// do not add an action if the source or destination is already in an action
	for (RenameActionList::const_iterator it = m_actions.begin();
			 it != m_actions.end();
			 ++it) {
		if ((!src.isEmpty() && (*it).m_src == src) ||
				(!dest.isEmpty() && (*it).m_dest == dest)){
			return;
		}
	}

	m_actions.push_back(RenameAction(type, src, dest));
}

 /**
	* Add a rename action.
	*
	* @param type type of action
	* @param dest destination file or directory name
	*/
void RenDirDialog::addAction(RenameAction::Type type, const QString& dest)
{
	addAction(type, QString(), dest);
}

/**
 * Check if there is already an action scheduled for this source.
 *
 * @return true if a rename action for the source exists.
 */
bool RenDirDialog::actionHasSource(const QString& src) const
{
	if (src.isEmpty()) {
		return false;
	}
	for (RenameActionList::const_iterator it = m_actions.begin();
			 it != m_actions.end();
			 ++it) {
		if ((*it).m_src == src) {
			return true;
		}
	}
	return false;
}

/**
 * Check if there is already an action scheduled for this destination.
 *
 * @return true if a rename or create action for the destination exists.
 */
bool RenDirDialog::actionHasDestination(const QString& dest) const
{
	if (dest.isEmpty()) {
		return false;
	}
	for (RenameActionList::const_iterator it = m_actions.begin();
			 it != m_actions.end();
			 ++it) {
		if ((*it).m_dest == dest) {
			return true;
		}
	}
	return false;
}

/**
 * Replace directory name if there is already a rename action.
 *
 * @param src directory name, will be replaced if there is a rename action
 */
void RenDirDialog::replaceIfAlreadyRenamed(QString& src) const
{
	bool found = true;
	for (int i = 0; found && i <  5; ++i) {
		found = false;
		for (RenameActionList::const_iterator it = m_actions.begin();
				 it != m_actions.end();
				 ++it) {
			if ((*it).m_type == RenameAction::RenameDirectory &&
					(*it).m_src == src) {
				src = (*it).m_dest;
				found = true;
				break;
			}
		}
	}
}

/**
 * Schedule the actions necessary to rename the directory containing a file.
 *
 * @param taggedFile file in directory
 */
void RenDirDialog::scheduleAction(TaggedFile* taggedFile)
{
	QString currentDirname;
	QString newDirname(generateNewDirname(taggedFile, &currentDirname));
	bool again = false;
	for (int round = 0; round < 2; ++round) {
		replaceIfAlreadyRenamed(currentDirname);
		if (newDirname != currentDirname) {
			if (newDirname.startsWith(currentDirname + '/')) {
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
					addAction(RenameAction::CreateDirectory, currentDirname + newPart);
					if (!createDir) {
						addAction(RenameAction::RenameFile,
											dirWithFiles + '/' + taggedFile->getFilename(),
											currentDirname + newPart + '/' + taggedFile->getFilename());
					}
					currentDirname = currentDirname + newPart;
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
					QString parentWithNewPart = parent + newPart;
					if ((QFileInfo(parentWithNewPart).isDir() &&
							 !actionHasSource(parentWithNewPart)) ||
							actionHasDestination(parentWithNewPart)) {
						// directory already exists => move files
						addAction(RenameAction::RenameFile,
											currentDirname + '/' + taggedFile->getFilename(),
											parentWithNewPart + '/' + taggedFile->getFilename());
						currentDirname = parentWithNewPart;
					} else {
						addAction(RenameAction::RenameDirectory, currentDirname, parentWithNewPart);
						currentDirname = parentWithNewPart;
					}
				} else {
					// new directory name is too different
					addAction(RenameAction::ReportError, i18n("New directory name is too different\n"));
				}
			}
		}
		if (!again) break;
	}
}

/**
 * Perform the scheduled rename actions.
 *
 * @param errorMsg if not 0 and an error occurred, a message is appended here,
 *                 otherwise it is not touched
 */
void RenDirDialog::performActions(QString* errorMsg)
{
	for (RenameActionList::const_iterator it = m_actions.begin();
			 it != m_actions.end();
			 ++it) {
		switch ((*it).m_type) {
			case RenameAction::CreateDirectory:
				createDirectory((*it).m_dest, errorMsg);
				break;
			case RenameAction::RenameDirectory:
				renameDirectory((*it).m_src, (*it).m_dest, errorMsg);
				break;
			case RenameAction::RenameFile:
				renameFile((*it).m_src, (*it).m_dest, errorMsg);
				break;
			case RenameAction::ReportError:
			default:
				if (errorMsg) {
					*errorMsg += (*it).m_dest;
				}
		}
	}
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

/**
 * Request action scheduling and then accept dialog.
 */
void RenDirDialog::requestActionSchedulingAndAccept()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	emit actionSchedulingRequested();
	QApplication::restoreOverrideCursor();
	accept();
}

/**
 * Clear action preview.
 */
void RenDirDialog::clearPreview()
{
	if (m_edit) {
		m_edit->clear();
	}
}

/**
 * Display action preview.
 */
void RenDirDialog::displayPreview()
{
	static const char* const typeStr[] = {
		I18N_NOOP("Create directory"),
		I18N_NOOP("Rename directory"),
		I18N_NOOP("Rename file"),
		I18N_NOOP("Error")
	};
	static const unsigned numTypeStr = sizeof(typeStr) / sizeof(typeStr[0]);
	static int typeWidth = -1;
	if (typeWidth == -1) {
		QFontMetrics metrics = fontMetrics();
		for (unsigned i = 0; i < numTypeStr; ++i) {
			int width = metrics.width(QCM_translate(typeStr[i]));
			if (typeWidth < width) {
				typeWidth = width;
			}
		}
		m_edit->setTabStopWidth(typeWidth + 8);
#if QT_VERSION >= 0x040000
		m_edit->setLineWrapMode(QTextEdit::NoWrap);
#else
		m_edit->setWordWrap(QTextEdit::NoWrap);
#endif
	}
	if (m_edit) {
		m_edit->clear();

		for (RenameActionList::const_iterator it = m_actions.begin();
				 it != m_actions.end();
				 ++it) {
			unsigned typeIdx = static_cast<unsigned>((*it).m_type);
			if (typeIdx >= numTypeStr) {
				typeIdx = numTypeStr - 1;
			}
			QString str = QCM_translate(typeStr[typeIdx]);
			if (!(*it).m_src.isEmpty()) {
				str += '\t';
				str += (*it).m_src;
				str += '\n';
			}
			str += '\t';
			str += (*it).m_dest;
			m_edit->append(str);
		}
	}
}

/**
 * Wizard page changed.
 */
void RenDirDialog::pageChanged()
{
	if (
#if QT_VERSION >= 0x040000
#if QT_VERSION >= 0x040300
		currentId()
#else
		-1
#endif
#else
		indexOf(currentPage())
#endif
		== 1) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		clearPreview();
		emit actionSchedulingRequested();
		displayPreview();
		QApplication::restoreOverrideCursor();
	}
}

/**
 * Called when the wizard is canceled.
 */
void RenDirDialog::reject()
{
	m_aborted = true;
	RenDirDialogBaseClass::reject();
}

#if QT_VERSION < 0x040000
void RenDirDialog::accept()
{
	// The Qt 3 wizard offers no possiblity to have a "Save Settings" button,
	// so the configuration is saved when "Finish" is clicked.
	saveConfig();
	QWizard::accept();
}
#endif
