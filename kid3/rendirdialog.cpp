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

#include "rendirdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#include <QApplication>
#include <QTextEdit>
#include <QCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "taggedfile.h"
#include "frame.h"
#include "trackdata.h"
#include "configstore.h"
#include "contexthelp.h"
#include "miscconfig.h"
#include "dirrenamer.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param dirRenamer directory renamer
 */
RenDirDialog::RenDirDialog(QWidget* parent, DirRenamer* dirRenamer) :
	QWizard(parent), m_taggedFile(0), m_dirRenamer(dirRenamer)
{
	setModal(true);
	setWindowTitle(i18n("Rename Directory"));

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
		m_actionComboBox->insertItem(ActionRename, i18n("Rename Directory"));
		m_actionComboBox->insertItem(ActionCreate, i18n("Create Directory"));
		actionLayout->addWidget(m_actionComboBox);
		connect(m_actionComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		m_tagversionComboBox->addItem(i18n("From Tag 2 and Tag 1"), TrackData::TagV2V1);
		m_tagversionComboBox->addItem(i18n("From Tag 1"), TrackData::TagV1);
		m_tagversionComboBox->addItem(i18n("From Tag 2"), TrackData::TagV2);
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
		m_formatComboBox->addItems(strList);
		m_formatComboBox->setEditable(true);
		m_formatComboBox->setItemText(ConfigStore::s_miscCfg.m_dirFormatItem,
																	ConfigStore::s_miscCfg.m_dirFormatText);
		m_formatComboBox->setCurrentIndex(ConfigStore::s_miscCfg.m_dirFormatItem);
		m_tagversionComboBox->setCurrentIndex(
					m_tagversionComboBox->findData(ConfigStore::s_miscCfg.m_renDirSrc));
		formatLabel->setBuddy(m_formatComboBox);
		formatLayout->addWidget(formatLabel);
		formatLayout->addWidget(m_formatComboBox);
		connect(m_formatComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateNewDirname()));
		connect(m_formatComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(slotUpdateNewDirname()));
		vlayout->addLayout(formatLayout);
	}

	QGridLayout* fromToLayout = new QGridLayout;
	vlayout->addLayout(fromToLayout);
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
			m_edit->setAcceptRichText(false);
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
	restart();
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
 * Set configuration from dialog in directory renamer.
 */
void RenDirDialog::setDirRenamerConfiguration() {
	m_dirRenamer->setTagVersion(TrackData::tagVersionCast(m_tagversionComboBox->itemData(m_tagversionComboBox->currentIndex()).toInt()));
	m_dirRenamer->setAction(m_actionComboBox->currentIndex() == ActionCreate);
	m_dirRenamer->setFormat(m_formatComboBox->currentText());
}

/**
 * Set new directory name according to current settings.
 */
void RenDirDialog::slotUpdateNewDirname()
{
	if (m_taggedFile) {
		setDirRenamerConfiguration();
		QString currentDirname;
		QString newDirname(m_dirRenamer->generateNewDirname(m_taggedFile, &currentDirname));
		m_currentDirLabel->setText(currentDirname);
		setNewDirname(newDirname);
	}
}

/**
 * Save the local settings to the configuration.
 */
void RenDirDialog::saveConfig()
{
	ConfigStore::s_miscCfg.m_dirFormatItem = m_formatComboBox->currentIndex();
	ConfigStore::s_miscCfg.m_dirFormatText = m_formatComboBox->currentText();
	ConfigStore::s_miscCfg.m_renDirSrc = TrackData::tagVersionCast(
		m_tagversionComboBox->itemData(m_tagversionComboBox->currentIndex()).toInt());
}

/**
 * Show help.
 */
void RenDirDialog::showHelp()
{
	ContextHelp::displayHelp("rename-directory");
}

/**
 * Request action scheduling and then accept dialog.
 */
void RenDirDialog::requestActionSchedulingAndAccept()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	setDirRenamerConfiguration();
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
	m_edit->clear();
	m_edit->setLineWrapMode(QTextEdit::NoWrap);
	QFontMetrics metrics = fontMetrics();
	int typeWidth = -1;
	foreach (const QStringList& actionStrs, m_dirRenamer->describeActions()) {
		QString str = actionStrs.at(0);
		int width = metrics.width(str);
		if (typeWidth < width) {
			typeWidth = width;
		}
		if (actionStrs.size() > 1) {
			str += '\t';
			str += actionStrs.at(1);
		}
		if (actionStrs.size() > 2) {
			str += "\n\t";
			str += actionStrs.at(2);
		}
		m_edit->append(str);
	}
	if (typeWidth > 0) {
		m_edit->setTabStopWidth(typeWidth + 8);
	}
}

/**
 * Wizard page changed.
 */
void RenDirDialog::pageChanged()
{
	if (currentId() == 1) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		clearPreview();
		setDirRenamerConfiguration();
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
	m_dirRenamer->setAbortFlag();
	QWizard::reject();
}
