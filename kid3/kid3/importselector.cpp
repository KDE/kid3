/**
 * \file importselector.cpp
 * Import selector widget.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qbitarray.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3Table>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <QGridLayout>
#include <Q3GroupBox>
#else
#include <qtable.h>
#include <qgroupbox.h>
#endif
#include "genres.h"
#include "standardtags.h"
#include "importparser.h"
#include "freedbdialog.h"
#include "musicbrainzreleasedialog.h"
#include "discogsdialog.h"
#include "kid3.h"
#include "taggedfile.h"
#include "importselector.h"
#ifdef HAVE_TUNEPIMP
#include "musicbrainzdialog.h"
#include "musicbrainzconfig.h"
#endif

/**
 * Table used for import data.
 * Subclassed to be able to change the cell colors.
 */
class ImportTable: public Q3Table {
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 * @param name   Qt name
	 */
	ImportTable(QWidget* parent = 0, const char* name = 0) :
		Q3Table(parent, name) {}
	/**
	 * Constructor.
	 * @param numRows number of rows
	 * @param parent  parent widget
	 * @param name    Qt name
	 */
	ImportTable(int numRows, int numCols, QWidget* parent = 0, const char* name = 0) :
		Q3Table(numRows, numCols, parent, name) {} 
	/**
	 * Clear marked rows.
	 */
	void clearMarks() { m_markedRows.fill(false); }
	/**
	 * Mark a row.
	 * This first cell of such a row will have a red background.
	 * @param row number of row
	 */
	void markRow(unsigned row) {
		if (m_markedRows.size() <= row) m_markedRows.resize(row + 1);
		m_markedRows.setBit(row);
	}
	/**
	 * Check if a row is marked.
	 * @param row number of row
	 * @return true if row is marked.
	 */
	bool isRowMarked(unsigned row) const {
		return m_markedRows.size() > row && m_markedRows.testBit(row);
	}

protected:
	/**
	 * Called when a cell is painted.
	 * Paint the first cell of marked rows with red background.
	 * @param p painter
	 * @param row column
	 * @param col column
	 * @param cr  cell rectangle
	 * @param selected true if selected
	 * @param cg color group
	 */
	virtual void paintCell(QPainter* p, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg) {
		if (col == 0 && isRowMarked(row)) {
			QColorGroup g(cg);
			g.setColor(QColorGroup::Base, QColor("red"));
			Q3Table::paintCell(p, row, col, cr, selected, g);
		} else {
			Q3Table::paintCell(p, row, col, cr, selected, cg);
		}
	}

private:
	QBitArray m_markedRows;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 * @param name          Qt name
 * @param f             window flags
 */
ImportSelector::ImportSelector(
	QWidget *parent, ImportTrackDataVector& trackDataList,
	const char *name, Qt::WFlags f) :
	Q3VBox(parent, name, f),
	m_trackDataVector(trackDataList)
{
	freedbDialog = 0;
	m_musicBrainzReleaseDialog = 0;
	m_discogsDialog = 0;
#ifdef HAVE_TUNEPIMP
	m_musicBrainzDialog = 0;
#endif
	importSource = None;
	header_parser = new ImportParser();
	track_parser = new ImportParser();
	setSpacing(6);
	setMargin(6);
	tab = new ImportTable(0, NumColumns, this);
	tab->setReadOnly(true);
	tab->setFocusStyle(Q3Table::FollowStyle);
	tab->setRowMovingEnabled(true);
	tab->setSelectionMode(Q3Table::NoSelection);
	Q3Header* hHeader = tab->horizontalHeader();
	Q3Header* vHeader = tab->verticalHeader();
	hHeader->setLabel(LengthColumn, i18n("Length"));
	hHeader->setLabel(TrackColumn, i18n("Track"));
	hHeader->setLabel(TitleColumn, i18n("Title"));
	hHeader->setLabel(ArtistColumn, i18n("Artist"));
	hHeader->setLabel(AlbumColumn, i18n("Album"));
	hHeader->setLabel(YearColumn, i18n("Year"));
	hHeader->setLabel(GenreColumn, i18n("Genre"));
	hHeader->setLabel(CommentColumn, i18n("Comment"));
	tab->adjustColumn(TrackColumn);
	tab->adjustColumn(YearColumn);

	Q3GroupBox *fmtbox = new Q3GroupBox(3, Qt::Vertical, i18n("Format"), this);
#if QT_VERSION >= 0x040000
	fmtbox->setInsideMargin(5);
#endif
	formatComboBox = new QComboBox(false, fmtbox, "formatComboBox");
	formatComboBox->setEditable(true);
	headerLineEdit = new QLineEdit(fmtbox);
	trackLineEdit = new QLineEdit(fmtbox);
	connect(formatComboBox, SIGNAL(activated(int)), this, SLOT(setFormatLineEdit(int)));

	QWidget *butbox = new QWidget(this);
	QGridLayout *butlayout = new QGridLayout(butbox, 2, 7, 0, 6);
	fileButton = new QPushButton(i18n("From F&ile"), butbox);
	butlayout->addWidget(fileButton, 0, 0);
	clipButton = new QPushButton(i18n("From Clip&board"), butbox);
	butlayout->addWidget(clipButton, 0, 1);
	freedbButton = new QPushButton(i18n("From &freedb.org"), butbox);
	butlayout->addWidget(freedbButton, 0, 2);
	m_discogsButton = new QPushButton(i18n("From Disco&gs"), butbox);
	butlayout->addWidget(m_discogsButton, 0, 3);
	QSpacerItem *butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	butlayout->addItem(butspacer, 0, 4);
	QLabel *destLabel = new QLabel(butbox, "destLabel");
	destLabel->setText(i18n("D&estination:"));
	butlayout->addWidget(destLabel, 0, 5);
	destComboBox = new QComboBox(false, butbox, "destComboBox");
	destComboBox->insertItem(i18n("Tag 1"), DestV1);
	destComboBox->insertItem(i18n("Tag 2"), DestV2);
	destLabel->setBuddy(destComboBox);
	butlayout->addWidget(destComboBox, 0, 7);

	m_musicBrainzReleaseButton = new QPushButton(i18n("From MusicBrainz &Release"), butbox);
	butlayout->addMultiCellWidget(m_musicBrainzReleaseButton, 1, 1, 0, 1);
#ifdef HAVE_TUNEPIMP
	m_musicBrainzButton = new QPushButton(i18n("From &MusicBrainz Fingerprint"), butbox);
	butlayout->addMultiCellWidget(m_musicBrainzButton, 1, 1, 2, 3);
#endif

	QWidget* matchBox = new QWidget(this, "matchBox");
	QHBoxLayout* matchLayout = new QHBoxLayout(matchBox, 0, 6, "matchLayout");
	mismatchCheckBox = new QCheckBox(
		i18n("Check maximum allowable time &difference (sec):"), matchBox ,
		"mismatchCheckBox");
	matchLayout->addWidget(mismatchCheckBox);
	maxDiffSpinBox = new QSpinBox(matchBox, "maxDiffSpinBox");
	maxDiffSpinBox->setMaxValue(9999);
	matchLayout->addWidget(maxDiffSpinBox);
	QSpacerItem* matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
	matchLayout->addItem(matchSpacer);
	QLabel* matchLabel = new QLabel(i18n("Match with:"), matchBox, "matchLabel");
	matchLayout->addWidget(matchLabel);
	m_lengthButton = new QPushButton(i18n("&Length"), matchBox, "lengthButton");
	matchLayout->addWidget(m_lengthButton);
	m_trackButton = new QPushButton(i18n("T&rack"), matchBox, "trackButton");
	matchLayout->addWidget(m_trackButton);
	m_titleButton = new QPushButton(i18n("&Title"), matchBox, "titleButton");
	matchLayout->addWidget(m_titleButton);

	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
	connect(clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
	connect(freedbButton, SIGNAL(clicked()), this, SLOT(fromFreedb()));
	connect(m_musicBrainzReleaseButton, SIGNAL(clicked()), this, SLOT(fromMusicBrainzRelease()));
	connect(m_discogsButton, SIGNAL(clicked()), this, SLOT(fromDiscogs()));
#ifdef HAVE_TUNEPIMP
	connect(m_musicBrainzButton, SIGNAL(clicked()), this, SLOT(fromMusicBrainz()));
#endif
	connect(m_lengthButton, SIGNAL(clicked()), this, SLOT(matchWithLength()));
	connect(m_trackButton, SIGNAL(clicked()), this, SLOT(matchWithTrack()));
	connect(m_titleButton, SIGNAL(clicked()), this, SLOT(matchWithTitle()));
	connect(vHeader, SIGNAL(indexChange(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
	connect(mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
	connect(maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
}

/**
 * Destructor.
 */
ImportSelector::~ImportSelector()
{
	delete header_parser;
	delete track_parser;
	if (freedbDialog) {
		freedbDialog->disconnect();
		delete freedbDialog;
		freedbDialog = 0;
	}
	if (m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog->disconnect();
		delete m_musicBrainzReleaseDialog;
		m_musicBrainzReleaseDialog = 0;
	}
	if (m_discogsDialog) {
		m_discogsDialog->disconnect();
		delete m_discogsDialog;
		m_discogsDialog = 0;
	}
#ifdef HAVE_TUNEPIMP
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->disconnect();
		delete m_musicBrainzDialog;
		m_musicBrainzDialog = 0;
	}
#endif
}

/**
 * Clear dialog data.
 */
void ImportSelector::clear()
{
	tab->setNumRows(0);
	destComboBox->setCurrentItem(
		static_cast<int>(Kid3App::s_genCfg.importDestV1 ? DestV1 : DestV2));

	formatHeaders = Kid3App::s_genCfg.importFormatHeaders;
	formatTracks = Kid3App::s_genCfg.importFormatTracks;
	formatComboBox->clear();
	formatComboBox->insertStringList(Kid3App::s_genCfg.importFormatNames);
	formatComboBox->setCurrentItem(Kid3App::s_genCfg.importFormatIdx);
	setFormatLineEdit(Kid3App::s_genCfg.importFormatIdx);

	mismatchCheckBox->setChecked(Kid3App::s_genCfg.enableTimeDifferenceCheck);
	maxDiffSpinBox->setValue(Kid3App::s_genCfg.maxTimeDifference);
}

/**
 * Look for album specific information (artist, album, year, genre) in
 * a header.
 *
 * @param st standard tags to put resulting values in,
 *           fields which are not found are not touched.
 *
 * @return true if one or more field were found.
 */
bool ImportSelector::parseHeader(StandardTags &st)
{
	int pos = 0;
	header_parser->setFormat(headerLineEdit->text());
	return header_parser->getNextTags(text, st, pos);
}

/**
 * Let user select file, assign file contents to text and preview in
 * table.
 */
void ImportSelector::fromFile()
{
	QString fn =
#ifdef CONFIG_USE_KDE
		KFileDialog::getOpenFileName(QString::null, QString::null, this);
#else
		QFileDialog::getOpenFileName(QString::null, QString::null, this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_ReadOnly)) {
			QTextStream stream(&file);
			text = stream.read();
			if (!text.isNull()) {
				updateTrackData(File);
				showPreview();
			}
			file.close();
		}
	}
}

/**
 * Assign clipboard contents to text and preview in table.
 */
void ImportSelector::fromClipboard()
{
	QClipboard *cb = QApplication::clipboard();
#if QT_VERSION >= 0x030100
	text = cb->text(QClipboard::Clipboard);
	if (!text.isNull() && updateTrackData(Clipboard)) {
		showPreview();
	} else {
		text = cb->text(QClipboard::Selection);
		if (!text.isNull()) {
			updateTrackData(Clipboard);
			showPreview();
		}
	}
#else
	text = cb->text();
	if (!text.isNull()) {
		updateTrackData(Clipboard);
		showPreview();
	}
#endif
}

/**
 * Import from freedb.org and preview in table.
 */
void ImportSelector::fromFreedb()
{
	if (!freedbDialog) {
		freedbDialog = new FreedbDialog(this, m_trackDataVector);
		connect(freedbDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (freedbDialog) {
		freedbDialog->setArtistAlbum(m_trackDataVector.artist,
																 m_trackDataVector.album);
		(void)freedbDialog->exec();
	}
}

/**
 * Import from MusicBrainz release database and preview in table.
 */
void ImportSelector::fromMusicBrainzRelease()
{
	if (!m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog = new MusicBrainzReleaseDialog(this, m_trackDataVector);
		connect(m_musicBrainzReleaseDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog->setArtistAlbum(m_trackDataVector.artist,
																							 m_trackDataVector.album);
		(void)m_musicBrainzReleaseDialog->exec();
	}
}
/**
 * Import from www.discogs.com and preview in table.
 */
void ImportSelector::fromDiscogs()
{
	if (!m_discogsDialog) {
		m_discogsDialog = new DiscogsDialog(this, m_trackDataVector);
		connect(m_discogsDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_discogsDialog) {
		m_discogsDialog->setArtistAlbum(m_trackDataVector.artist,
																		m_trackDataVector.album);
		(void)m_discogsDialog->exec();
	}
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ImportSelector::setFormatLineEdit(int index)
{
	headerLineEdit->setText(formatHeaders[index]);
	trackLineEdit->setText(formatTracks[index]);
}

/**
 * Update track data list with imported tags.
 *
 * @param impSrc import source
 *
 * @return true if tags were found.
 */
bool ImportSelector::updateTrackData(ImportSource impSrc) {
	StandardTags st_hdr;
	st_hdr.setInactive();
	importSource = impSrc;
	(void)parseHeader(st_hdr);

	StandardTags st(st_hdr);
	bool start = true;
	ImportTrackDataVector::iterator it = m_trackDataVector.begin();
	bool atTrackDataListEnd = (it == m_trackDataVector.end());
	while (getNextTags(st, start)) {
		start = false;
		if (atTrackDataListEnd) {
			ImportTrackData trackData;
			trackData.setStandardTags(st);
			m_trackDataVector.push_back(trackData);
		} else {
			(*it).setStandardTags(st);
			++it;
			atTrackDataListEnd = (it == m_trackDataVector.end());
		}
		st = st_hdr;
	}
	st.setInactive();
	while (!atTrackDataListEnd) {
		if ((*it).getFileDuration() == 0) {
			it = m_trackDataVector.erase(it);
		} else {
			(*it).setStandardTags(st);
			(*it).setImportDuration(0);
			++it;
		}
		atTrackDataListEnd = (it == m_trackDataVector.end());
	}

	if (!start) {
		/* start is false => tags were found */
		Q3ValueList<int>* trackDuration = getTrackDurations();
		if (trackDuration) {
			it = m_trackDataVector.begin();
			for (Q3ValueList<int>::const_iterator tdit = trackDuration->begin();
					 tdit != trackDuration->end();
					 ++tdit) {
				if (it != m_trackDataVector.end()) {
					(*it).setImportDuration(*tdit);
					++it;
				} else {
					break;
				}
			}
		}
		return true;
	}
	return false;
}

/**
 * Show fields to import in text as preview in table.
 */
void ImportSelector::showPreview() {
	tab->setNumRows(0);
	int row = 0;
	Q3Header* vHeader = tab->verticalHeader();
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		tab->setNumRows(row + 1);
		int fileDuration = (*it).getFileDuration();
		if (fileDuration != 0) {
			vHeader->setLabel(row, TaggedFile::formatTime(fileDuration));
		}
		int importDuration = (*it).getImportDuration();
		if (importDuration != 0)
			tab->setText(row, LengthColumn, TaggedFile::formatTime(importDuration));
		if ((*it).track != -1) {
			QString trackStr;
			trackStr.setNum((*it).track);
			tab->setText(row, TrackColumn, trackStr);
		}
		if (!(*it).title.isNull())
			tab->setText(row, TitleColumn, (*it).title);
		if (!(*it).artist.isNull())
			tab->setText(row, ArtistColumn, (*it).artist);
		if (!(*it).album.isNull())
			tab->setText(row, AlbumColumn, (*it).album);
		if ((*it).year != -1) {
			QString yearStr;
			yearStr.setNum((*it).year);
			tab->setText(row, YearColumn, yearStr);
		}
		if ((*it).genre != -1) {
			QString genreStr(Genres::getName((*it).genre));
			if (genreStr.isEmpty() && !(*it).genreStr.isEmpty()) {
				genreStr = (*it).genreStr;
			}
			tab->setText(row, GenreColumn, genreStr);
		}
		if (!(*it).comment.isNull())
			tab->setText(row, CommentColumn, (*it).comment);
		++row;
	}

	// make time difference check
	tab->clearMarks();
	bool diffCheckEnable;
	int maxDiff;
	getTimeDifferenceCheck(diffCheckEnable, maxDiff);
	if (diffCheckEnable) {
		bool tracksAvailable = false;
		row = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			int fileDuration = (*it).getFileDuration();
			int importDuration = (*it).getImportDuration();
			if (fileDuration != 0 && importDuration != 0) {
				int diff = fileDuration > importDuration ?
					fileDuration - importDuration : importDuration - fileDuration;
				if (diff > maxDiff) {
					tab->markRow(row);
				}
				tracksAvailable = true;
			}
			if (tracksAvailable &&
					((fileDuration == 0 && importDuration != 0) ||
					 (fileDuration != 0 && importDuration == 0))) {
				tab->markRow(row);
			}
			++row;
		}
	}
}

/**
 * Get next line as standardtags from imported file or clipboard.
 *
 * @param st standard tags
 * @param start true to start with the first line, false for all
 *              other lines
 *
 * @return true if ok (result in st),
 *         false if end of file reached.
 */
bool ImportSelector::getNextTags(StandardTags &st, bool start)
{
	static int pos = 0;
	if (start || pos == 0) {
		pos = 0;
		track_parser->setFormat(trackLineEdit->text(), true);
	}
	return track_parser->getNextTags(text, st, pos);
}

/**
 * Get import destination.
 *
 * @return DestV1 or DestV2 for ID3v1 or ID3v2.
 */
ImportSelector::Destination ImportSelector::getDestination()
{
	return (Destination)destComboBox->currentItem();
}

/**
 * Save the local settings to the configuration.
 */
void ImportSelector::saveConfig()
{
	Kid3App::s_genCfg.importDestV1 = 
		(destComboBox->currentItem() == static_cast<int>(ImportSelector::DestV1));

	Kid3App::s_genCfg.importFormatIdx = formatComboBox->currentItem();
	Kid3App::s_genCfg.importFormatNames[Kid3App::s_genCfg.importFormatIdx] = formatComboBox->currentText();
	Kid3App::s_genCfg.importFormatHeaders[Kid3App::s_genCfg.importFormatIdx] = headerLineEdit->text();
	Kid3App::s_genCfg.importFormatTracks[Kid3App::s_genCfg.importFormatIdx] = trackLineEdit->text();
	getTimeDifferenceCheck(Kid3App::s_genCfg.enableTimeDifferenceCheck,
												 Kid3App::s_genCfg.maxTimeDifference);

}

/**
 * Get list with track durations.
 *
 * @return list with track durations,
 *         0 if no track durations found.
 */
Q3ValueList<int>* ImportSelector::getTrackDurations()
{
	Q3ValueList<int>* lst = 0;
	if (header_parser && ((lst = header_parser->getTrackDurations()) != 0) &&
			(lst->size() > 0)) {
		return lst;
	} else if (track_parser && ((lst = track_parser->getTrackDurations()) != 0) &&
						 (lst->size() > 0)) {
		return lst;
	} else {
		return 0;
	}
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportSelector::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = mismatchCheckBox->isChecked();
	maxDiff = maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportSelector::maxDiffChanged() {
	if (mismatchCheckBox->isChecked()) {
		showPreview();
	}
}

/**
 * Move a table row.
 *
 * @param section not used
 * @param fromIndex index of position moved from
 * @param fromIndex index of position moved to
 */
void ImportSelector::moveTableRow(int, int fromIndex, int toIndex) {
	if (toIndex > fromIndex && toIndex > 0) {
		--toIndex;
	}
	int numTracks = static_cast<int>(m_trackDataVector.size());
	if (fromIndex < numTracks && toIndex < numTracks) {
		// swap elements but keep file durations and names
		ImportTrackData fromData(m_trackDataVector[fromIndex]);
		ImportTrackData toData(m_trackDataVector[toIndex]);
		m_trackDataVector[fromIndex].setStandardTags(toData);
		m_trackDataVector[toIndex].setStandardTags(fromData);
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
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
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
			if ((*it).track > 0 && (*it).track <= static_cast<int>(numTracks)) {
				md[i].track = (*it).track - 1;
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
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
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
				int startIndex = fileName.findRev(QDir::separator()) + 1;
				int endIndex = fileName.findRev('.');
				if (endIndex > startIndex) {
					fileName = fileName.mid(startIndex, endIndex - startIndex);
				} else {
					fileName = fileName.mid(startIndex);
				}
				md[i].fileWords = QStringList::split(
					nonWordCharRegExp, fileName.lower().
					replace(nonLetterSpaceRegExp, " "));
			}
			if (!(*it).title.isEmpty()) {
				++numImports;
				md[i].titleWords = QStringList::split(
					nonWordCharRegExp, (*it).title.lower().
					replace(nonLetterSpaceRegExp, " "));
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
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}
