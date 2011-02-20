/**
 * \file musicbrainzdialog.cpp
 * MusicBrainz import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 *
 * Copyright (C) 2005-2007  Urs Fleisch
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

#include "musicbrainzdialog.h"
#ifdef HAVE_TUNEPIMP

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qfileinfo.h>
#if QT_VERSION >= 0x040000
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#else
#include <qhbox.h>
#include <qtable.h>
#endif
#include "kid3.h"
#include "musicbrainzclient.h"

/**
 * Constructor.
 *
 * @param parent          parent widget
 * @param trackDataVector track data to be filled with imported values,
 *                        is passed with filenames set
 */
MusicBrainzDialog::MusicBrainzDialog(QWidget* parent,
																		 ImportTrackDataVector& trackDataVector)
	: QDialog(parent), m_statusBar(0),
		m_timer(0), m_client(0), m_trackDataVector(trackDataVector)
{
	setModal(true);
	QCM_setWindowTitle(i18n("MusicBrainz"));

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return;
	}
	vlayout->setMargin(6);
	vlayout->setSpacing(6);
	QHBoxLayout* serverLayout = new QHBoxLayout;
	QLabel* serverLabel = new QLabel(i18n("&Server:"), this);
	m_serverComboBox = new QComboBox(this);
	if (serverLayout && serverLabel && m_serverComboBox) {
		m_serverComboBox->setEditable(true);
		static const char* serverList[] = {
			"musicbrainz.org:80",
			"de.musicbrainz.org:80",
			"nl.musicbrainz.org:80",
			0                  // end of StrList
		};
		QStringList strList;
		for (const char** sl = serverList; *sl != 0; ++sl) {
			strList += *sl;
		}
		m_serverComboBox->QCM_addItems(strList);
		m_serverComboBox->setSizePolicy(
			QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		serverLabel->setBuddy(m_serverComboBox);
		serverLayout->addWidget(serverLabel);
		serverLayout->addWidget(m_serverComboBox);
		connect(m_serverComboBox, SIGNAL(activated(int)),
						this, SLOT(setClientConfig()));
		vlayout->addLayout(serverLayout);
	}
#if QT_VERSION >= 0x040000
	m_albumTable = new QTableWidget(this);
	if (m_albumTable) {
		m_albumTable->setColumnCount(2);
		m_albumTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
		m_albumTable->setSelectionMode(QAbstractItemView::NoSelection);
		m_albumTable->setHorizontalHeaderLabels(
			QStringList() <<
			"08 A Not So Short Title/Medium Sized Artist - And The Album Title [2005]" <<
			"A Not So Short State");
		m_albumTable->resizeColumnToContents(0);
		m_albumTable->resizeColumnToContents(1);
		m_albumTable->setHorizontalHeaderLabels(
			QStringList() <<
			i18n("Track Title/Artist - Album") <<
			i18n("State"));
		initTable();
		vlayout->addWidget(m_albumTable);
	}
#else
	m_albumTable = new QTable(this, "albumTable");
	if (m_albumTable) {
		m_albumTable->setNumCols(2);
		m_albumTable->setColumnReadOnly(1, true);
		m_albumTable->setFocusStyle(QTable::FollowStyle);
		m_albumTable->setColumnStretchable(0, true);
		m_albumTable->setSelectionMode(QTable::NoSelection);
		QHeader* hHeader = m_albumTable->horizontalHeader();
		hHeader->setLabel(0, "08 A Not So Short Title/Medium Sized Artist - And The Album Title [2005]");
		hHeader->setLabel(1, "A Not So Short State");
		m_albumTable->adjustColumn(0);
		m_albumTable->adjustColumn(1);
		hHeader->setLabel(0, i18n("Track Title/Artist - Album"));
		hHeader->setLabel(1, i18n("State"));
		initTable();
		vlayout->addWidget(m_albumTable);
	}
#endif

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* applyButton = new QPushButton(i18n("&Apply"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && saveButton &&
			okButton && cancelButton && applyButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(applyButton);
		hlayout->addWidget(cancelButton);
		// auto default is switched off to use the return key to set the server
		// configuration
		okButton->setAutoDefault(false);
		cancelButton->setAutoDefault(false);
		applyButton->setAutoDefault(false);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
		vlayout->addLayout(hlayout);
	}

	m_statusBar = new QStatusBar(this);
	if (m_statusBar) {
		vlayout->addWidget(m_statusBar);
		if (m_albumTable) {
#if QT_VERSION >= 0x040000
			connect(m_albumTable, SIGNAL(currentCellChanged(int, int, int, int)),
							this, SLOT(showFilenameInStatusBar(int)));
#else
			connect(m_albumTable, SIGNAL(currentChanged(int, int)),
							this, SLOT(showFilenameInStatusBar(int)));
#endif
		}
	}
}

/**
 * Destructor.
 */
MusicBrainzDialog::~MusicBrainzDialog()
{
	stopClient();
}

/**
 * Initialize the table.
 * Has to be called before reusing the dialog with new track data.
 */
void MusicBrainzDialog::initTable()
{
	setServer(Kid3App::s_musicBrainzCfg.m_server);

	unsigned numRows = m_trackDataVector.size();
	m_trackResults.resize(numRows);
#if QT_VERSION >= 0x040000
	m_albumTable->setRowCount(numRows);
	for (unsigned i = 0; i < numRows; ++i) {
		QTableWidgetItem* twi;
		QComboBox* combo;
		if ((twi = m_albumTable->item(i, 0)) == 0) {
			twi = new QTableWidgetItem;
			twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
			m_albumTable->setItem(i, 0, twi);
			combo = new QComboBox;
			m_albumTable->setCellWidget(i, 0, combo);
		} else {
			combo = dynamic_cast<QComboBox*>(m_albumTable->cellWidget(i, 0));
		}
		if (combo) {
			combo->clear();
			combo->addItem(i18n("No result"));
			combo->addItem(i18n("Unknown"));
		}

		if ((twi = m_albumTable->item(i, 1)) != 0) {
			twi->setText(i18n("Unknown"));
		} else {
			twi = new QTableWidgetItem(i18n("Unknown"));
			twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
			m_albumTable->setItem(i, 1, twi);
		}
	}
#else
	m_albumTable->setNumRows(numRows);
	for (unsigned i = 0; i < numRows; ++i) {
		QComboTableItem* cti = new QComboTableItem(
			m_albumTable, QStringList(i18n("No result")));
		m_albumTable->setItem(i, 0, cti);
		m_albumTable->setText(i, 1, i18n("Unknown"));
	}
#endif
	showFilenameInStatusBar(m_albumTable->currentRow());
}

/**
 * Clear all results.
 */
void MusicBrainzDialog::clearResults()
{
	unsigned numRows = m_trackDataVector.size();
	for (unsigned i = 0; i < numRows; ++i) {
		m_trackResults[i].clear();
		setFileStatus(i, i18n("Unknown"));
		updateFileTrackData(i);
	}
}

/**
 * Set the configuration in the client.
 */
void MusicBrainzDialog::setClientConfig()
{
	if (m_client) {
		m_client->setConfig(
			getServer(),
			Kid3App::s_miscCfg.m_proxy, Kid3App::s_miscCfg.m_useProxy);
	}
}

/**
 * Create and start the MusicBrainz client.
 */
void MusicBrainzDialog::startClient()
{
	clearResults();
	if (!m_client) {
		m_client = new MusicBrainzClient(m_trackDataVector);
		setClientConfig();
		connect(m_client, SIGNAL(statusChanged(int, QString)),
						this, SLOT(setFileStatus(int, QString)));
		connect(m_client, SIGNAL(metaDataReceived(int, ImportTrackData&)),
						this, SLOT(setMetaData(int, ImportTrackData&)));
		connect(m_client, SIGNAL(resultsReceived(int, ImportTrackDataVector&)),
						this, SLOT(setResults(int, ImportTrackDataVector&)));
		m_client->addFiles();
	}
	if (!m_timer) {
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(timerDone()));
	}
	if (m_timer) {
		m_timer->start(1000);
	}
}

