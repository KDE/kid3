/**
 * \file importdialog.cpp
 * Import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "importdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QBitArray>
#include <QToolTip>
#include <QTableView>
#include <QHeaderView>
#include <QList>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include "genres.h"
#include "freedbimporter.h"
#include "tracktypeimporter.h"
#include "musicbrainzreleaseimporter.h"
#include "discogsimporter.h"
#include "amazonimporter.h"
#include "serverimportdialog.h"
#include "textimportdialog.h"
#include "kid3.h"
#include "taggedfile.h"
#include "trackdata.h"
#include "trackdatamodel.h"
#include "frametablemodel.h"
#include "trackdatamatcher.h"
#include "qtcompatmac.h"
#include "config.h"
#ifdef HAVE_TUNEPIMP
#include "musicbrainzdialog.h"
#include "musicbrainzconfig.h"
#endif

/**
 * Constructor.
 *
 * @param parent        parent widget
 * @param caption       dialog title
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 */
ImportDialog::ImportDialog(QWidget* parent, QString& caption,
													 ImportTrackDataVector& trackDataList) :
	QDialog(parent), m_trackDataImported(false),
	m_autoStartSubDialog(ASD_None),
	m_trackDataVector(trackDataList)
{
	setObjectName("ImportDialog");
	setModal(true);
	setWindowTitle(caption);

	m_freedbImporter = 0;
	m_trackTypeImporter = 0;
	m_musicBrainzReleaseImporter = 0;
	m_discogsImporter = 0;
	m_amazonImporter = 0;
	m_serverImportDialog = 0;
	m_textImportDialog = 0;
#ifdef HAVE_TUNEPIMP
	m_musicBrainzDialog = 0;
#endif

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	m_trackDataModel = new TrackDataModel(this);
	m_trackDataTable = new QTableView(this);
	m_trackDataTable->setModel(m_trackDataModel);
	m_trackDataTable->resizeColumnsToContents();
	m_trackDataTable->verticalHeader()->setMovable(true);
	m_trackDataTable->horizontalHeader()->setMovable(true);
	m_trackDataTable->setItemDelegateForColumn(6, new FrameItemDelegate(this));
	connect(m_trackDataTable->verticalHeader(), SIGNAL(sectionMoved(int, int, int)),
					this, SLOT(moveTableRow(int, int, int)));
	vlayout->addWidget(m_trackDataTable);

	QWidget* butbox = new QWidget(this);
	QHBoxLayout* butlayout = new QHBoxLayout(butbox);
	butlayout->setMargin(0);
	butlayout->setSpacing(6);
	QPushButton* fileButton = new QPushButton(i18n("From F&ile/Clipboard..."),
																						butbox);
	fileButton->setAutoDefault(false);
	butlayout->addWidget(fileButton);
	QPushButton* serverButton = new QPushButton(i18n("&From Server:"), butbox);
	serverButton->setAutoDefault(false);
	butlayout->addWidget(serverButton);
	m_serverComboBox = new QComboBox(butbox);
	m_serverComboBox->setEditable(false);
	m_serverComboBox->insertItem(ImportConfig::ServerFreedb, i18n("gnudb.org"));
	m_serverComboBox->insertItem(ImportConfig::ServerTrackType, i18n("TrackType.org"));
	m_serverComboBox->insertItem(ImportConfig::ServerDiscogs, i18n("Discogs"));
	m_serverComboBox->insertItem(ImportConfig::ServerAmazon,
																	 i18n("Amazon"));
	m_serverComboBox->insertItem(ImportConfig::ServerMusicBrainzRelease,
																	 i18n("MusicBrainz Release"));
#ifdef HAVE_TUNEPIMP
	m_serverComboBox->insertItem(ImportConfig::ServerMusicBrainzFingerprint,
																	 i18n("MusicBrainz Fingerprint"));
#endif
	butlayout->addWidget(m_serverComboBox);
	QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																				 QSizePolicy::Minimum);
	butlayout->addItem(butspacer);
	QLabel* destLabel = new QLabel(butbox);
	destLabel->setText(i18n("D&estination:"));
	butlayout->addWidget(destLabel);
	m_destComboBox = new QComboBox(butbox);
	m_destComboBox->setEditable(false);
	m_destComboBox->insertItem(ImportConfig::DestV1, i18n("Tag 1"));
	m_destComboBox->insertItem(ImportConfig::DestV2, i18n("Tag 2"));
	m_destComboBox->insertItem(ImportConfig::DestV1V2, i18n("Tag 1 and Tag 2"));
	connect(m_destComboBox, SIGNAL(activated(int)),
					this, SLOT(changeTagDestination()));
	destLabel->setBuddy(m_destComboBox);
	butlayout->addWidget(m_destComboBox);
	vlayout->addWidget(butbox);

	QWidget* matchBox = new QWidget(this);
	QHBoxLayout* matchLayout = new QHBoxLayout(matchBox);
	matchLayout->setMargin(0);
	matchLayout->setSpacing(6);
	m_mismatchCheckBox = new QCheckBox(
		i18n("Check maximum allowable time &difference (sec):"), matchBox);
	matchLayout->addWidget(m_mismatchCheckBox);
	m_maxDiffSpinBox = new QSpinBox(matchBox);
	m_maxDiffSpinBox->setMaximum(9999);
	matchLayout->addWidget(m_maxDiffSpinBox);
	QSpacerItem* matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
	matchLayout->addItem(matchSpacer);
	QLabel* matchLabel = new QLabel(i18n("Match with:"), matchBox);
	matchLayout->addWidget(matchLabel);
	QPushButton* lengthButton = new QPushButton(i18n("&Length"), matchBox);
	lengthButton->setAutoDefault(false);
	matchLayout->addWidget(lengthButton);
	QPushButton* trackButton = new QPushButton(i18n("T&rack"), matchBox);
	trackButton->setAutoDefault(false);
	matchLayout->addWidget(trackButton);
	QPushButton* titleButton = new QPushButton(i18n("&Title"), matchBox);
	titleButton->setAutoDefault(false);
	matchLayout->addWidget(titleButton);
	vlayout->addWidget(matchBox);

	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromText()));
	connect(serverButton, SIGNAL(clicked()), this, SLOT(fromServer()));
	connect(m_serverComboBox, SIGNAL(activated(int)), this, SLOT(fromServer()));
	connect(lengthButton, SIGNAL(clicked()), this, SLOT(matchWithLength()));
	connect(trackButton, SIGNAL(clicked()), this, SLOT(matchWithTrack()));
	connect(titleButton, SIGNAL(clicked()), this, SLOT(matchWithTitle()));
	connect(m_mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
	connect(m_maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
	connect(this, SIGNAL(finished(int)), this, SLOT(hideSubdialogs()));

	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	helpButton->setAutoDefault(false);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	saveButton->setAutoDefault(false);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	okButton->setAutoDefault(false);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	cancelButton->setAutoDefault(false);
	hlayout->addWidget(helpButton);
	hlayout->addWidget(saveButton);
	hlayout->addItem(hspacer);
	hlayout->addWidget(okButton);
	hlayout->addWidget(cancelButton);
	connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
ImportDialog::~ImportDialog()
{
	delete m_textImportDialog;
	delete m_serverImportDialog;
	delete m_freedbImporter;
	delete m_trackTypeImporter;
	delete m_musicBrainzReleaseImporter;
	delete m_discogsImporter;
	delete m_amazonImporter;
#ifdef HAVE_TUNEPIMP
	delete m_musicBrainzDialog;
#endif
}

/**
 * Import from server and preview in table.
 */
void ImportDialog::fromServer()
{
	if (m_serverComboBox) {
		switch (m_serverComboBox->currentIndex()) {
			case ImportConfig::ServerFreedb:
				fromFreedb();
				break;
			case ImportConfig::ServerTrackType:
				fromTrackType();
				break;
			case ImportConfig::ServerDiscogs:
				fromDiscogs();
				break;
			case ImportConfig::ServerAmazon:
				fromAmazon();
				break;
			case ImportConfig::ServerMusicBrainzRelease:
				fromMusicBrainzRelease();
				break;
			case ImportConfig::ServerMusicBrainzFingerprint:
				fromMusicBrainz();
				break;
		}
	}
}

/**
 * Import from text.
 */
void ImportDialog::fromText()
{
	if (!m_textImportDialog) {
		m_textImportDialog = new TextImportDialog(this, m_trackDataVector);
		connect(m_textImportDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreviewAfterImport()));
	}
	m_textImportDialog->clear();
	m_textImportDialog->show();
}

/**
 * Display server import dialog.
 *
 * @param source import source
 */
void ImportDialog::displayServerImportDialog(ServerImporter* source)
{
	if (!m_serverImportDialog) {
		m_serverImportDialog = new ServerImportDialog(this);
		connect(m_serverImportDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreviewAfterImport()));
	}
	if (m_serverImportDialog) {
		m_serverImportDialog->setImportSource(source);
		m_serverImportDialog->setArtistAlbum(m_trackDataVector.getArtist(),
																		m_trackDataVector.getAlbum());
		m_serverImportDialog->show();
	}
}

