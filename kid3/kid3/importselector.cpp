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
#include <klocale.h>
#include <kfiledialog.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#include <qfiledialog.h>
#endif
#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qtable.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include "genres.h"
#include "standardtags.h"
#include "importparser.h"
#include "freedbdialog.h"
#include "freedbconfig.h"
#include "mp3file.h"
#include "importselector.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   Qt name
 * @param f      window flags
 */
ImportSelector::ImportSelector(QWidget *parent, const char *name, WFlags f)
	: QVBox(parent, name, f)
{
	freedbDialog = 0;
	freedbCfg = 0;
	importSource = None;
	header_parser = new ImportParser();
	track_parser = new ImportParser();
	setSpacing(6);
	setMargin(6);
	tab = new QTable(0, NumColumns, this);
	tab->horizontalHeader()->setLabel(TrackColumn, i18n("Track"));
	tab->horizontalHeader()->setLabel(TitleColumn, i18n("Title"));
	tab->horizontalHeader()->setLabel(ArtistColumn, i18n("Artist"));
	tab->horizontalHeader()->setLabel(AlbumColumn, i18n("Album"));
	tab->horizontalHeader()->setLabel(YearColumn, i18n("Year"));
	tab->horizontalHeader()->setLabel(GenreColumn, i18n("Genre"));
	tab->horizontalHeader()->setLabel(CommentColumn, i18n("Comment"));
	tab->horizontalHeader()->setLabel(LengthColumn, i18n("Length"));
	tab->adjustColumn(TrackColumn);
	tab->adjustColumn(YearColumn);

#if QT_VERSION >= 300
	QGroupBox *fmtbox = new QGroupBox(3, Qt::Vertical, i18n("Format"), this);
	formatComboBox = new QComboBox(false, fmtbox, "formatComboBox");
	formatComboBox->setEditable(true);
	headerLineEdit = new QLineEdit(fmtbox);
	trackLineEdit = new QLineEdit(fmtbox);
	connect(formatComboBox, SIGNAL(activated(int)), this, SLOT(setFormatLineEdit(int)));
#endif

	QWidget *butbox = new QWidget(this);
	QHBoxLayout *butlayout = new QHBoxLayout(butbox);
	butlayout->setSpacing(6);
	butlayout->setMargin(6);
	fileButton = new QPushButton(i18n("From File"), butbox);
	butlayout->addWidget(fileButton);
	clipButton = new QPushButton(i18n("From Clipboard"), butbox);
	butlayout->addWidget(clipButton);
	freedbButton = new QPushButton(i18n("From freedb.org"), butbox);
	butlayout->addWidget(freedbButton);
	QSpacerItem *butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	butlayout->addItem(butspacer);
	QLabel *destLabel = new QLabel(butbox, "destLabel");
	destLabel->setText(i18n("Destination:"));
	butlayout->addWidget(destLabel);
	destComboBox = new QComboBox(false, butbox, "destComboBox");
	destComboBox->insertItem("ID3v1", DestV1);
	destComboBox->insertItem("ID3v2", DestV2);
	butlayout->addWidget(destComboBox);

	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
	connect(clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
	connect(freedbButton, SIGNAL(clicked()), this, SLOT(fromFreedb()));
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
}

/**
 * Look for album specific information (artist, album, year, genre) in
 * a header (e.g. in a freedb header).
 *
 * @param st standard tags to put resulting values in,
 *           fields which are not found are not touched.
 *
 * @return true if one or more field were found.
 */
bool ImportSelector::parseHeader(StandardTags &st)
{
	int pos = 0;
	header_parser->setFormat(
		importSource == Freedb ? "freedb_header" :
#if QT_VERSION >= 300
		headerLineEdit->text()
#else
		/* dummy format to indicate a header format */
		"%y(+\\d)"
#endif
		);
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
		KFileDialog::getOpenFileName();
#else
		QFileDialog::getOpenFileName();
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(IO_ReadOnly)) {
			QTextStream stream(&file);
			text = stream.read();
			if (!text.isNull()) {
				importSource = File;
				(void)showPreview();
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
#if QT_VERSION >= 300
	text = cb->text(QClipboard::Clipboard);
	if (text.isNull() || !(importSource = Clipboard, showPreview())) {
		text = cb->text(QClipboard::Selection);
		if (!text.isNull()) {
			importSource = Clipboard;
			(void)showPreview();
		}
	}
#else
	text = cb->text();
	if (!text.isNull()) {
		importSource = Clipboard;
		(void)showPreview();
	}
#endif
}

/**
 * Import from freedb.org and preview in table.
 */
void ImportSelector::fromFreedb()
{
	if (!freedbDialog) {
		freedbDialog = new FreedbDialog();
		if (!freedbDialog) {
			return;
		}
		if (freedbCfg) {
			freedbDialog->setFreedbConfig(freedbCfg);
		}
		connect(freedbDialog, SIGNAL(albumDataReceived(QString)),
				this, SLOT(freedbAlbumDataReceived(QString)));
	}
	(void)freedbDialog->exec();
}

/**
 * Called when freedb.org album data is received.
 *
 * @param txt text containing album data from freedb.org
 */
void ImportSelector::freedbAlbumDataReceived(QString txt)
{
	importSource = Freedb;
	text = txt;
	showPreview();
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ImportSelector::setFormatLineEdit(int index)
{
#if QT_VERSION >= 300
	headerLineEdit->setText(formatHeaders[index]);
	trackLineEdit->setText(formatTracks[index]);
#endif
}

/**
 * Show fields to import in text as preview in table.
 *
 * @return true if tags were found.
 */
bool ImportSelector::showPreview()
{
	StandardTags st_hdr;
	tab->setNumRows(0);
	st_hdr.setInactive();
	(void)parseHeader(st_hdr);

	StandardTags st(st_hdr);
	int row = 0;
	bool start = true;
	while (getNextTags(st, start)) {
		start = false;
		tab->setNumRows(row + 1);
		if (st.track != -1) {
			QString trackStr;
			trackStr.setNum(st.track);
			tab->setText(row, TrackColumn, trackStr);
		}
		if (!st.title.isNull())
			tab->setText(row, TitleColumn, st.title);
		if (!st.artist.isNull())
			tab->setText(row, ArtistColumn, st.artist);
		if (!st.album.isNull())
			tab->setText(row, AlbumColumn, st.album);
		if (st.year != -1) {
			QString yearStr;
			yearStr.setNum(st.year);
			tab->setText(row, YearColumn, yearStr);
		}
		if (st.genre != -1) {
			QString genreStr(Genres::getName(st.genre));
			tab->setText(row, GenreColumn, genreStr);
		}
		if (!st.comment.isNull())
			tab->setText(row, CommentColumn, st.comment);
		st = st_hdr;
		++row;
	}
	if (!start) {
		/* start is false => tags were found */
		QValueList<int>* trackDuration = getTrackDurations();
		if (trackDuration) {
			row = 0;
			for(
#if QT_VERSION >= 300
				QValueList<int>::iterator
#else
				QValueListConstIterator<int>
#endif
				it = trackDuration->begin();
				it != trackDuration->end();
				++it) {
				tab->setText(row, LengthColumn, Mp3File::formatTime(*it));
				++row;
			}
		}
		for (int col = 0; col < NumColumns; col++) {
			tab->adjustColumn(col);
		}
		return true;
	}
	return false;
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
		track_parser->setFormat(
			importSource == Freedb ? "freedb_tracks" :
#if QT_VERSION >= 300
			trackLineEdit->text()
#else
			/* dummy format to indicate a track format */
			"%t(\\d+)"
#endif
			, true);
	}
	return track_parser->getNextTags(text, st, pos);
}

/**
 * Set ID3v1 or ID3v2 tags as import destination.
 *
 * @param dest DestV1 or DestV2 for ID3v1 or ID3v2
 */
void ImportSelector::setDestination(ImportSelector::Destination dest)
{
	destComboBox->setCurrentItem((int)dest);
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
 * Set import format regexp.
 *
 * @param names   import format names list
 * @param headers import format header regexps
 * @param tracks  import format track regexps
 * @param idx     selected index
 */
void ImportSelector::setImportFormat(const QStringList &names,
									 const QStringList &headers,
									 const QStringList &tracks,
									 int idx)
{
#if QT_VERSION >= 300
	formatHeaders = headers;
	formatTracks = tracks;
	formatComboBox->clear();
	formatComboBox->insertStringList(names);
	formatComboBox->setCurrentItem(idx);
	setFormatLineEdit(idx);
#endif
}

/**
 * Get import format regexp.
 *
 * @param name   import format name
 * @param header import format header regexp
 * @param track  import format track regexp
 *
 * @return index of current selection.
 */
int ImportSelector::getImportFormat(QString &name,
									QString &header,
									QString &track) const
{
#if QT_VERSION >= 300
	name = formatComboBox->currentText();
	header = headerLineEdit->text();
	track = trackLineEdit->text();
	return formatComboBox->currentItem();
#else
	return 0;
#endif
}

/**
 * Set freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportSelector::setFreedbConfig(const FreedbConfig *cfg)
{
	freedbCfg = cfg;
}

/**
 * Get freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportSelector::getFreedbConfig(FreedbConfig *cfg) const
{
	if (freedbDialog) {
		freedbDialog->getFreedbConfig(cfg);
	} else {
		// freedb dialog does not exist => copy configuration which was set
		if (freedbCfg && (freedbCfg != cfg)) {
			*cfg = *freedbCfg;
		}
	}
}

/**
 * Get list with track durations.
 *
 * @return list with track durations,
 *         0 if no track durations found.
 */
QValueList<int>* ImportSelector::getTrackDurations()
{
	QValueList<int>* lst = 0;
	if (header_parser && ((lst = header_parser->getTrackDurations()) != 0) &&
#if QT_VERSION >= 300
		(lst->size() > 0)
#else
		(lst->count() > 0)
#endif
		) {
		return lst;
	} else if (track_parser && ((lst = track_parser->getTrackDurations()) != 0) &&
#if QT_VERSION >= 300
		(lst->size() > 0)
#else
		(lst->count() > 0)
#endif
	   ) {
		return lst;
	} else {
		return 0;
	}
}