/**
 * Stop and destroy the MusicBrainz client.
 */
void MusicBrainzDialog::stopClient()
{
	if (m_timer) {
		m_timer->stop();
		// will be destroyed by parent
	}
	if (m_client) {
		m_client->disconnect();
		delete m_client;
		m_client = 0;
	}
}

/**
 * Hides the dialog and sets the result to QDialog::Accepted.
 */
void MusicBrainzDialog::accept()
{
	apply();
	stopClient();
	QDialog::accept();
}

/**
 * Hides the dialog and sets the result to QDialog::Rejected.
 */
void MusicBrainzDialog::reject()
{
	stopClient();
	QDialog::reject();
}

/**
 * Apply imported data.
 */
void MusicBrainzDialog::apply()
{
	bool newTrackData = false;
	unsigned numRows = m_trackDataVector.size();
	for (unsigned index = 0; index < numRows; ++index) {
#if QT_VERSION >= 0x040000
		QTableWidgetItem* item = m_albumTable->item(index, 0);
		if (item) {
			QComboBox* combo = dynamic_cast<QComboBox*>(m_albumTable->cellWidget(index, 0));
			int selectedItem = -1;
			if (combo) {
				selectedItem = combo->currentIndex();
			}
#else
		QComboTableItem* item =
			dynamic_cast<QComboTableItem*>(m_albumTable->item(index, 0));
		if (item) {
			int selectedItem = item->currentItem();
#endif
			if (selectedItem > 0) {
				const ImportTrackData& selectedData =
					m_trackResults[index][selectedItem - 1];
				m_trackDataVector[index].setTitle(selectedData.getTitle());
				m_trackDataVector[index].setArtist(selectedData.getArtist());
				m_trackDataVector[index].setAlbum(selectedData.getAlbum());
				m_trackDataVector[index].setTrack(selectedData.getTrack());
				m_trackDataVector[index].setYear(selectedData.getYear());
				m_trackDataVector[index].setImportDuration(
					selectedData.getImportDuration());
				newTrackData = true;
			}
		}
	}
	if (newTrackData) {
		emit trackDataUpdated();
	}
}

/**
 * Shows the dialog as a modal dialog.
 */
int MusicBrainzDialog::exec()
{
	startClient();
	return QDialog::exec();
}

/**
 * Called when the periodic timer times out.
 * Used to poll the MusicBrainz client.
 */
void MusicBrainzDialog::timerDone()
{
	if (m_client) {
		m_client->pollStatus();
	}
}

/**
 * Set the status of a file.
 *
 * @param index  index of file
 * @param status status string
 */
void MusicBrainzDialog::setFileStatus(int index, QString status)
{
#if QT_VERSION >= 0x040000
	QTableWidgetItem* twi = m_albumTable->item(index, 1);
	if (twi) {
		twi->setText(status);
	} else {
		twi = new QTableWidgetItem(status);
		twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
		m_albumTable->setItem(index, 1, twi);
	}
#else
	m_albumTable->setText(index, 1, status);
#endif
}

/**
 * Update the track data combo box of a file.
 *
 * @param index  index of file
 */
void MusicBrainzDialog::updateFileTrackData(int index)
{
	QStringList stringList;
	unsigned numResults = m_trackResults[index].size();
	QString str(numResults == 0 ?
							i18n("No result") : i18n("No result selected"));
	stringList.push_back(str);
	for (ImportTrackDataVector::const_iterator it = m_trackResults[index].begin();
			 it != m_trackResults[index].end();
			 ++it) {
		str.sprintf("%02d ", (*it).getTrack());
		str += (*it).getTitle();
		str += '/';
		str += (*it).getArtist();
		str += " - ";
		str += (*it).getAlbum();
		if ((*it).getYear() > 0) {
			str += QString(" [%1]").arg((*it).getYear());
		}
		stringList.push_back(str);
	}
#if QT_VERSION >= 0x040000
	QTableWidgetItem* item = m_albumTable->item(index, 0);
	if (item) {
		QComboBox* combo = dynamic_cast<QComboBox*>(m_albumTable->cellWidget(index, 0));
		if (combo) {
			combo->clear();
			combo->addItems(stringList);
			// if there is only one result, select it, else let the user select
			if (numResults == 1) {
				combo->setCurrentIndex(1);
			}
		}
	}
#else
	QComboTableItem* item =
		dynamic_cast<QComboTableItem*>(m_albumTable->item(index, 0));
	if (item) {
		item->setStringList(stringList);
		// if there is only one result, select it, else let the user select
		if (numResults == 1) {
			item->setCurrentItem(1);
		}
	}
#endif
}

/**
 * Set meta data for a file.
 *
 * @param index     index of file
 * @param trackData meta data
 */
void MusicBrainzDialog::setMetaData(int index, ImportTrackData& trackData)
{
	m_trackResults[index].clear();
	m_trackResults[index].push_back(trackData);
	updateFileTrackData(index);
}

/**
 * Set result list for a file.
 *
 * @param index           index of file
 * @param trackDataVector result list
 */
void MusicBrainzDialog::setResults(
	int index, ImportTrackDataVector& trackDataVector)
{
	m_trackResults[index] = trackDataVector;
	updateFileTrackData(index);
	for (
		 ImportTrackDataVector::const_iterator
			 it = trackDataVector.begin();
			 it != trackDataVector.end();
			 ++it) {
	}
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString MusicBrainzDialog::getServer() const
{
	QString server(m_serverComboBox->currentText());
	if (server.isEmpty()) {
		server = "musicbrainz.org:80";
	}
	return server;
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void MusicBrainzDialog::setServer(const QString& srv)
{
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

/**
 * Save the local settings to the configuration.
 */
void MusicBrainzDialog::saveConfig()
{
	Kid3App::s_musicBrainzCfg.m_server = getServer();
}

/**
 * Show help.
 */
void MusicBrainzDialog::showHelp()
{
	Kid3App::displayHelp("import-musicbrainz");
}

/**
 * Show the name of the current track in the status bar.
 *
 * @param row table row
 */
void MusicBrainzDialog::showFilenameInStatusBar(int row)
{
	if (m_statusBar) {
		unsigned numRows = m_trackDataVector.size();
		if (row >= 0 && row < static_cast<int>(numRows)) {
			QFileInfo fi(m_trackDataVector[row].getAbsFilename());
			m_statusBar->QCM_showMessage(fi.fileName());
		} else {
			m_statusBar->QCM_clearMessage();
		}
	}
}

#else // HAVE_TUNEPIMP

MusicBrainzDialog::MusicBrainzDialog(QWidget*, ImportTrackDataVector&) {}
MusicBrainzDialog::~MusicBrainzDialog() {}
int MusicBrainzDialog::exec() { return 0; }
void MusicBrainzDialog::accept() {}
void MusicBrainzDialog::reject() {}
void MusicBrainzDialog::setClientConfig() {}
void MusicBrainzDialog::timerDone() {}
void MusicBrainzDialog::apply() {}
void MusicBrainzDialog::setFileStatus(int, QString) {}
void MusicBrainzDialog::updateFileTrackData(int) {}
void MusicBrainzDialog::setMetaData(int, ImportTrackData&) {}
void MusicBrainzDialog::setResults(int, ImportTrackDataVector&) {}
void MusicBrainzDialog::saveConfig() {}
void MusicBrainzDialog::showHelp() {}
void MusicBrainzDialog::showFilenameInStatusBar(int) {}

#endif // HAVE_TUNEPIMP