/**
 * Hide subdialogs.
 */
void ImportDialog::hideSubdialogs()
{
	if (m_serverImportDialog)
		m_serverImportDialog->hide();
	if (m_textImportDialog)
		m_textImportDialog->hide();
}

/**
 * Import from freedb.org and preview in table.
 */
void ImportDialog::fromFreedb()
{
	if (!m_freedbImporter) {
		m_freedbImporter = new FreedbImporter(this, m_trackDataVector);
	}
	displayServerImportDialog(m_freedbImporter);
}

/**
 * Import from TrackType.org and preview in table.
 */
void ImportDialog::fromTrackType()
{
	if (!m_trackTypeImporter) {
		m_trackTypeImporter = new TrackTypeImporter(this, m_trackDataVector);
	}
	displayServerImportDialog(m_trackTypeImporter);
}

/**
 * Import from MusicBrainz release database and preview in table.
 */
void ImportDialog::fromMusicBrainzRelease()
{
	if (!m_musicBrainzReleaseImporter) {
		m_musicBrainzReleaseImporter =
				new MusicBrainzReleaseImporter(this, m_trackDataVector);
	}
	displayServerImportDialog(m_musicBrainzReleaseImporter);
}

/**
 * Import from www.discogs.com and preview in table.
 */
void ImportDialog::fromDiscogs()
{
	if (!m_discogsImporter) {
		m_discogsImporter = new DiscogsImporter(this, m_trackDataVector);
	}
	displayServerImportDialog(m_discogsImporter);
}

