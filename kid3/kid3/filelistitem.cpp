/**
 * \file filelistitem.cpp
 * Item in filelist.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 02 Oct 2006
 */

#include "filelistitem.h"
#include "taggedfile.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>

/** Empty pixmap, will be allocated in constructor */
QPixmap *FileListItem::nullPixmap = 0;
/** Pixmap for modified file, will be allocated in constructor */
QPixmap *FileListItem::modifiedPixmap = 0;
/** Pixmap for V1V2, will be allocated in constructor */
QPixmap *FileListItem::v1v2Pixmap = 0;
/** Pixmap for V1, will be allocated in constructor */
QPixmap *FileListItem::v1Pixmap = 0;
/** Pixmap for V2, will be allocated in constructor */
QPixmap *FileListItem::v2Pixmap = 0;
/** Pixmap for "no tag", will be allocated in constructor */
QPixmap *FileListItem::notagPixmap = 0;

/* The bitmaps are stored here instead of using KDE bitmaps to make
   it work for the Qt only versions. */
/** picture for modified pixmap */
static const char * const modified_xpm[] = {
	"16 16 33 1",
	". c None",
	"B c None",
	"A c None",
	"C c None",
	"D c None",
	"E c None",
	"# c #000000",
	"b c #006562",
	"j c #414041",
	"x c #525552",
	"f c #529594",
	"e c #52959c",
	"w c #5a555a",
	"v c #626162",
	"u c #626562",
	"r c #737173",
	"p c #737573",
	"q c #7b757b",
	"o c #838183",
	"m c #838583",
	"z c #8b8d8b",
	"l c #949194",
	"k c #9c959c",
	"i c #a4a1a4",
	"h c #a4a5a4",
	"y c #b4b6b4",
	"g c #bdb6bd",
	"a c #c5c2c5",
	"s c #c5c6c5",
	"c c #cdc6cd",
	"t c #dedade",
	"n c #eeeaee",
	"d c #ffffff",
	".......##.......",
	"......#ab#......",
	".....#cbde#.....",
	"....#abdddf#....",
	"...#gbddddde#...",
	"..#hijddddddf#..",
	".#kjkljdddddd##.",
	"#mjnjmojddddjma#",
	"#jnpnjqrjddjqs#.",
	"#drtttjuvjjua#..",
	".#dasajjwxws#...",
	"..#dyjzljxa#...A",
	"...#jrrjws#...AB",
	"....#cjxa#...ACB",
	".....#cs#...ADE.",
	"......##...ABB.."
};

/** picture for empty pixmap */
static const char * const null_xpm[] = {
	"16 16 2 1",
	"# c None",
	". c None",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#.",
	".#.#.#.#.#.#.#.#",
	"#.#.#.#.#.#.#.#."
};

/* XPM */
static const char * const v1v2_xpm[] = {
	"16 16 3 1",
	"       c None",
	".      c #000000",
	"+      c #FFFFFF",
	"                ",
	"                ",
	"   .   .   .    ",
	"   .   .  ..    ",
	"    . .  . .    ",
	"    . .    .    ",
	"     .     .    ",
	"                ",
	"                ",
	"   .   .  ..    ",
	"   .   . .  .   ",
	"    . .    .    ",
	"    . .   .     ",
	"     .   ....   ",
	"                ",
	"                "};

/* XPM */
static const char * const v1_xpm[] = {
	"16 16 3 1",
	"       c None",
	".      c #000000",
	"+      c #FFFFFF",
	"                ",
	"                ",
	"   .   .   .    ",
	"   .   .  ..    ",
	"    . .  . .    ",
	"    . .    .    ",
	"     .     .    ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                "};

/* XPM */
static const char * const v2_xpm[] = {
	"16 16 3 1",
	"       c None",
	".      c #000000",
	"+      c #FFFFFF",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"                ",
	"   .   .  ..    ",
	"   .   . .  .   ",
	"    . .    .    ",
	"    . .   .     ",
	"     .   ....   ",
	"                ",
	"                "};

