/**
 * \file textimportdialog.cpp
 * Dialog to import from a text (file or clipboard).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "textimportdialog.h"
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFile>
#include <QClipboard>
#include <QTextStream>
#include <QApplication>
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <QFileDialog>
#endif
#include "textimporter.h"
#include "importparser.h"
#include "kid3.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param trackDataModel track data to be filled with imported values
 */
TextImportDialog::TextImportDialog(QWidget* parent,
																	 TrackDataModel* trackDataModel) :
	QDialog(parent), m_textImporter(new TextImporter(trackDataModel))
{
	setObjectName("TextImportDialog");
	setWindowTitle(i18n("Import from File/Clipboard"));
	setSizeGripEnabled(true);

	QVBoxLayout* vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(6);

	m_formatComboBox = new QComboBox(this);
	m_formatComboBox->setEditable(true);
	m_headerLineEdit = new QLineEdit(this);
	m_trackLineEdit = new QLineEdit(this);
	QString formatToolTip = ImportParser::getFormatToolTip();
	m_headerLineEdit->setToolTip(formatToolTip);
	m_trackLineEdit->setToolTip(formatToolTip);
	connect(m_formatComboBox, SIGNAL(activated(int)), this, SLOT(setFormatLineEdit(int)));
	QFormLayout* formatLayout = new QFormLayout;
	formatLayout->addRow(i18n("Format:"), m_formatComboBox);
	formatLayout->addRow(i18n("Header:"), m_headerLineEdit);
	formatLayout->addRow(i18n("Tracks:"), m_trackLineEdit);
	vboxLayout->addLayout(formatLayout);

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	helpButton->setAutoDefault(false);
	buttonLayout->addWidget(helpButton);
	connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	saveButton->setAutoDefault(false);
	buttonLayout->addWidget(saveButton);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
	buttonLayout->addStretch();
	QPushButton* fileButton = new QPushButton(i18n("From F&ile"), this);
	fileButton->setAutoDefault(false);
	buttonLayout->addWidget(fileButton);
	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
	QPushButton* clipButton = new QPushButton(i18n("From Clip&board"), this);
	clipButton->setAutoDefault(false);
	buttonLayout->addWidget(clipButton);
	connect(clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	closeButton->setAutoDefault(false);
	buttonLayout->addWidget(closeButton);
	connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	vboxLayout->addLayout(buttonLayout);
}

/**
 * Destructor.
 */
TextImportDialog::~TextImportDialog()
{
}

/**
 * Clear dialog data.
 */
void TextImportDialog::clear()
{
	setFormatFromConfig();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void TextImportDialog::setFormatFromConfig()
{
	m_formatHeaders = Kid3App::s_genCfg.m_importFormatHeaders;
	m_formatTracks = Kid3App::s_genCfg.m_importFormatTracks;
	m_formatComboBox->clear();
	m_formatComboBox->addItems(Kid3App::s_genCfg.m_importFormatNames);
	m_formatComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importFormatIdx);
	setFormatLineEdit(Kid3App::s_genCfg.m_importFormatIdx);
}

/**
 * Import from a file.
 *
 * @param fn file name
 *
 * @return true if ok.
 */
bool TextImportDialog::importFromFile(const QString& fn)
{
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QIODevice::ReadOnly)) {
			Kid3App::s_genCfg.m_importDir = QFileInfo(file).dir().path();
			QTextStream stream(&file);
			QString text = stream.readAll();
			if (!text.isNull() &&
					m_textImporter->updateTrackData(
							text, m_headerLineEdit->text(), m_trackLineEdit->text())) {
				emit trackDataUpdated();
			}
			file.close();
			return true;
		}
	}
	return false;
}

/**
 * Let user select file, assign file contents to text and preview in
 * table.
 */
void TextImportDialog::fromFile()
{
	importFromFile(
#ifdef CONFIG_USE_KDE
		KFileDialog::getOpenFileName(Kid3App::s_genCfg.m_importDir, QString::null, this)
#else
		QFileDialog::getOpenFileName(this, QString(), Kid3App::s_genCfg.m_importDir
#if !defined Q_OS_WIN32 && !defined Q_OS_MAC
			, QString(), 0, QFileDialog::DontUseNativeDialog
#endif
			)
#endif
		);
}

/**
 * Assign clipboard contents to text and preview in table.
 */
void TextImportDialog::fromClipboard()
{
	QClipboard* cb = QApplication::clipboard();
	QString text = cb->text(QClipboard::Clipboard);
	if (text.isNull())
		text = cb->text(QClipboard::Selection);
	if (!text.isNull() &&
			m_textImporter->updateTrackData(
				text, m_headerLineEdit->text(), m_trackLineEdit->text()))
		emit trackDataUpdated();
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void TextImportDialog::setFormatLineEdit(int index)
{
	if (index < static_cast<int>(m_formatHeaders.size())) {
		m_headerLineEdit->setText(m_formatHeaders[index]);
		m_trackLineEdit->setText(m_formatTracks[index]);
	} else {
		m_headerLineEdit->clear();
		m_trackLineEdit->clear();
	}
}

/**
 * Save the local settings to the configuration.
 */
void TextImportDialog::saveConfig()
{
	Kid3App::s_genCfg.m_importFormatIdx = m_formatComboBox->currentIndex();
	if (Kid3App::s_genCfg.m_importFormatIdx < static_cast<int>(Kid3App::s_genCfg.m_importFormatNames.size())) {
		Kid3App::s_genCfg.m_importFormatNames[Kid3App::s_genCfg.m_importFormatIdx] = m_formatComboBox->currentText();
		Kid3App::s_genCfg.m_importFormatHeaders[Kid3App::s_genCfg.m_importFormatIdx] = m_headerLineEdit->text();
		Kid3App::s_genCfg.m_importFormatTracks[Kid3App::s_genCfg.m_importFormatIdx] = m_trackLineEdit->text();
	} else {
		Kid3App::s_genCfg.m_importFormatIdx = Kid3App::s_genCfg.m_importFormatNames.size();
		Kid3App::s_genCfg.m_importFormatNames.append(m_formatComboBox->currentText());
		Kid3App::s_genCfg.m_importFormatHeaders.append(m_headerLineEdit->text());
		Kid3App::s_genCfg.m_importFormatTracks.append(m_trackLineEdit->text());
	}

	setFormatFromConfig();
}

/**
 * Show help.
 */
void TextImportDialog::showHelp()
{
	Kid3App::displayHelp("import-text");
}
