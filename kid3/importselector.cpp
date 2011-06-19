/**
 * \file importselector.cpp
 * Import selector widget.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
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

#include "importselector.h"
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QBitArray>
#include <QCheckBox>
#include <QSpinBox>
#include <QToolTip>
#include <QTableView>
#include <QHeaderView>
#include <QList>
#include <QHBoxLayout>
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
#include "qtcompatmac.h"
#include "config.h"
#ifdef HAVE_TUNEPIMP
#include "musicbrainzdialog.h"
#include "musicbrainzconfig.h"
#endif

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 */
ImportSelector::ImportSelector(
	QWidget* parent, ImportTrackDataVector& trackDataList) :
	QWidget(parent),
	m_trackDataVector(trackDataList)
{
	setObjectName("ImportSelector");
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
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(6);
	m_trackDataModel = new TrackDataModel(this);
	m_trackDataTable = new QTableView(this);
	m_trackDataTable->setModel(m_trackDataModel);
	m_trackDataTable->resizeColumnsToContents();
	m_trackDataTable->verticalHeader()->setMovable(true);
	m_trackDataTable->horizontalHeader()->setMovable(true);
	m_trackDataTable->setItemDelegateForColumn(6, new FrameItemDelegate(this));
	connect(m_trackDataTable->verticalHeader(), SIGNAL(sectionMoved(int, int, int)),
					this, SLOT(moveTableRow(int, int, int)));
	vboxLayout->addWidget(m_trackDataTable);

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
	destLabel->setBuddy(m_destComboBox);
	butlayout->addWidget(m_destComboBox);
	vboxLayout->addWidget(butbox);

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
	matchLayout->addWidget(lengthButton);
	QPushButton* trackButton = new QPushButton(i18n("T&rack"), matchBox);
	matchLayout->addWidget(trackButton);
	QPushButton* titleButton = new QPushButton(i18n("&Title"), matchBox);
	matchLayout->addWidget(titleButton);
	vboxLayout->addWidget(matchBox);

	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromText()));
	connect(serverButton, SIGNAL(clicked()), this, SLOT(fromServer()));
	connect(m_serverComboBox, SIGNAL(activated(int)), this, SLOT(fromServer()));
	connect(lengthButton, SIGNAL(clicked()), this, SLOT(matchWithLength()));
	connect(trackButton, SIGNAL(clicked()), this, SLOT(matchWithTrack()));
	connect(titleButton, SIGNAL(clicked()), this, SLOT(matchWithTitle()));
	connect(m_mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
	connect(m_maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
}

/**
 * Destructor.
 */
ImportSelector::~ImportSelector()
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
 * Select the import server.
 *
 * @param server import server
 */
void ImportSelector::setImportServer(ImportConfig::ImportServer server)
{
	m_serverComboBox->setCurrentIndex(server);
}

/**
 * Clear dialog data.
 */
void ImportSelector::clear()
{
	m_trackDataModel->setTrackData(ImportTrackDataVector());

	m_serverComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importServer);
	m_destComboBox->setCurrentIndex(Kid3App::s_genCfg.m_importDest);

	m_mismatchCheckBox->setChecked(Kid3App::s_genCfg.m_enableTimeDifferenceCheck);
	m_maxDiffSpinBox->setValue(Kid3App::s_genCfg.m_maxTimeDifference);
}

/**
 * Import from server and preview in table.
 */
