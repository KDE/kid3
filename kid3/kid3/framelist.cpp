/**
 * \file framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qimage.h>
#include <qpainter.h>
#include <qcombobox.h>
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#else
#include <qlistbox.h>
#endif

#include "taggedfile.h"
#include "framelist.h"


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 */

LabeledTextEdit::LabeledTextEdit(QWidget *parent, const char *name) :
    QWidget(parent, name)
{
	layout = new QVBoxLayout(this);
	label = new QLabel(this);
	edit = new QTextEdit(this);
	if (layout && label && edit) {
		edit->setTextFormat(Qt::PlainText);
		layout->addWidget(label);
		layout->addWidget(edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 */

LabeledLineEdit::LabeledLineEdit(QWidget *parent, const char *name) :
    QWidget(parent, name)
{
	layout = new QVBoxLayout(this);
	label = new QLabel(this);
	edit = new QLineEdit(this);
	if (layout && label && edit) {
		layout->addWidget(label);
		layout->addWidget(edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 * @param strlst list with ComboBox items, terminated by NULL
 */

LabeledComboBox::LabeledComboBox(QWidget *parent, const char *name,
				 const char **strlst) : QWidget(parent, name)
{
	layout = new QVBoxLayout(this);
	label = new QLabel(this);
	combo = new QComboBox(this);
	if (layout && label && combo) {
//		combo->insertStrList(strlst);
		while (*strlst) {
			combo->insertItem(i18n(*strlst++));
		}
		layout->addWidget(label);
		layout->addWidget(combo);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 */

LabeledSpinBox::LabeledSpinBox(QWidget *parent, const char *name) :
    QWidget(parent, name)
{
	layout = new QVBoxLayout(this);
	label = new QLabel(this);
	spinbox = new QSpinBox(this);
	if (layout && label && spinbox) {
		layout->addWidget(label);
		layout->addWidget(spinbox);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 * @param img    image to display in window
 */

ImageViewer::ImageViewer(QWidget *parent, const char *name, QImage *img) :
    QDialog(parent, name, TRUE), image(img)
{
	setFixedSize(image->width(), image->height());
	setCaption(i18n("View Picture"));
}

/**
 * Paint image, called when window has to be drawn.
 */

void ImageViewer::paintEvent(QPaintEvent *)
{
	QPainter paint(this);
	paint.drawImage(0, 0, *image, 0, 0, image->width(), image->height());
}


/** List box to select frame */
Q3ListBox* FrameList::listbox = 0;

/**
 * Constructor.
 */
FrameList::FrameList() : m_file(0) {}

/**
 * Destructor.
 */
FrameList::~FrameList() {}

/**
 * Clear listbox and file reference.
 */
void FrameList::clear(void)
{
	listbox->clear();
	m_file = 0;
}

/**
 * Get file containing frames.
 *
 * @return file, NULL if no file selected.
 */
TaggedFile* FrameList::getFile(void) const
{
	return m_file;
}