/* XPM */
static const char * const notag_xpm[] = {
	"16 16 3 1",
	"       c None",
	".      c #000000",
	"+      c #FFFFFF",
	"                ",
	"                ",
	"   .   .  ..    ",
	"   ..  . .  .   ",
	"   . . . .  .   ",
	"   .  .. .  .   ",
	"   .   .  ..    ",
	"                ",
	"                ",
	"  ...  .   ..   ",
	"   .  . . .     ",
	"   .  ... . ..  ",
	"   .  . . .  .  ",
	"   .  . .  ..   ",
	"                ",
	"                "};

/** width of both pixmaps, got using QPixmap::width() */
static const int pixmapWidth = 16;
/** height of both pixmaps, got using QPixmap::height() */
static const int pixmapHeight = 16;

/**
 * Constructor.
 *
 * @param file tagged file (will be owned by this item)
 */
FileListItem::FileListItem(TaggedFile* file) : m_file(file)
{
	setInSelection(false);
	if (m_file) {
		setText(m_file->getFilename());
	}

	// this two objects should be destructed when the program terminates.
	// static QPixmap objects are not possible:
	// "QPaintDevice: Must construct a QApplication before a QPaintDevice"
	if (!nullPixmap) {
		nullPixmap = new QPixmap((const char **)null_xpm);
	}
	if (!modifiedPixmap) {
		modifiedPixmap = new QPixmap((const char **)modified_xpm);
	}
	if (!v1v2Pixmap) {
		v1v2Pixmap = new QPixmap((const char **)v1v2_xpm);
	}
	if (!v1Pixmap) {
		v1Pixmap = new QPixmap((const char **)v1_xpm);
	}
	if (!v2Pixmap) {
		v2Pixmap = new QPixmap((const char **)v2_xpm);
	}
	if (!notagPixmap) {
		notagPixmap = new QPixmap((const char **)notag_xpm);
	}
}

/**
 * Destructor.
 */
FileListItem::~FileListItem()
{
	delete m_file;
}

/**
 * Set tagged file.
 * The item takes ownership of this file and the old file is deleted.
 *
 * @param file tagged file.
 */
void FileListItem::setFile(TaggedFile* file)
{
	if (m_file) {
		delete m_file;
	}
	m_file = file;
	if (m_file) {
		setText(m_file->getFilename());
	}
}

/**
 * Get height of item.
 *
 * @param lb listbox containing the item
 *
 * @return height.
 */
int FileListItem::height(const Q3ListBox* lb) const
{
	int h = text().isEmpty() ? pixmapHeight :
		QMAX(pixmapHeight, lb->fontMetrics().lineSpacing() + 1);
	return QMAX(h, QApplication::globalStrut().height());
}

/**
 * Get width of item.
 *
 * @param lb listbox containing the item
 *
 * @return width.
 */
int FileListItem::width(const Q3ListBox* lb) const
{
	if (text().isEmpty()) {
		return QMAX(pixmapWidth * 2 + 6, QApplication::globalStrut().width());
	}
	return QMAX(pixmapWidth * 2 + 6 + lb->fontMetrics().width(text()), QApplication::globalStrut().width());
}

/**
 * Paint item.
 *
 * @param painter painter used
 */
void FileListItem::paint(QPainter *painter)
{
	static const QPixmap *tagpm[] = {
		notagPixmap, v1Pixmap, v2Pixmap, v1v2Pixmap, nullPixmap
	};
	int tagpmIdx;
	if (!m_file->isTagInformationRead()) {
		tagpmIdx = 4;
	} else {
		tagpmIdx = 0;
		if (m_file->hasTagV1()) {
			tagpmIdx |= 1;
		}
		if (m_file->hasTagV2()) {
			tagpmIdx |= 2;
		}
	}
	painter->drawPixmap(3, 0, m_file->isChanged() ? *modifiedPixmap : *nullPixmap);
	painter->drawPixmap(pixmapWidth + 3, 0, *tagpm[tagpmIdx]);
	if (!text().isEmpty()) {
		QFontMetrics fm = painter->fontMetrics();
		painter->drawText(pixmapWidth * 2 + 5,
						  pixmapHeight < fm.height() ?
						  fm.ascent() + fm.leading() / 2 :
						  pixmapHeight / 2 - fm.height() / 2 + fm.ascent(),
						  text());
	}
}
