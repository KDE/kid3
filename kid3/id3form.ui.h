/**
 * \file id3form.ui.h
 * Functions used by id3form.cpp.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "autoconf.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h> /* tr2i18n() */
#else
// to be replaced in id3form.cpp
#define i18n(s) tr(s)
#endif

#if QT_VERSION < 300
// to be replaced in id3form.cpp
#define QSizePolicy(a,b,c,d,e) QSizePolicy(a,b)
#endif

#define theApp ((Kid3App *)parentWidget())

/**
 * Button ID3v1 From Filename.
 */
void id3Form::fromFilenameV1()
{
	theApp->getTagsFromFilenameV1();
}

/**
 * Button ID3v2 From Filename.
 */
void id3Form::fromFilenameV2()
{
	theApp->getTagsFromFilenameV2();
}

/**
 * Button ID3v2 From ID3v1.
 */
void id3Form::fromID3V2()
{
	theApp->copyV1ToV2();
}

/**
 * Button ID3v1 From ID3v2.
 */
void id3Form::fromID3V1()
{
	theApp->copyV2ToV1();
}

/**
 * Button ID3v1 Copy.
 */
void id3Form::copyV1()
{
	StandardTags st;
	getStandardTagsV1(&st);
	theApp->copyTags(&st);
}

/**
 * Button ID3v2 Copy.
 */
void id3Form::copyV2()
{
	StandardTags st;
	getStandardTagsV2(&st);
	theApp->copyTags(&st);
}

/**
 * Button ID3v2 Remove.
 */
void id3Form::removeV2()
{
	StandardTags st;
	st.setEmpty();
	setStandardTagsV2(&st);
	theApp->removeTagsV2();
	framesListBox->clear();
}

/**
 * Button ID3v1 Paste.
 */
void id3Form::pasteV1()
{
	StandardTags st;
	getStandardTagsV1(&st);
	theApp->pasteTags(&st);
	setStandardTagsV1(&st);
}

/**
 * Button ID3v2 Paste.
 */
void id3Form::pasteV2()
{
	StandardTags st;
	getStandardTagsV2(&st);
	theApp->pasteTags(&st);
	setStandardTagsV2(&st);
}

/**
 * Button ID3v1 Remove.
 */
void id3Form::removeV1()
{
	StandardTags st;
	st.setEmpty();
	setStandardTagsV1(&st);
	theApp->removeTagsV1();
}

/**
 * File list box file selected
 */
void id3Form::fileSelected(void)
{
	theApp->fileSelected();
}

/**
 * Get standard tags from the ID3v1 controls.
 *
 * @param st standard tags to store result
 */
void id3Form::getStandardTagsV1(StandardTags *st)
{
	st->title   = titleV1CheckBox->isChecked()   ? titleV1LineEdit->text()
		: QString::null;
	st->artist  = artistV1CheckBox->isChecked()  ? artistV1LineEdit->text()
		: QString::null;
	st->album   = albumV1CheckBox->isChecked()   ? albumV1LineEdit->text()
		: QString::null;
	st->comment = commentV1CheckBox->isChecked() ? commentV1LineEdit->text()
		: QString::null;
	st->year    = yearV1CheckBox->isChecked()    ? yearV1SpinBox->value()
		: -1;
	st->track   = trackV1CheckBox->isChecked()   ? trackV1SpinBox->value()
		: -1;
	st->genre   = genreV1CheckBox->isChecked()   ?
		Genres::getNumber(genreV1ComboBox->currentItem()) : -1;
}

/**
 * Get standard tags from the ID3v2 controls.
 *
 * @param st standard tags to store result
 */
void id3Form::getStandardTagsV2(StandardTags *st)
{
	st->title   = titleV2CheckBox->isChecked()   ? titleV2LineEdit->text()
		: QString::null;
	st->artist  = artistV2CheckBox->isChecked()  ? artistV2LineEdit->text()
		: QString::null;
	st->album   = albumV2CheckBox->isChecked()   ? albumV2LineEdit->text()
		: QString::null;
	st->comment = commentV2CheckBox->isChecked() ? commentV2LineEdit->text()
		: QString::null;
	st->year    = yearV2CheckBox->isChecked()    ? yearV2SpinBox->value()
		: -1;
	st->track   = trackV2CheckBox->isChecked()   ? trackV2SpinBox->value()
		: -1;
	st->genre   = genreV2CheckBox->isChecked()   ?
		genreV2ComboBox->currentItem() - 1 : -1;
	st->genre   = genreV2CheckBox->isChecked()   ?
		Genres::getNumber(genreV2ComboBox->currentItem()) : -1;
}

/**
 * Set ID3v1 standard tags controls.
 *
 * @param st standard tags to set
 */
