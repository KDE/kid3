/**
 * \file importsourcedialog.cpp
 * Generic dialog to import from an external source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qregexp.h>
#if QT_VERSION >= 0x040000
#include <QVBoxLayout>
#include <QHBoxLayout>
#endif
#include "importsourceconfig.h"
#include "importsourceclient.h"
#include "importsourcedialog.h"
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param trackDataVector track data to be filled with imported values
 * @param client  client to use, this object takes ownership of it
 * @param props   constant dialog properties, must exist while dialog exists
 */
ImportSourceDialog::ImportSourceDialog(QWidget* parent, QString caption,
																			 ImportTrackDataVector& trackDataVector,
																			 ImportSourceClient* client,
																			 const Properties& props)
	: QDialog(parent), m_trackDataVector(trackDataVector),
		m_serverComboBox(0), m_cgiLineEdit(0), m_additionalTagsCheckBox(0),
		m_coverArtCheckBox(0), m_client(client), m_props(props)
{
	setModal(true);
	QCM_setWindowTitle(caption);

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout* findLayout = new QHBoxLayout;
	m_artistLineEdit = new QComboBox(this);
	m_albumLineEdit = new QComboBox(this);
	m_findButton = new QPushButton(i18n("&Find"), this);
	if (findLayout && m_artistLineEdit && m_albumLineEdit && m_findButton) {
		m_artistLineEdit->setEditable(true);
		m_artistLineEdit->setAutoCompletion(true);
		m_artistLineEdit->setDuplicatesEnabled(false);
		m_albumLineEdit->setEditable(true);
		m_albumLineEdit->setAutoCompletion(true);
		m_albumLineEdit->setDuplicatesEnabled(false);
		m_artistLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		m_albumLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		m_findButton->setDefault(true);
		findLayout->addWidget(m_artistLineEdit);
		findLayout->addWidget(m_albumLineEdit);
		findLayout->addWidget(m_findButton);
		connect(m_findButton, SIGNAL(clicked()), this, SLOT(slotFind()));
		vlayout->addLayout(findLayout);
	}
	if (m_props.defaultServer) {
		QHBoxLayout* serverLayout = new QHBoxLayout;
		QLabel* serverLabel = new QLabel(i18n("&Server:"), this);
		m_serverComboBox = new QComboBox(this);
		QLabel* cgiLabel = 0;
		if (m_props.defaultCgiPath) {
			cgiLabel = new QLabel(i18n("C&GI Path:"), this);
			m_cgiLineEdit = new QLineEdit(this);
		}
		if (serverLayout && serverLabel && m_serverComboBox) {
			if (m_props.serverList) {
				QStringList strList;
				for (const char** sl = m_props.serverList; *sl != 0; ++sl) {
					strList += *sl;
				}
				m_serverComboBox->QCM_addItems(strList);
			}
			m_serverComboBox->setEditable(true);
			serverLayout->addWidget(serverLabel);
			serverLayout->addWidget(m_serverComboBox);
			serverLabel->setBuddy(m_serverComboBox);
			if (cgiLabel && m_cgiLineEdit) {
				serverLayout->addWidget(cgiLabel);
				serverLayout->addWidget(m_cgiLineEdit);
				cgiLabel->setBuddy(m_cgiLineEdit);
			}
			vlayout->addLayout(serverLayout);
		}
	}
	if (m_props.additionalTags) {
		QHBoxLayout* hlayout = new QHBoxLayout;
		m_additionalTagsCheckBox = new QCheckBox(i18n("&Additional Tags"), this);
		m_coverArtCheckBox = new QCheckBox(i18n("C&over Art"), this);
		if (hlayout && m_additionalTagsCheckBox && m_coverArtCheckBox) {
			hlayout->addWidget(m_additionalTagsCheckBox);
			hlayout->addWidget(m_coverArtCheckBox);
			vlayout->addLayout(hlayout);
		}
	}
#if QT_VERSION >= 0x040000
	m_albumListBox = new QListWidget(this);
#else
	m_albumListBox = new QListBox(this);
#endif
	if (m_albumListBox) {
		vlayout->addWidget(m_albumListBox);
#if QT_VERSION >= 0x040000
		connect(m_albumListBox, SIGNAL(itemClicked(QListWidgetItem*)),
				this, SLOT(requestTrackList(QListWidgetItem*)));
		connect(m_albumListBox, SIGNAL(itemActivated(QListWidgetItem*)),
				this, SLOT(requestTrackList(QListWidgetItem*)));
#else
		connect(m_albumListBox, SIGNAL(selectionChanged(QListBoxItem*)),
				this, SLOT(requestTrackList(QListBoxItem*)));
		connect(m_albumListBox, SIGNAL(selected(int)),
				this, SLOT(requestTrackList(int)));
#endif
	}

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	QPushButton* helpButton = m_props.helpAnchor ?
		new QPushButton(i18n("&Help"), this) : 0;
	QPushButton* saveButton = m_props.cfg ? new QPushButton(i18n("&Save Settings"), this) : 0;
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	if (buttonLayout && closeButton) {
		if (helpButton) {
			buttonLayout->addWidget(helpButton);
			connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		}
		if (saveButton) {
			buttonLayout->addWidget(saveButton);
			connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		}
		QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																					 QSizePolicy::Minimum);
		buttonLayout->addItem(hspacer);
		buttonLayout->addWidget(closeButton);
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
		vlayout->addLayout(buttonLayout);
	}

	m_statusBar = new QStatusBar(this);
	if (m_statusBar) {
		vlayout->addWidget(m_statusBar);
		showStatusMessage(i18n("Ready."));
		connect(m_client, SIGNAL(progress(const QString&, int, int)),
						this, SLOT(showStatusMessage(const QString&)));
		connect(m_client, SIGNAL(findFinished(const QByteArray&)),
				this, SLOT(slotFindFinished(const QByteArray&)));
		connect(m_client, SIGNAL(albumFinished(const QByteArray&)),
				this, SLOT(slotAlbumFinished(const QByteArray&)));
	}
}

