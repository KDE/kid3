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
#include <Q3ListBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
	EditOggFrameDialog(QWidget *parent, const QString& caption,
									const QString& text);

	/**
	 * Destructor.
	 */
	virtual ~EditOggFrameDialog();

	/**
	 * Set text to edit.
	 * @param text text
	 */
	void setText(const QString& text) { m_edit->setText(text); }

	/**
	 * Get edited text.
	 * @return text.
	 */
	QString getText() const { return m_edit->text(); }

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
	QDialog(parent, "edit_frame", true)
{
	setCaption(caption);
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (vlayout) {
		vlayout->setSpacing(6);
		vlayout->setMargin(6);
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setText(text);
			m_edit->moveCursor(QTextEdit::MoveEnd, false);
			vlayout->addWidget(m_edit);
		}
	}
	QHBoxLayout* hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
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
	}
#if QT_VERSION < 0x040000
	// the widget is not painted correctly after resizing in Qt4
	resize(fontMetrics().maxWidth() * 30, -1);
#endif
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
	listbox->clear();
	if (m_tags) {
		for (OggFile::CommentList::const_iterator it = m_tags->begin();
				 it != m_tags->end();
				 ++it) {
			listbox->insertItem((*it).getName());
		}
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
	EditOggFrameDialog *dialog =
		new EditOggFrameDialog(NULL, frame.getName(), frame.getValue());
	if (dialog && dialog->exec() == QDialog::Accepted) {
		frame.setValue(dialog->getText());
		if (m_file) {
			m_file->changedV2 = true;
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
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tags) {
		OggFile::CommentList::iterator it = m_tags->at(selectedIndex);
		return editFrame(*it);
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
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tags) {
		OggFile::CommentList::iterator it = m_tags->at(selectedIndex);
		m_tags->erase(it);
		readTags(); // refresh listbox
		// select the next item (or the last if it was the last)
		if (selectedIndex >= 0) {
			const int lastIndex = listbox->count() - 1;
			if (lastIndex >= 0) {
				listbox->setSelected(
					selectedIndex <= lastIndex ? selectedIndex : lastIndex, true);
			}
		}
		if (m_file) {
			m_file->changedV2 = true;
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
		readTags(); // refresh listbox
		const int lastIndex = listbox->count() - 1;
		if (lastIndex >= 0) {
			listbox->setSelected(lastIndex, true);
		}
		if (m_file) {
			m_file->changedV2 = true;
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
	QString res = QInputDialog::getItem(
		i18n("Add Frame"),
		i18n("Select the frame ID"), lst, 0, true, &ok);
	if (ok) {
		m_selectedName = res.stripWhiteSpace().upper();
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
	int selectedIndex = listbox->currentItem();
	if (selectedIndex != -1 && m_tags) {
		OggFile::CommentList::iterator it = m_tags->at(selectedIndex);
		if (it != m_tags->end()) {
			m_copyFrame = *it;
		}
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
			m_file->changedV2 = true;
		}
		return true;
	}
	return false;
}

#endif // HAVE_VORBIS || defined HAVE_FLAC