/**
 * Import from www.amazon.com and preview in table.
 */
void ImportDialog::fromAmazon()
{
	if (!m_amazonImporter) {
		m_amazonImporter = new AmazonImporter(this, m_trackDataVector);
	}
	displayServerImportDialog(m_amazonImporter);
}

/**
 * Import from MusicBrainz and preview in table.
 */
void ImportDialog::fromMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	if (!m_musicBrainzDialog) {
		m_musicBrainzDialog = new MusicBrainzDialog(this, m_trackDataVector);
		connect(m_musicBrainzDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreviewAfterImport()));
	}
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->initTable();
		(void)m_musicBrainzDialog->exec();
	}
#endif
}

/**
 * Shows the dialog as a modal dialog.
 */
int ImportDialog::exec()
{
	switch (m_autoStartSubDialog) {
		case ASD_Freedb:
			show();
			fromFreedb();
			break;
		case ASD_TrackType:
			show();
			fromTrackType();
			break;
		case ASD_Discogs:
			show();
			fromDiscogs();
			break;
		case ASD_Amazon:
			show();
			fromAmazon();
			break;
		case ASD_MusicBrainzRelease:
			show();
			fromMusicBrainzRelease();
			break;
		case ASD_MusicBrainz:
			show();
			fromMusicBrainz();
			break;
		case ASD_None:
			break;
	}
	return QDialog::exec();
}

/**
 * Set dialog to be started automatically.
 *
 * @param asd dialog to be started
 */
void ImportDialog::setAutoStartSubDialog(AutoStartSubDialog asd)
{
	m_autoStartSubDialog = asd;

	ImportConfig::ImportServer server;
	switch (asd) {
		case ASD_Freedb:
			server = ImportConfig::ServerFreedb;
			break;
		case ASD_TrackType:
			server = ImportConfig::ServerTrackType;
			break;
		case ASD_Discogs:
			server = ImportConfig::ServerDiscogs;
			break;
		case ASD_Amazon:
			server = ImportConfig::ServerAmazon;
			break;
		case ASD_MusicBrainzRelease:
			server = ImportConfig::ServerMusicBrainzRelease;
			break;
		case ASD_MusicBrainz:
			server = ImportConfig::ServerMusicBrainzFingerprint;
			break;
		case ASD_None:
		default:
			return;
	}
	m_serverComboBox->setCurrentIndex(server);
}

/**
 * Clear dialog data.
 */
void ImportDialog::clear()
{
	m_trackDataImported = false;
	m_trackDataModel->setTrackData(ImportTrackDataVector());

	m_serverComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importServer);
	m_destComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importDest);

	m_mismatchCheckBox->setChecked(Kid3App::s_genCfg.m_enableTimeDifferenceCheck);
	m_maxDiffSpinBox->setValue(Kid3App::s_genCfg.m_maxTimeDifference);

	if (Kid3App::s_genCfg.m_importWindowWidth > 0 &&
			Kid3App::s_genCfg.m_importWindowHeight > 0) {
		resize(Kid3App::s_genCfg.m_importWindowWidth,
					 Kid3App::s_genCfg.m_importWindowHeight);
	}

	showPreview();
}

/**
 * Show fields to import in text as preview in table.
 */
void ImportDialog::showPreview()
{
	// make time difference check
	bool diffCheckEnable;
	int maxDiff;
	getTimeDifferenceCheck(diffCheckEnable, maxDiff);
	m_trackDataModel->setTimeDifferenceCheck(diffCheckEnable, maxDiff);
	m_trackDataModel->setTrackData(m_trackDataVector);
	m_trackDataTable->scrollToTop();
	m_trackDataTable->resizeColumnsToContents();
	m_trackDataTable->resizeRowsToContents();
}