/**
 * Destructor.
 */
ImportSourceDialog::~ImportSourceDialog()
{
	m_client->disconnect();
	delete m_client;
}

/**
 * Display message in status bar.
 *
 * @param msg status message
 */
void ImportSourceDialog::showStatusMessage(const QString& msg)
{
	m_statusBar->QCM_showMessage(msg);
}

/**
 * Clear dialog data.
 */
void ImportSourceDialog::clear()
{
	m_albumListBox->clear();
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString ImportSourceDialog::getServer() const
{
	if (m_serverComboBox) {
		QString server(m_serverComboBox->currentText());
		if (server.isEmpty()) {
			server = m_props.defaultServer;
		}
		return server;
	} else {
		return QString::null;
	}
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void ImportSourceDialog::setServer(const QString& srv)
{
	if (m_serverComboBox) {
#if QT_VERSION >= 0x040000
		int idx = m_serverComboBox->findText(srv);
		if (idx >= 0) {
			m_serverComboBox->setCurrentIndex(idx);
		} else {
			m_serverComboBox->addItem(srv);
			m_serverComboBox->setCurrentIndex(m_serverComboBox->count() - 1);
		}
#else
		m_serverComboBox->setCurrentText(srv);
#endif
	}
}

/**
 * Get string with CGI path.
 *
 * @return CGI path, e.g. "/~cddb/cddb.cgi".
 */
QString ImportSourceDialog::getCgiPath() const
{
	if (m_cgiLineEdit) {
		QString cgi(m_cgiLineEdit->text());
		if (cgi.isEmpty()) {
			cgi = m_props.defaultCgiPath;
		}
		return cgi;
	} else {
		return QString::null;
	}
}

/**
 * Set string with CGI path.
 *
 * @param cgi CGI path, e.g. "/~cddb/cddb.cgi".
 */
void ImportSourceDialog::setCgiPath(const QString& cgi)
{
	if (m_cgiLineEdit) {
		m_cgiLineEdit->setText(cgi);
	}
}

/**
 * Get additional tags option.
 *
 * @return true if additional tags are enabled.
 */
bool ImportSourceDialog::getAdditionalTags() const
{
	return m_additionalTagsCheckBox ?
#if QT_VERSION >= 0x040000
		m_additionalTagsCheckBox->checkState() == Qt::Checked
#else
		m_additionalTagsCheckBox->isChecked()
#endif
		: false;
}

/**
 * Set additional tags option.
 *
 * @param enable true if additional tags are enabled
 */
void ImportSourceDialog::setAdditionalTags(bool enable)
{
	if (m_additionalTagsCheckBox) {
#if QT_VERSION >= 0x040000
		m_additionalTagsCheckBox->setCheckState(
			enable ? Qt::Checked : Qt::Unchecked);
#else
		m_additionalTagsCheckBox->setChecked(enable);
#endif
	}
}

/**
 * Get cover art option.
 *
 * @return true if cover art are enabled.
 */
bool ImportSourceDialog::getCoverArt() const
{
	return m_coverArtCheckBox ?
#if QT_VERSION >= 0x040000
		m_coverArtCheckBox->checkState() == Qt::Checked
#else
		m_coverArtCheckBox->isChecked()
#endif
		: false;
}

/**
 * Set cover art option.
 *
 * @param enable true if cover art are enabled
 */
void ImportSourceDialog::setCoverArt(bool enable)
{
	if (m_coverArtCheckBox) {
#if QT_VERSION >= 0x040000
		m_coverArtCheckBox->setCheckState(
			enable ? Qt::Checked : Qt::Unchecked);
#else
		m_coverArtCheckBox->setChecked(enable);
#endif
	}
}

/**
 * Get the local configuration.
 *
 * @param cfg configuration
 */
void ImportSourceDialog::getImportSourceConfig(ImportSourceConfig* cfg) const
{
	cfg->m_server = getServer();
	cfg->m_cgiPath = getCgiPath();
	cfg->m_additionalTags = getAdditionalTags();
	cfg->m_coverArt = getCoverArt();
	cfg->m_windowWidth = size().width();
	cfg->m_windowHeight = size().height();
}

/**
 * Save the local settings to the configuration.
 */
void ImportSourceDialog::saveConfig()
{
	if (m_props.cfg) {
		getImportSourceConfig(m_props.cfg);
	}
}

/**
 * Set a find string from artist and album information.
 *
 * @param artist artist
 * @param album  album
 */
void ImportSourceDialog::setArtistAlbum(const QString& artist, const QString& album)
{
	if (m_props.cfg) {
		setServer(m_props.cfg->m_server);
		setCgiPath(m_props.cfg->m_cgiPath);
		setAdditionalTags(m_props.cfg->m_additionalTags);
		setCoverArt(m_props.cfg->m_coverArt);
		if (m_props.cfg->m_windowWidth > 0 && m_props.cfg->m_windowHeight > 0) {
			resize(m_props.cfg->m_windowWidth, m_props.cfg->m_windowHeight);
		}
	}

	if (!(artist.isEmpty() && album.isEmpty())) {
#if QT_VERSION >= 0x040000
		int idx = m_artistLineEdit->findText(artist);
		if (idx >= 0) {
			m_artistLineEdit->setCurrentIndex(idx);
		} else {
			m_artistLineEdit->addItem(artist);
			m_artistLineEdit->setCurrentIndex(m_artistLineEdit->count() - 1);
		}
		idx = m_albumLineEdit->findText(album);
		if (idx >= 0) {
			m_albumLineEdit->setCurrentIndex(idx);
		} else {
			m_albumLineEdit->addItem(album);
			m_albumLineEdit->setCurrentIndex(m_albumLineEdit->count() - 1);
		}
#else
		m_artistLineEdit->setCurrentText(artist);
		m_albumLineEdit->setCurrentText(album);
#endif
		QLineEdit* lineEdit = m_artistLineEdit->lineEdit();
		if (lineEdit) {
			lineEdit->selectAll();
		}
		m_artistLineEdit->setFocus();
	}
}

/**
 * Query a search for a keyword from the server.
 */
void ImportSourceDialog::slotFind()
{
	ImportSourceConfig cfg;
	getImportSourceConfig(&cfg);
	m_client->find(&cfg, m_artistLineEdit->currentText(),
								 m_albumLineEdit->currentText());
}

/**
 * Process finished find request.
 *
 * @param searchStr search data received
 */
void ImportSourceDialog::slotFindFinished(const QByteArray& searchStr)
{
	parseFindResults(searchStr);
}

/**
 * Process finished album data.
 *
 * @param albumStr album track data received
 */
void ImportSourceDialog::slotAlbumFinished(const QByteArray& albumStr)
{
	parseAlbumResults(albumStr);
	emit trackDataUpdated();
}

/**
 * Request track list from server.
 *
 * @param li list box item containing an AlbumListItem
 */
#if QT_VERSION >= 0x040000
void ImportSourceDialog::requestTrackList(QListWidgetItem* li)
#else
void ImportSourceDialog::requestTrackList(QListBoxItem* li)
#endif
{
	AlbumListItem* ali;
	if ((ali = dynamic_cast<AlbumListItem *>(li)) != 0) {
		ImportSourceConfig cfg;
		getImportSourceConfig(&cfg);
		m_client->getTrackList(&cfg, ali->getCategory(), ali->getId());
	}
}

/**
 * Request track list from server.
 *
 * @param index index of list box item containing an AlbumListItem
 */
void ImportSourceDialog::requestTrackList(int index)
{
	requestTrackList(m_albumListBox->item(index));
}

/**
 * Show help.
 */
void ImportSourceDialog::showHelp()
{
	if (m_props.helpAnchor) {
		Kid3App::displayHelp(m_props.helpAnchor);
	}
}
