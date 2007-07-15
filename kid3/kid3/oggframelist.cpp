/**
 * \file oggframelist.cpp
 * List of Ogg comment frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 28 Sep 2005
 */

#include "oggframelist.h"
#if defined HAVE_VORBIS || defined HAVE_FLAC

#include <qdialog.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <qstringlist.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextCursor>
#else
#include <qlistbox.h>
#endif

/** Field edit dialog */
class EditOggFrameDialog : public QDialog {
public:
 /**
	* Constructor.
	*
	* @param parent  parent widget
	* @param caption window title
	* @param text    text to edit
	*/
	EditOggFrameDialog(QWidget* parent, const QString& caption,
									const QString& text);

	/**
	 * Destructor.
	 */
	virtual ~EditOggFrameDialog();

	/**
	 * Set text to edit.
	 * @param text text
	 */
	void setText(const QString& text) {
		m_edit->QCM_setPlainText(text);
	}

	/**
	 * Get edited text.
	 * @return text.
	 */
	QString getText() const { return m_edit->QCM_toPlainText(); }

private:
	QTextEdit* m_edit;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;
};

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption window title
 * @param text    text to edit
 */
EditOggFrameDialog::EditOggFrameDialog(QWidget* parent, const QString& caption,
																 const QString& text) :
	QDialog(parent)
{
	setModal(true);
	QCM_setWindowTitle(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setSpacing(6);
		vlayout->setMargin(6);
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->QCM_setPlainText(text);
#if QT_VERSION >= 0x040200
			m_edit->moveCursor(QTextCursor::End);
#elif QT_VERSION >= 0x040000
			QTextCursor cursor = m_edit->textCursor();
			cursor.movePosition(QTextCursor::End);
			m_edit->setTextCursor(cursor);
#else
			m_edit->moveCursor(QTextEdit::MoveEnd, false);
#endif
			vlayout->addWidget(m_edit);
		}
	}
	QHBoxLayout* hlayout = new QHBoxLayout;
	QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
					   QSizePolicy::Minimum);
	m_okButton = new QPushButton(i18n("&OK"), this);
	m_cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && m_okButton && m_cancelButton) {
		hlayout->addItem(hspacer);
		hlayout->addWidget(m_okButton);
		hlayout->addWidget(m_cancelButton);
		m_okButton->setDefault(true);
		connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		vlayout->addLayout(hlayout);
	}
	resize(fontMetrics().maxWidth() * 30, -1);
}

/**
 * Destructor.
 */
EditOggFrameDialog::~EditOggFrameDialog() {
}


/**
 * Constructor.
 */
OggFrameList::OggFrameList() : m_tags(0)
{
}

/**
 * Destructor.
 */
OggFrameList::~OggFrameList()
{
}

/**
 * Fill listbox with frame descriptions.
 * Before using this method, the listbox and file have to be set.
 * @see setListBox(), setTags()
 */
void OggFrameList::readTags()
{
	s_listbox->clear();
	if (m_tags) {
		int i = 0;
		for (OggFile::CommentList::const_iterator it = m_tags->begin();
				 it != m_tags->end();
				 ++it) {
			new FrameListItem(s_listbox, (*it).getName(), i++);
		}
#if QT_VERSION >= 0x040000
		s_listbox->sortItems();
#else
		s_listbox->sort();
#endif
	}
}

/**
 * Set file and fill the list box with its frames.
 * The listbox has to be set with setListBox() before calling this
 * function.
 *
 * @param taggedFile file
 */
void OggFrameList::setTags(TaggedFile* taggedFile)
{
	m_file = taggedFile;
	OggFile* oggFile = dynamic_cast<OggFile*>(m_file);
	if (oggFile && oggFile->isTagInformationRead()) {
		m_tags = &oggFile->m_comments;
		readTags();
	}
}

/**
 * Create dialog to edit a frame and update the fields
 * if Ok is returned.
 *
 * @param frame frame to edit
 *
 * @return true if Ok selected in dialog.
 */
bool OggFrameList::editFrame(OggFile::CommentField& frame)
{
	EditOggFrameDialog* dialog =
		new EditOggFrameDialog(NULL, frame.getName(), frame.getValue());
	if (dialog && dialog->exec() == QDialog::Accepted) {
		frame.setValue(dialog->getText());
		if (m_file) {
			m_file->markTag2Changed();
		}
		return true;
	}
	return false;
}

/**
 * Create dialog to edit the selected frame and update the fields
 * if Ok is returned.
 *
 * @return true if Ok selected in dialog.
 */
bool OggFrameList::editFrame()
{
	int selectedId = getSelectedId();
	if (selectedId != -1 && m_tags) {
#if QT_VERSION >= 0x040000
		return editFrame((*m_tags)[selectedId]);
#else
		OggFile::CommentList::iterator it = m_tags->at(selectedId);
		return editFrame(*it);
#endif
	}
	return false;
}

/**
 * Delete selected frame.
 *
 * @return false if frame not found.
 */
