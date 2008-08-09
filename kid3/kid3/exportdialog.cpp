/**
 * \file exportdialog.cpp
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 *
 * Copyright (C) 2006-2008  Urs Fleisch
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

#include "exportdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include "taggedfile.h"
#include "genres.h"
#include "kid3.h"
#include "importselector.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qurl.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QGroupBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#else
#include <qgroupbox.h>
#endif

/**
 * Constructor.
 *
 * @param parent        parent widget
 */
ExportDialog::ExportDialog(QWidget* parent) :
	QDialog(parent)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Export"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setMargin(6);
		vlayout->setSpacing(6);
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->QCM_setTextFormat_PlainText();
			vlayout->addWidget(m_edit);
		}

#if QT_VERSION >= 0x040000
		QGroupBox* fmtbox = new QGroupBox(i18n("&Format"), this);
#else
		QGroupBox* fmtbox = new QGroupBox(4, Qt::Vertical, i18n("&Format"), this);
#endif
		if (fmtbox) {
			m_formatComboBox = new QComboBox(fmtbox);
			m_formatComboBox->setEditable(true);
			m_headerLineEdit = new QLineEdit(fmtbox);
			m_trackLineEdit = new QLineEdit(fmtbox);
			m_trailerLineEdit = new QLineEdit(fmtbox);
			QString formatToolTip = ImportTrackData::getFormatToolTip();
			QCM_setToolTip(m_headerLineEdit, formatToolTip);
			QCM_setToolTip(m_trackLineEdit, formatToolTip);
			QCM_setToolTip(m_trailerLineEdit, formatToolTip);
#if QT_VERSION >= 0x040000
			QVBoxLayout* vbox = new QVBoxLayout;
			vbox->setMargin(2);
			vbox->addWidget(m_formatComboBox);
			vbox->addWidget(m_headerLineEdit);
			vbox->addWidget(m_trackLineEdit);
			vbox->addWidget(m_trailerLineEdit);
			fmtbox->setLayout(vbox);
#endif
			vlayout->addWidget(fmtbox);
			connect(m_formatComboBox, SIGNAL(activated(int)), this,
							SLOT(setFormatLineEdit(int)));
			connect(m_headerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
			connect(m_trackLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
			connect(m_trailerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
		}

		QHBoxLayout* butlayout = new QHBoxLayout;
		if (butlayout) {
			butlayout->setSpacing(6);
			m_fileButton = new QPushButton(i18n("To F&ile"), this);
			if (m_fileButton) {
				m_fileButton->setAutoDefault(false);
				butlayout->addWidget(m_fileButton);
				connect(m_fileButton, SIGNAL(clicked()), this, SLOT(slotToFile()));
			}
			m_clipButton = new QPushButton(i18n("To Clip&board"), this);
			if (m_clipButton) {
				m_clipButton->setAutoDefault(false);
				butlayout->addWidget(m_clipButton);
				connect(m_clipButton, SIGNAL(clicked()), this, SLOT(slotToClipboard()));
			}
			QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																							 QSizePolicy::Minimum);
			butlayout->addItem(butspacer);

			QLabel* srcLabel = new QLabel(i18n("&Source:"), this);
			butlayout->addWidget(srcLabel);
			m_srcComboBox = new QComboBox(this);
			if (m_srcComboBox) {
				m_srcComboBox->setEditable(false);
				m_srcComboBox->QCM_insertItem(SrcV1, i18n("Tag 1"));
				m_srcComboBox->QCM_insertItem(SrcV2, i18n("Tag 2"));
				srcLabel->setBuddy(m_srcComboBox);
				butlayout->addWidget(m_srcComboBox);
				connect(m_srcComboBox, SIGNAL(activated(int)),
								this, SIGNAL(exportDataRequested(int)));
			}
			vlayout->addLayout(butlayout);
		}

		QHBoxLayout* hlayout = new QHBoxLayout;
		if (hlayout) {
			hlayout->setSpacing(6);
			QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
			if (helpButton) {
				helpButton->setAutoDefault(false);
				hlayout->addWidget(helpButton);
				connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
			}
			QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
			if (saveButton) {
				saveButton->setAutoDefault(false);
				hlayout->addWidget(saveButton);
				connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
			}
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
			if (closeButton) {
				closeButton->setAutoDefault(false);
				hlayout->addWidget(closeButton);
				connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
			}
			vlayout->addLayout(hlayout);
		}
	}
}

/**
 * Destructor.
 */
ExportDialog::~ExportDialog()
{}

/**
 * Export to a file.
 *
 * @param fn file name
 *
 * @return true if ok.
 */
bool ExportDialog::exportToFile(const QString& fn)
{
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_WriteOnly)) {
			ImportSelector::setImportDir(
#if QT_VERSION >= 0x040000
				QFileInfo(file).dir().path()
#else
				QFileInfo(file).dirPath(true)
#endif
				);
			QTextStream stream(&file);
#if QT_VERSION < 0x040000
			stream.setEncoding(QTextStream::Locale);
#endif
			stream << m_edit->QCM_toPlainText();
			file.close();
			return true;
		}
	}
	return false;
}

/**
 * Export to a file.
 */
void ExportDialog::slotToFile()
{
	QString fileName =
#ifdef CONFIG_USE_KDE
		KFileDialog::getSaveFileName(ImportSelector::getImportDir(),
																 QString::null, this);
#else
		QFileDialog::QCM_getSaveFileName(this, ImportSelector::getImportDir());
#endif
	if (!fileName.isEmpty()) {
		if (!exportToFile(fileName)) {
			QMessageBox::warning(
				0, i18n("File Error"),
				i18n("Error while writing file:\n") + fileName,
				QMessageBox::Ok, QCM_NoButton);
		}
	}
}