void id3Form::setStandardTagsV1(const StandardTags *st)
{
	titleV1CheckBox->setChecked(!st->title.isNull());
	titleV1LineEdit->setText(st->title);
	artistV1CheckBox->setChecked(!st->artist.isNull());
	artistV1LineEdit->setText(st->artist);
	albumV1CheckBox->setChecked(!st->album.isNull());
	albumV1LineEdit->setText(st->album);
	commentV1CheckBox->setChecked(!st->comment.isNull());
	commentV1LineEdit->setText(st->comment);
	yearV1CheckBox->setChecked(st->year >= 0);
	yearV1SpinBox->setValue(st->year >= 0 ? st->year : 0);
	trackV1CheckBox->setChecked(st->track >= 0);
	trackV1SpinBox->setValue(st->track >= 0 ? st->track : 0);
	genreV1CheckBox->setChecked(st->genre >= 0);
	genreV1ComboBox->setCurrentItem(st->genre >= 0 ?
					Genres::getIndex(st->genre) : 0);
}

/**
 * Set ID3v2 standard tags controls.
 *
 * @param st standard tags to set
 */
void id3Form::setStandardTagsV2(const StandardTags *st)
{
	titleV2CheckBox->setChecked(!st->title.isNull());
	titleV2LineEdit->setText(st->title);
	artistV2CheckBox->setChecked(!st->artist.isNull());
	artistV2LineEdit->setText(st->artist);
	albumV2CheckBox->setChecked(!st->album.isNull());
	albumV2LineEdit->setText(st->album);
	commentV2CheckBox->setChecked(!st->comment.isNull());
	commentV2LineEdit->setText(st->comment);
	yearV2CheckBox->setChecked(st->year >= 0);
	yearV2SpinBox->setValue(st->year >= 0 ? st->year : 0);
	trackV2CheckBox->setChecked(st->track >= 0);
	trackV2SpinBox->setValue(st->track >= 0 ? st->track : 0);
	genreV2CheckBox->setChecked(st->genre >= 0);
	genreV2ComboBox->setCurrentItem(st->genre >= 0 ?
					Genres::getIndex(st->genre) : 0);
}

/**
 * Set all ID3v1 and ID3v2 check boxes on or off.
 *
 * @param val TRUE to set check boxes on.
 */
void id3Form::setAllCheckBoxes(bool val)
{
	titleV1CheckBox->setChecked(val);
	artistV1CheckBox->setChecked(val);
	albumV1CheckBox->setChecked(val);
	commentV1CheckBox->setChecked(val);
	yearV1CheckBox->setChecked(val);
	trackV1CheckBox->setChecked(val);
	genreV1CheckBox->setChecked(val);
	
	titleV2CheckBox->setChecked(val);
	artistV2CheckBox->setChecked(val);
	albumV2CheckBox->setChecked(val);
	commentV2CheckBox->setChecked(val);
	yearV2CheckBox->setChecked(val);
	trackV2CheckBox->setChecked(val);
	genreV2CheckBox->setChecked(val);
}

/**
 * Get number of files selected in file list box.
 *
 * @return number of files selected.
 */
int id3Form::numFilesSelected()
{
	int i, num_files_selected = 0;
	for (i = 0; i < (int)mp3ListBox->count(); i++) {
		if (mp3ListBox->isSelected(i)) {
			++num_files_selected;
		}
	}
	return num_files_selected;
}

/**
 * Accept drag.
 *
 * @param ev drag event.
 */
void id3Form::dragEnterEvent(QDragEnterEvent *ev)
{
	ev->accept(QTextDrag::canDecode(ev));
}

/**
 * Handle drop event.
 *
 * @param ev drop event.
 */
void id3Form::dropEvent(QDropEvent *ev)
{
	QString text;
	if (QTextDrag::decode(ev, text)) {
		theApp->openDrop(text);
	}
}

/**
 * Frame list button Edit.
 */
void id3Form::editFrame(void)
{
	theApp->editFrame();
}

/**
 * Frame list button Add.
 */
void id3Form::addFrame(void)
{
	theApp->addFrame();
}

/**
 * Frame list button Delete.
 */
void id3Form::deleteFrame(void)
{
	theApp->deleteFrame();
}


/**
 * Called from constructor.
 * Make size adjustments.
 */
void id3Form::init()
{
	titleV1LineEdit->setMinimumWidth(fontMetrics().maxWidth() * 15);
	mp3ListBox->resize(fontMetrics().maxWidth() * 25, -1);
}


/**
 * Set filename according to ID3v1 tags.
 */

void id3Form::fnFromID3V1(void)
{
	theApp->getFilenameFromTags(1);
}

/**
 * Set filename according to ID3v1 tags.
 */

void id3Form::fnFromID3V2(void)
{
	theApp->getFilenameFromTags(2);
}