void ImportSelector::fromServer()
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
void ImportSelector::fromText()
{
	if (!m_textImportDialog) {
		m_textImportDialog = new TextImportDialog(this, m_trackDataVector);
		connect(m_textImportDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	m_textImportDialog->clear();
	m_textImportDialog->show();
}

/**
 * Display dialog with import source.
 *
 * @param source import source
 */
void ImportSelector::displayImportSourceDialog(ServerImporter* source)
{
	if (!m_serverImportDialog) {
		m_serverImportDialog = new ServerImportDialog(this);
		connect(m_serverImportDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_serverImportDialog) {
		m_serverImportDialog->setImportSource(source);
		m_serverImportDialog->setArtistAlbum(m_trackDataVector.getArtist(),
																		m_trackDataVector.getAlbum());
		m_serverImportDialog->show();
	}
}


/**
 * Import from freedb.org and preview in table.
 */
void ImportSelector::fromFreedb()
{
	if (!m_freedbImporter) {
		m_freedbImporter = new FreedbImporter(this, m_trackDataVector);
	}
	displayImportSourceDialog(m_freedbImporter);
}

/**
 * Import from TrackType.org and preview in table.
 */
void ImportSelector::fromTrackType()
{
	if (!m_trackTypeImporter) {
		m_trackTypeImporter = new TrackTypeImporter(this, m_trackDataVector);
	}
	displayImportSourceDialog(m_trackTypeImporter);
}

/**
 * Import from MusicBrainz release database and preview in table.
 */
void ImportSelector::fromMusicBrainzRelease()
{
	if (!m_musicBrainzReleaseImporter) {
		m_musicBrainzReleaseImporter =
				new MusicBrainzReleaseImporter(this, m_trackDataVector);
	}
	displayImportSourceDialog(m_musicBrainzReleaseImporter);
}

/**
 * Import from www.discogs.com and preview in table.
 */
void ImportSelector::fromDiscogs()
{
	if (!m_discogsImporter) {
		m_discogsImporter = new DiscogsImporter(this, m_trackDataVector);
	}
	displayImportSourceDialog(m_discogsImporter);
}

/**
 * Import from www.amazon.com and preview in table.
 */
void ImportSelector::fromAmazon()
{
	if (!m_amazonImporter) {
		m_amazonImporter = new AmazonImporter(this, m_trackDataVector);
	}
	displayImportSourceDialog(m_amazonImporter);
}

/**
 * Show fields to import in text as preview in table.
 */
void ImportSelector::showPreview() {
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
 * Get import destination.
 *
 * @return DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both.
 */
ImportConfig::ImportDestination ImportSelector::getDestination()
{
	return static_cast<ImportConfig::ImportDestination>(
		m_destComboBox->currentIndex());
}

/**
 * Set import destination.
 *
 * @param dest DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both
 */
void ImportSelector::setDestination(ImportConfig::ImportDestination dest)
{
	m_destComboBox->setCurrentIndex(dest);
}

/**
 * Save the local settings to the configuration.
 *
 * @param width  window width
 * @param height window height
 */
void ImportSelector::saveConfig(int width, int height)
{
	Kid3App::s_genCfg.m_importDest = static_cast<ImportConfig::ImportDestination>(
		m_destComboBox->currentIndex());

	Kid3App::s_genCfg.m_importServer = static_cast<ImportConfig::ImportServer>(
		m_serverComboBox->currentIndex());
	getTimeDifferenceCheck(Kid3App::s_genCfg.m_enableTimeDifferenceCheck,
												 Kid3App::s_genCfg.m_maxTimeDifference);

	Kid3App::s_genCfg.m_importWindowWidth = width;
	Kid3App::s_genCfg.m_importWindowHeight = height;
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportSelector::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = m_mismatchCheckBox->isChecked();
	maxDiff = m_maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportSelector::maxDiffChanged() {
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
void ImportSelector::moveTableRow(int, int fromIndex, int toIndex) {
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
 * Import from MusicBrainz and preview in table.
 */
void ImportSelector::fromMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	if (!m_musicBrainzDialog) {
		m_musicBrainzDialog = new MusicBrainzDialog(this, m_trackDataVector);
		connect(m_musicBrainzDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->initTable();
		(void)m_musicBrainzDialog->exec();
	}
#endif
}

/**
 * Match import data with length.
 */
void ImportSelector::matchWithLength()
{
	struct MatchData {
		int fileLen;      // length of file
		int importLen;    // length of import
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		bool diffCheckEnable;
		int maxDiff;
		getTimeDifferenceCheck(diffCheckEnable, maxDiff);

		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			md[i].fileLen = (*it).getFileDuration();
			if (md[i].fileLen > 0) {
				++numFiles;
			}
			md[i].importLen = (*it).getImportDuration();
			if (md[i].importLen > 0) {
				++numImports;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			// If time difference checking is enabled and the time difference
			// is not larger then the allowed limit, do not reassign the track.
			if (diffCheckEnable) {
				if (md[i].fileLen != 0 && md[i].importLen != 0) {
					int diff = md[i].fileLen > md[i].importLen ?
						md[i].fileLen - md[i].importLen : md[i].importLen - md[i].fileLen;
					if (diff <= maxDiff) {
						md[i].assignedTo = i;
						md[i].assignedFrom = i;
					}
				}
			}
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned import with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedDiff = md[i].fileLen > md[comparedTrack].importLen ?
								md[i].fileLen - md[comparedTrack].importLen :
								md[comparedTrack].importLen - md[i].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned file with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedDiff = md[comparedTrack].fileLen > md[i].importLen ?
								md[comparedTrack].fileLen - md[i].importLen :
								md[i].importLen - md[comparedTrack].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}

/**
 * Match import data with track number.
 */
void ImportSelector::matchWithTrack()
{
	struct MatchData {
		int track;        // track number starting with 0
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];

		// 1st pass: Get track data and keep correct assignments.
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			if ((*it).getTrack() > 0 && (*it).getTrack() <= static_cast<int>(numTracks)) {
				md[i].track = (*it).getTrack() - 1;
			} else {
				md[i].track = -1;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			if (md[i].track == static_cast<int>(i)) {
				md[i].assignedTo = i;
				md[i].assignedFrom = i;
			}
			++i;
		}

		// 2nd pass: Assign imported track numbers to unassigned tracks.
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedTo == -1 &&
					md[i].track >= 0 && md[i].track < static_cast<int>(numTracks)) {
				if (md[md[i].track].assignedFrom == -1) {
					md[md[i].track].assignedFrom = i;
					md[i].assignedTo = md[i].track;
				}
			}
		}

		// 3rd pass: Assign remaining tracks.
		unsigned unassignedTrack = 0;
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedFrom == -1) {
				while (unassignedTrack < numTracks) {
					if (md[unassignedTrack].assignedTo == -1) {
						md[i].assignedFrom = unassignedTrack;
						md[unassignedTrack++].assignedTo = i;
						break;
					}
					++unassignedTrack;
				}
				if (md[i].assignedFrom == -1) {
					qDebug("No track assigned to %d", i);
					failed = true;
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}

/**
 * Match import data with title.
 */
void ImportSelector::matchWithTitle()
{
	struct MatchData {
		QStringList fileWords;  // words in file name
		QStringList titleWords; // words in title
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		QRegExp nonWordCharRegExp("\\W");
		QRegExp nonLetterSpaceRegExp("[^a-z ]");
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			QString fileName = (*it).getAbsFilename();
			if (!fileName.isEmpty()) {
				++numFiles;
				int startIndex = fileName.lastIndexOf(QDir::separator()) + 1;
				int endIndex = fileName.lastIndexOf('.');
				if (endIndex > startIndex) {
					fileName = fileName.mid(startIndex, endIndex - startIndex);
				} else {
					fileName = fileName.mid(startIndex);
				}
				md[i].fileWords = fileName.toLower().
					replace(nonLetterSpaceRegExp, " ").split(nonWordCharRegExp);
			}
			if (!(*it).getTitle().isEmpty()) {
				++numImports;
				md[i].titleWords = (*it).getTitle().toLower().
					replace(nonLetterSpaceRegExp, " ").split(nonWordCharRegExp);
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned import with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[i].fileWords.begin();
									 fwit != md[i].fileWords.end();
									 ++fwit) {
								if (md[comparedTrack].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned file with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[comparedTrack].fileWords.begin();
									 fwit != md[comparedTrack].fileWords.end();
									 ++fwit) {
								if (md[i].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}
		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}

/**
 * Hide subdialogs.
 */
void ImportSelector::hideSubdialogs()
{
	if (m_serverImportDialog)
		m_serverImportDialog->hide();
	if (m_textImportDialog)
		m_textImportDialog->hide();
}