/**
 * Export to clipboard.
 */
void ExportDialog::slotToClipboard()
{
	QApplication::clipboard()->setText(
		m_edit->QCM_toPlainText()
#if QT_VERSION >= 0x030100
		, QClipboard::Clipboard
#endif
		);
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ExportDialog::setFormatLineEdit(int index)
{
	if (index < static_cast<int>(m_formatHeaders.size())) {
		m_headerLineEdit->setText(m_formatHeaders[index]);
		m_trackLineEdit->setText(m_formatTracks[index]);
		m_trailerLineEdit->setText(m_formatTrailers[index]);
	} else {
		m_headerLineEdit->clear();
		m_trackLineEdit->clear();
		m_trailerLineEdit->clear();
	}
	showPreview();
}

/**
 * Show exported text as preview in editor.
 */
void ExportDialog::showPreview()
{
	m_edit->clear();
	unsigned numTracks = m_trackDataVector.size();
	unsigned trackNr = 0;
	QString headerFormat(m_headerLineEdit->text());
	QString trackFormat(m_trackLineEdit->text());
	QString trailerFormat(m_trailerLineEdit->text());
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		if (trackNr == 0 && !headerFormat.isEmpty()) {
			m_edit->QCM_insertPlainText((*it).formatString(headerFormat, numTracks));
			m_edit->QCM_insertPlainText("\n");
		}
		if (!trackFormat.isEmpty()) {
			m_edit->QCM_insertPlainText((*it).formatString(trackFormat, numTracks));
			m_edit->QCM_insertPlainText("\n");
		}
		if (trackNr == numTracks - 1 && !trailerFormat.isEmpty()) {
			m_edit->QCM_insertPlainText((*it).formatString(trailerFormat, numTracks));
			m_edit->QCM_insertPlainText("\n");
		}
		++trackNr;
	}
}

/**
 * Set data to be exported.
 *
 * @param trackDataVector data to export
 */
void ExportDialog::setExportData(const ImportTrackDataVector& trackDataVector)
{
	m_trackDataVector = trackDataVector;
	showPreview();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void ExportDialog::setFormatFromConfig()
{
	m_formatHeaders = Kid3App::s_genCfg.m_exportFormatHeaders;
	m_formatTracks = Kid3App::s_genCfg.m_exportFormatTracks;
	m_formatTrailers = Kid3App::s_genCfg.m_exportFormatTrailers;
	m_formatComboBox->clear();
	m_formatComboBox->QCM_addItems(Kid3App::s_genCfg.m_exportFormatNames);
	m_formatComboBox->QCM_setCurrentIndex(Kid3App::s_genCfg.m_exportFormatIdx);
	setFormatLineEdit(Kid3App::s_genCfg.m_exportFormatIdx);
}

/**
 * Read the local settings from the configuration.
 */
void ExportDialog::readConfig()
{
	m_srcComboBox->QCM_setCurrentIndex(Kid3App::s_genCfg.m_exportSrcV1 ? SrcV1 : SrcV2);

	setFormatFromConfig();

	if (Kid3App::s_genCfg.m_exportWindowWidth > 0 &&
			Kid3App::s_genCfg.m_exportWindowHeight > 0) {
		resize(Kid3App::s_genCfg.m_exportWindowWidth,
					 Kid3App::s_genCfg.m_exportWindowHeight);
	}
}

/**
 * Save the local settings to the configuration.
 */
void ExportDialog::saveConfig()
{
	Kid3App::s_genCfg.m_exportSrcV1 = (m_srcComboBox->QCM_currentIndex() == SrcV1);
	Kid3App::s_genCfg.m_exportFormatIdx = m_formatComboBox->QCM_currentIndex();
	if (Kid3App::s_genCfg.m_exportFormatIdx < static_cast<int>(Kid3App::s_genCfg.m_exportFormatNames.size())) {
		Kid3App::s_genCfg.m_exportFormatNames[Kid3App::s_genCfg.m_exportFormatIdx] = m_formatComboBox->currentText();
		Kid3App::s_genCfg.m_exportFormatHeaders[Kid3App::s_genCfg.m_exportFormatIdx] = m_headerLineEdit->text();
		Kid3App::s_genCfg.m_exportFormatTracks[Kid3App::s_genCfg.m_exportFormatIdx] = m_trackLineEdit->text();
		Kid3App::s_genCfg.m_exportFormatTrailers[Kid3App::s_genCfg.m_exportFormatIdx] = m_trailerLineEdit->text();
	} else {
		Kid3App::s_genCfg.m_exportFormatIdx = Kid3App::s_genCfg.m_exportFormatNames.size();
		Kid3App::s_genCfg.m_exportFormatNames.append(m_formatComboBox->currentText());
		Kid3App::s_genCfg.m_exportFormatHeaders.append(m_headerLineEdit->text());
		Kid3App::s_genCfg.m_exportFormatTracks.append(m_trackLineEdit->text());
		Kid3App::s_genCfg.m_exportFormatTrailers.append(m_trailerLineEdit->text());
	}
	Kid3App::s_genCfg.m_exportWindowWidth = size().width();
	Kid3App::s_genCfg.m_exportWindowHeight = size().height();

	setFormatFromConfig();
}

/**
 * Show help.
 */
void ExportDialog::showHelp()
{
	Kid3App::displayHelp("export");
}
