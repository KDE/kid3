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
#include <QListWidget>
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
 */
LabeledTextEdit::LabeledTextEdit(QWidget* parent) :
    QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_edit = new QTextEdit(this);
	if (m_layout && m_label && m_edit) {
		m_edit->QCM_setTextFormat_PlainText();
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_edit);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledLineEdit::LabeledLineEdit(QWidget* parent) :
    QWidget(parent)
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
 * @param strlst list with ComboBox items, terminated by NULL
 */
LabeledComboBox::LabeledComboBox(QWidget* parent,
				 const char **strlst) : QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_label = new QLabel(this);
	m_combo = new QComboBox(this);
	if (m_layout && m_label && m_combo) {
		QStringList strList;
		while (*strlst) {
			strList += i18n(*strlst++);
		}
		m_combo->QCM_addItems(strList);
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_combo);
	}
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
LabeledSpinBox::LabeledSpinBox(QWidget* parent) :
    QWidget(parent)
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
 * @param img    image to display in window
 */
ImageViewer::ImageViewer(QWidget* parent, QImage* img) :
    QDialog(parent), m_image(img)
{
	setModal(true);
	setFixedSize(m_image->width(), m_image->height());
	QCM_setWindowTitle(i18n("View Picture"));
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
#if QT_VERSION >= 0x040000
QListWidget* FrameList::s_listbox = 0;
#else
QListBox* FrameList::s_listbox = 0;
#endif

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

/**
 * Reload the frame list, keeping the same row selected.
 */
void FrameList::reloadTags()
{
#if QT_VERSION >= 0x040000
	int selectedRow = s_listbox->currentRow();
	int topRow = s_listbox->row(s_listbox->itemAt(0, 0));
	setTags(m_file);
	if (topRow >= 0 && topRow < static_cast<int>(s_listbox->count())) {
		s_listbox->scrollToItem(s_listbox->item(topRow), QAbstractItemView::PositionAtTop);
	}
	if (selectedRow >= 0 && selectedRow < static_cast<int>(s_listbox->count())) {
		s_listbox->setCurrentRow(selectedRow);
	}
#else
	int selectedRow = -1;
	int topRow = s_listbox->topItem();
	for (int i = 0; i < static_cast<int>(s_listbox->count()); ++i) {
		if (s_listbox->isSelected(i)) {
			selectedRow = i;
			break;
		}
	}
	setTags(m_file);
	if (topRow >= 0 && topRow < static_cast<int>(s_listbox->count())) {
		s_listbox->setTopItem(topRow);
	}
	if (selectedRow >= 0 && selectedRow < static_cast<int>(s_listbox->count())) {
		s_listbox->setSelected(selectedRow, true);
	}
#endif
}

/**
 * Get ID of selected frame list item.
 *
 * @return ID of selected item,
 *         -1 if not item is selected.
 */
int FrameList::getSelectedId()
{
#if QT_VERSION >= 0x040000
	FrameListItem* fli;
	QList<QListWidgetItem*> items = s_listbox->selectedItems();
	return
		!items.empty() &&
		(fli = dynamic_cast<FrameListItem*>(items.front())) != 0 ? fli->getId() : -1;
#else
	QListBoxItem* lbi;
	FrameListItem* fli;
	return
		(lbi = s_listbox->selectedItem()) != 0 &&
		(fli = dynamic_cast<FrameListItem*>(lbi)) != 0 ? fli->getId() : -1;
#endif
}

/**
 * Select the frame by ID.
 *
 * @param id ID of frame to select
 */
void FrameList::setSelectedId(int id)
{
#if QT_VERSION >= 0x040000
	for (int i = 0; i < s_listbox->count(); ++i) {
		FrameListItem* fli = dynamic_cast<FrameListItem*>(s_listbox->item(i));
		if (fli && fli->getId() == id) {
			s_listbox->setCurrentRow(i);
			break;
		}
	}
#else
	QListBoxItem* lbi = s_listbox->firstItem();
	while (lbi) {
		FrameListItem* fli = dynamic_cast<FrameListItem*>(lbi);
		if (fli && fli->getId() == id) {
			s_listbox->setSelected(lbi, true);
			break;
		}
		lbi = lbi->next();
	}
#endif
}

/**
 * Get the name of the selected frame.
 *
 * @return name, QString::null if nothing selected.
 */
QString FrameList::getSelectedName()
{
#if QT_VERSION >= 0x040000
	QListWidgetItem* item;
	if (s_listbox &&
			(item = s_listbox->currentItem()) != 0) {
		return item->text();
	}
	return QString::null;
#else
	return s_listbox ? s_listbox->currentText() : QString::null;
#endif
}

/**
 * Select a frame with a given name.
 *
 * @param name name of frame
 *
 * @return true if a frame with that name could be selected.
 */
bool FrameList::selectByName(const QString& name)
{
	if (s_listbox) {
#if QT_VERSION >= 0x040000
		QList<QListWidgetItem*> items =
			s_listbox->findItems(name, Qt::MatchStartsWith);
		if (!items.empty()) {
			s_listbox->setCurrentItem(items.front());
			return true;
		}
#else
		QListBoxItem* lbi = s_listbox->findItem(name);
		if (lbi) {
			s_listbox->setSelected(lbi, true);
			return true;
		}
#endif
	}
	return false;
}

/**
 * Clear list box.
 */
void FrameList::clearListBox()
{
	if (s_listbox) {
		s_listbox->clear();
	}
}