/**
 * Show fields to import in text as preview in table.
 * This method also marks that an import was made and thus switching the tag
 * version is no longer possible.
 */
void ImportDialog::showPreviewAfterImport()
{
	m_trackDataImported = true;
	showPreview();
}

/**
 * Get import destination.
 *
 * @return DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both.
 */
ImportConfig::ImportDestination ImportDialog::getDestination() const
{
	return static_cast<ImportConfig::ImportDestination>(
		m_destComboBox->currentIndex());
}

/**
 * Set import destination.
 *
 * @param dest DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both
 */
void ImportDialog::setDestination(ImportConfig::ImportDestination dest)
{
	m_destComboBox->setCurrentIndex(dest);
}

/**
 * Show help.
 */
void ImportDialog::showHelp()
{
	Kid3App::displayHelp("import");
}

/**
 * Save the local settings to the configuration.
 */
void ImportDialog::saveConfig()
{
	Kid3App::s_genCfg.m_importDest = static_cast<ImportConfig::ImportDestination>(
				m_destComboBox->currentIndex());

	Kid3App::s_genCfg.m_importServer = static_cast<ImportConfig::ImportServer>(
		m_serverComboBox->currentIndex());
	getTimeDifferenceCheck(Kid3App::s_genCfg.m_enableTimeDifferenceCheck,
												 Kid3App::s_genCfg.m_maxTimeDifference);

	Kid3App::s_genCfg.m_importWindowWidth = size().width();
	Kid3App::s_genCfg.m_importWindowHeight = size().height();
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */
void ImportDialog::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = m_mismatchCheckBox->isChecked();
	maxDiff = m_maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportDialog::maxDiffChanged() {
	if (m_mismatchCheckBox->isChecked()) {
		showPreview();
	}
}

/**
 * Move a table row.
 *
 * The first parameter @a section is not used.
 * @param fromIndex index of position moved from
 * @param fromIndex index of position moved to
 */
void ImportDialog::moveTableRow(int, int fromIndex, int toIndex) {
	QHeaderView* vHeader = qobject_cast<QHeaderView*>(sender());
	if (vHeader) {
		// revert movement, but avoid recursion
		disconnect(vHeader, SIGNAL(sectionMoved(int, int, int)), 0, 0);
		vHeader->moveSection(toIndex, fromIndex);
		connect(vHeader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
	}
	int numTracks = static_cast<int>(m_trackDataVector.size());
	if (fromIndex < numTracks && toIndex < numTracks) {
		// swap elements but keep file durations and names
		ImportTrackData fromData(m_trackDataVector[fromIndex]);
		ImportTrackData toData(m_trackDataVector[toIndex]);
		m_trackDataVector[fromIndex].setFrameCollection(toData.getFrameCollection());
		m_trackDataVector[toIndex].setFrameCollection(fromData.getFrameCollection());
		m_trackDataVector[fromIndex].setImportDuration(toData.getImportDuration());
		m_trackDataVector[toIndex].setImportDuration(fromData.getImportDuration());
		// redisplay the table
		showPreview();
	}
}

/**
 * Called when OK is clicked.
 */
void ImportDialog::onOkButtonClicked()
{
	// Set changes made in the table in the track data.
	m_trackDataVector = m_trackDataModel->getTrackData();
	accept();
}

/**
 * Called when the destination combo box value is changed.
 */
void ImportDialog::changeTagDestination()
{
	// Prevent switching of track data after an import.
	if (!m_trackDataImported) {
		ImportConfig::ImportDestination dest = getDestination();

		TrackData::TagVersion tagVersion = TrackData::TagNone;
		switch (dest) {
		case ImportConfig::DestV1:
			tagVersion = TrackData::TagV1;
			break;
		case ImportConfig::DestV2:
			tagVersion = TrackData::TagV2;
			break;
		case ImportConfig::DestV1V2:
			tagVersion = TrackData::TagV2V1;
		}

		m_trackDataVector.readTags(tagVersion);
		showPreview();
	}
}

/**
 * Match import data with length.
 */
void ImportDialog::matchWithLength()
{
	bool diffCheckEnable;
	int maxDiff;
	getTimeDifferenceCheck(diffCheckEnable, maxDiff);
	if (TrackDataMatcher::matchWithLength(m_trackDataVector, diffCheckEnable, maxDiff))
		showPreview();
}

/**
 * Match import data with track number.
 */
void ImportDialog::matchWithTrack()
{
	if (TrackDataMatcher::matchWithTrack(m_trackDataVector))
		showPreview();
}

/**
 * Match import data with title.
 */
void ImportDialog::matchWithTitle()
{
	if (TrackDataMatcher::matchWithTitle(m_trackDataVector))
		showPreview();
}