bool OggFrameList::deleteFrame()
{
#if QT_VERSION >= 0x040000
	int selectedIndex = s_listbox->currentRow();
#else
	int selectedIndex = s_listbox->currentItem();
#endif
	int selectedId = getSelectedId();
	if (selectedId != -1 && m_tags) {
#if QT_VERSION >= 0x040000
		m_tags->removeAt(selectedId);
#else
		OggFile::CommentList::iterator it = m_tags->at(selectedId);
		m_tags->erase(it);
#endif
		readTags(); // refresh listbox
		// select the next item (or the last if it was the last)
		if (selectedIndex >= 0) {
			const int lastIndex = s_listbox->count() - 1;
			if (lastIndex >= 0) {
#if QT_VERSION >= 0x040000
				s_listbox->setCurrentRow(
					selectedIndex <= lastIndex ? selectedIndex : lastIndex);
#else
				s_listbox->setSelected(
					selectedIndex <= lastIndex ? selectedIndex : lastIndex, true);
				s_listbox->ensureCurrentVisible();
#endif
			}
		}
		if (m_file) {
			m_file->markTag2Changed();
		}
		return true;
	}
	return false;
}

/**
 * Add a new frame.
 *
 * @param frameId ID of frame to add, from selectFrameId()
 * @param edit    true to edit frame after adding it
 * @return true if frame added.
 */
bool OggFrameList::addFrame(int frameId, bool edit)
{
	if (frameId != 0) {
		return false;
	}
	if (m_tags) {
		OggFile::CommentField frame(m_selectedName, "");
		if (edit && !editFrame(frame)) {
			return false;
		}
		m_tags->push_back(frame);
		int frameIndex = m_tags->size() - 1;
		readTags(); // refresh listbox
		setSelectedId(frameIndex);
#if QT_VERSION < 0x040000
		s_listbox->ensureCurrentVisible();
#endif
		if (m_file) {
			m_file->markTag2Changed();
		}
		return true;
	}
	return false;
}

/**
 * Display a dialog to select a frame type.
 *
 * @return ID of selected frame, to be passed to addFrame(),
 *         -1 if no frame selected.
 */
int OggFrameList::selectFrameId()
{
	/** Alphabetically sorted list of frame descriptions */
	static const char* const fieldNames[] = {
		"ALBUM",
		"ARRANGER",
		"ARTIST",
		"AUTHOR",
		"CATALOGNUMBER",
		"COMMENT",
		"COMPOSER",
		"CONDUCTOR",
		"CONTACT",
		"COPYRIGHT",
		"DATE",
		"DESCRIPTION",
		"DISCID",
		"DISCNUMBER",
		"EAN/UPN",
		"ENCODED-BY",
		"ENCODING",
		"ENGINEER",
		"ENSEMBLE",
		"GENRE",
		"GUEST ARTIST",
		"ISRC",
		"LABEL",
		"LABELNO",
		"LICENSE",
		"LOCATION",
		"LYRICIST",
		"OPUS",
		"ORGANIZATION",
		"PART",
		"PARTNUMBER",
		"PERFORMER",
		"PRODUCER",
		"PRODUCTNUMBER",
		"PUBLISHER",
		"RELEASE DATE",
		"REMIXER",
		"SOURCE ARTIST",
		"SOURCE MEDIUM",
		"SOURCE WORK",
		"SOURCEMEDIA",
		"SPARS",
		"SUBTITLE",
		"TITLE",
		"TRACKNUMBER",
		"TRACKTOTAL",
		"VERSION",
		"VOLUME",
		"" // user comment
	};

	QStringList lst;
	for (unsigned i = 0; i < sizeof(fieldNames) / sizeof(fieldNames[0]); ++i) {
		lst.append(fieldNames[i]);
	}
	bool ok = false;
	QString res = QInputDialog::QCM_getItem(
		0, i18n("Add Frame"),
		i18n("Select the frame ID"), lst, 0, true, &ok);
	if (ok) {
		m_selectedName = res.QCM_trimmed().QCM_toUpper();
		return 0; // just used by addFrame()
	}
	return -1;
}

/**
 * Copy the selected frame to the copy buffer.
 *
 * @return true if frame copied.
 */
bool OggFrameList::copyFrame() {
	int selectedId = getSelectedId();
	if (selectedId != -1 && m_tags) {
#if QT_VERSION >= 0x040000
		if (selectedId < m_tags->size()) {
			m_copyFrame = m_tags->at(selectedId);
		}
#else
		OggFile::CommentList::iterator it = m_tags->at(selectedId);
		if (it != m_tags->end()) {
			m_copyFrame = *it;
		}
#endif
		return true;
	}
	return false;
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool OggFrameList::pasteFrame() {
	if (!(m_copyFrame.getName().isEmpty() && m_copyFrame.getValue().isEmpty()) &&
			m_tags) {
		m_tags->push_back(m_copyFrame);
		if (m_file) {
			m_file->markTag2Changed();
		}
		return true;
	}
	return false;
}

#endif // HAVE_VORBIS || defined HAVE_FLAC
