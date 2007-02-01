/**
 * \file framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

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
LabeledTextEdit::LabeledTextEdit(QWidget* parent, const char* name) :
    QWidget(parent, name)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_edit = new QTextEdit(this);
	if (m_layout && m_label && m_edit) {
		m_edit->setTextFormat(Qt::PlainText);
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 */
LabeledLineEdit::LabeledLineEdit(QWidget* parent, const char* name) :
    QWidget(parent, name)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_edit = new QLineEdit(this);
	if (m_layout && m_label && m_edit) {
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 * @param strlst list with ComboBox items, terminated by NULL
 */
LabeledComboBox::LabeledComboBox(QWidget* parent, const char* name,
				 const char **strlst) : QWidget(parent, name)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_combo = new QComboBox(this);
	if (m_layout && m_label && m_combo) {
		while (*strlst) {
			m_combo->insertItem(i18n(*strlst++));
		}
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_combo);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 */
LabeledSpinBox::LabeledSpinBox(QWidget* parent, const char* name) :
    QWidget(parent, name)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_spinbox = new QSpinBox(this);
	if (m_layout && m_label && m_spinbox) {
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_spinbox);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 * @param img    image to display in window
 */
ImageViewer::ImageViewer(QWidget* parent, const char* name, QImage* img) :
    QDialog(parent, name, true), m_image(img)
{
	setFixedSize(m_image->width(), m_image->height());
	setCaption(i18n("View Picture"));
}

/**
 * Paint image, called when window has to be drawn.
 */
void ImageViewer::paintEvent(QPaintEvent*)
{
	QPainter paint(this);
	paint.drawImage(0, 0, *m_image, 0, 0, m_image->width(), m_image->height());
}


/** List box to select frame */
Q3ListBox* FrameList::s_listbox = 0;

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
void FrameList::clear()
{
	s_listbox->clear();
	m_file = 0;
}

/**
 * Get file containing frames.
 *
 * @return file, NULL if no file selected.
 */
TaggedFile* FrameList::getFile() const
{
	return m_file;
}
