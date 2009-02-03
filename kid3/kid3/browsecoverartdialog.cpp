/**
 * \file browsecoverartdialog.cpp
 * Browse cover art dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11-Jan-2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include "browsecoverartdialog.h"
#include "kid3.h"
#include "externalprocess.h"
#include "configtable.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qurl.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#else
#include <qgroupbox.h>
#endif

/**
 * Get help text for supported format codes.
 *
 * @return help text.
 */
static QString getToolTip()
{
	QString str = "<table>\n";
	str += FrameFormatReplacer::getToolTip(true);

	str += "<tr><td>%ua...</td><td>%u{artist}...</td><td>";
	str += QCM_translate(I18N_NOOP("Encode as URL"));
	str += "</td></tr>\n";

	str += "</table>\n";
	return str;
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
BrowseCoverArtDialog::BrowseCoverArtDialog(QWidget* parent) :
	QDialog(parent), m_process(0)
{
	setModal(true);
	QCM_setWindowTitle(i18n("Browse Cover Art"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setMargin(6);
		vlayout->setSpacing(6);

		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setReadOnly(true);
			vlayout->addWidget(m_edit);
		}

#if QT_VERSION >= 0x040000
		QGroupBox* artistAlbumBox = new QGroupBox(i18n("&Artist/Album"), this);
#else
		QGroupBox* artistAlbumBox = new QGroupBox(2, Qt::Horizontal,
		                                          i18n("&Artist/Album"), this);
#endif
		if (artistAlbumBox) {
			m_artistLineEdit = new QLineEdit(artistAlbumBox);
			m_albumLineEdit = new QLineEdit(artistAlbumBox);
#if QT_VERSION >= 0x040000
			QHBoxLayout* hbox = new QHBoxLayout;
			hbox->setMargin(2);
			hbox->addWidget(m_artistLineEdit);
			hbox->addWidget(m_albumLineEdit);
			artistAlbumBox->setLayout(hbox);
#endif
			vlayout->addWidget(artistAlbumBox);
			connect(m_artistLineEdit, SIGNAL(returnPressed()),
							this, SLOT(showPreview()));
			connect(m_albumLineEdit, SIGNAL(returnPressed()),
							this, SLOT(showPreview()));
		}

#if QT_VERSION >= 0x040000
		QGroupBox* srcbox = new QGroupBox(i18n("&Source"), this);
#else
		QGroupBox* srcbox = new QGroupBox(2, Qt::Vertical, i18n("&Source"), this);
#endif
		if (srcbox) {
			m_sourceComboBox = new QComboBox(srcbox);
			m_sourceComboBox->setEditable(true);
			m_urlLineEdit = new QLineEdit(srcbox);
			QCM_setToolTip(m_urlLineEdit, getToolTip());
#if QT_VERSION >= 0x040000
			QVBoxLayout* vbox = new QVBoxLayout;
			vbox->setMargin(2);
			vbox->addWidget(m_sourceComboBox);
			vbox->addWidget(m_urlLineEdit);
			srcbox->setLayout(vbox);
#endif
			vlayout->addWidget(srcbox);
			connect(m_sourceComboBox, SIGNAL(activated(int)), this,
							SLOT(setSourceLineEdit(int)));
			connect(m_urlLineEdit, SIGNAL(returnPressed()),
							this, SLOT(showPreview()));
		}

#if QT_VERSION >= 0x040000
		QGroupBox* tabbox = new QGroupBox(i18n("&URL extraction"), this);
#else
		QGroupBox* tabbox = new QGroupBox(1, Qt::Vertical,
		                                  i18n("&URL extraction"), this);
#endif
		if (tabbox) {
			m_matchUrlTable = new ConfigTable(
				QStringList()
				<< i18n("Match")
				<< i18n("Picture URL"),
				tabbox);
#if QT_VERSION >= 0x040000
			QVBoxLayout* tablayout = new QVBoxLayout;
			tablayout->setMargin(2);
			tablayout->addWidget(m_matchUrlTable);
			tabbox->setLayout(tablayout);
#endif
			vlayout->addWidget(tabbox);
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

			QPushButton* browseButton = new QPushButton(i18n("&Browse"), this);
			QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
			if (browseButton && cancelButton) {
				browseButton->setAutoDefault(false);
				cancelButton->setAutoDefault(false);
				hlayout->addWidget(browseButton);
				hlayout->addWidget(cancelButton);
				connect(browseButton, SIGNAL(clicked()), this, SLOT(accept()));
				connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
			}
			vlayout->addLayout(hlayout);
		}
	}
}

/**
 * Destructor.
 */
BrowseCoverArtDialog::~BrowseCoverArtDialog()
{
	delete m_process;
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void BrowseCoverArtDialog::setSourceLineEdit(int index)
{
	if (index < static_cast<int>(m_urls.size())) {
		m_urlLineEdit->setText(m_urls[index]);
	} else {
		m_urlLineEdit->clear();
	}
	showPreview();
}

/**
 * Show browse command as preview.
 */
void BrowseCoverArtDialog::showPreview()
{
	m_frames.setArtist(m_artistLineEdit->text());
	m_frames.setAlbum(m_albumLineEdit->text());
	FrameFormatReplacer fmt(m_frames, m_urlLineEdit->text());
	fmt.replaceEscapedChars();
	fmt.replacePercentCodes(FormatReplacer::FSF_SupportUrlEncode);
	m_url = fmt.getString();
	QString txt("<p><b>");
	txt += i18n("Click Browse to start");
	txt += "</b></p><p><tt>";
	txt += Kid3App::s_miscCfg.m_browser;
	txt += " ";
	txt += m_url;
	txt += "</tt></p><p><b>";
	txt += i18n("Then drag the picture from the browser to Kid3.");
	txt += "</b></p>";
	m_edit->clear();
	m_edit->append(txt);
}

/**
 * Set frames for which picture has to be found.
 *
 * @param frames track data
 */
void BrowseCoverArtDialog::setFrames(const FrameCollection& frames)
{
	m_frames = frames;

	m_artistLineEdit->setText(m_frames.getArtist());
	m_albumLineEdit->setText(m_frames.getAlbum());

	showPreview();
}

/**
 * Set the source combo box and line edits from the configuration.
 */
void BrowseCoverArtDialog::setSourceFromConfig()
{
	m_urls = Kid3App::s_genCfg.m_pictureSourceUrls;
	m_sourceComboBox->clear();
	m_sourceComboBox->QCM_addItems(Kid3App::s_genCfg.m_pictureSourceNames);
	m_sourceComboBox->QCM_setCurrentIndex(Kid3App::s_genCfg.m_pictureSourceIdx);
	setSourceLineEdit(Kid3App::s_genCfg.m_pictureSourceIdx);
}

/**
 * Read the local settings from the configuration.
 */
void BrowseCoverArtDialog::readConfig()
{
	setSourceFromConfig();
	m_matchUrlTable->fromMap(Kid3App::s_genCfg.m_matchPictureUrlMap);

	if (Kid3App::s_genCfg.m_browseCoverArtWindowWidth > 0 &&
			Kid3App::s_genCfg.m_browseCoverArtWindowHeight > 0) {
		resize(Kid3App::s_genCfg.m_browseCoverArtWindowWidth,
					 Kid3App::s_genCfg.m_browseCoverArtWindowHeight);
	}
}

/**
 * Save the local settings to the configuration.
 */
void BrowseCoverArtDialog::saveConfig()
{
	Kid3App::s_genCfg.m_pictureSourceIdx = m_sourceComboBox->QCM_currentIndex();
	if (Kid3App::s_genCfg.m_pictureSourceIdx <
			static_cast<int>(Kid3App::s_genCfg.m_pictureSourceNames.size())) {
		Kid3App::s_genCfg.m_pictureSourceNames[Kid3App::s_genCfg.m_pictureSourceIdx] =
			m_sourceComboBox->currentText();
		Kid3App::s_genCfg.m_pictureSourceUrls[Kid3App::s_genCfg.m_pictureSourceIdx] =
			m_urlLineEdit->text();
	} else {
		Kid3App::s_genCfg.m_pictureSourceIdx =
			Kid3App::s_genCfg.m_pictureSourceNames.size();
		Kid3App::s_genCfg.m_pictureSourceNames.append(m_sourceComboBox->currentText());
		Kid3App::s_genCfg.m_pictureSourceUrls.append(m_urlLineEdit->text());
	}
	m_matchUrlTable->toMap(Kid3App::s_genCfg.m_matchPictureUrlMap);
	Kid3App::s_genCfg.m_browseCoverArtWindowWidth = size().width();
	Kid3App::s_genCfg.m_browseCoverArtWindowHeight = size().height();

	setSourceFromConfig();
}

/**
 * Show help.
 */
void BrowseCoverArtDialog::showHelp()
{
	Kid3App::displayHelp("browse_pictures");
}

/**
 * Hide modal dialog, start browse command.
 */
void BrowseCoverArtDialog::accept()
{
	if (!m_process) {
		m_process = new ExternalProcess(this);
	}
	if (m_process) {
		m_process->launchCommand(
			i18n("Browse Cover Art"),
			QStringList() << Kid3App::s_miscCfg.m_browser << m_url);
	}
	QDialog::accept();
}

/**
 * Get the URL of an image file.
 * The input URL is transformed using the match picture URL table to
 * get the URL of an image file.
 *
 * @param url URL from image drag
 *
 * @return URL of image file, empty if no image URL found.
 */
QString BrowseCoverArtDialog::getImageUrl(const QString& url)
{
	QString imgurl;
	if (url.startsWith("http://")) {
		if (url.endsWith(".jpg", QCM_CaseInsensitive) ||
				url.endsWith(".jpeg", QCM_CaseInsensitive) ||
				url.endsWith(".png", QCM_CaseInsensitive)) {
			imgurl = url;
		}
		else {
			for (QMap<QString, QString>::ConstIterator it =
						 Kid3App::s_genCfg.m_matchPictureUrlMap.begin();
					 it != Kid3App::s_genCfg.m_matchPictureUrlMap.end();
					 ++it) {
				QRegExp re(it.key());
				if (re.exactMatch(url)) {
					QString pictureUrl(url);
					imgurl = url;
					imgurl.replace(re, *it);
					if (imgurl.QCM_indexOf("%25") != -1) {
						// double URL encoded: first decode
						QCM_QUrl_decode(imgurl);
					}
					if (imgurl.QCM_indexOf("%2F") != -1) {
						// URL encoded: decode
						QCM_QUrl_decode(imgurl);
					}
					break;
				}
			}
		}
	}
	return imgurl;
}
