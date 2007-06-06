/**
 * \file mp3framelist.cpp
 * List of ID3v2.3 frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "mp3framelist.h"
#ifdef HAVE_ID3LIB

#ifdef CONFIG_USE_KDE
#include <kdialogbase.h>
#include <kfiledialog.h>
#else
#include <qdialog.h>
#endif

#include <qfile.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qfiledialog.h>
#if QT_VERSION >= 0x040000
#include <Q3ListBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#else
#include <qlistbox.h>
#endif
#ifdef WIN32
#include <id3.h>
#endif

#include "mp3file.h"


/** Base class for MP3 field controls */
class Mp3FieldControl : public FieldControl {
public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	Mp3FieldControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld) :
		m_frmLst(fl), m_fieldId(id), m_field(fld) {}

	/**
	 * Destructor.
	 */
	virtual ~Mp3FieldControl() {}

protected:
	/**
	 * Get description for ID3_Field.
	 *
	 * @param id ID of field
	 * @return description or NULL if id unknown.
	 */
	const char* getFieldIDString(ID3_FieldID id) const;

	/** Frame list */
	Mp3FrameList* m_frmLst;
	/** Field ID */
	ID3_FieldID m_fieldId;
	/** ID3 field */
	ID3_Field* m_field;
};

/** Control to edit standard UTF text fields */
class TextFieldControl : public Mp3FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	TextFieldControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld) :
	    Mp3FieldControl(fl, id, fld) {}

	/**
	 * Destructor.
	 */
	virtual ~TextFieldControl() {}
	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();
	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

 protected:
	/** Text editor widget */
	LabeledTextEdit* m_edit;
};

/** Control to edit single line text fields */
class LineFieldControl : public Mp3FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	LineFieldControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld) :
	    Mp3FieldControl(fl, id, fld) {}

	/**
	 * Destructor.
	 */
	virtual ~LineFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

 protected:
	/** Line editor widget */
	LabeledLineEdit* m_edit;
};

/** Control to edit integer fields */
class IntFieldControl : public Mp3FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	IntFieldControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld) :
	    Mp3FieldControl(fl, id, fld) {};

	/**
	 * Destructor.
	 */
	virtual ~IntFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

 protected:
	/** Spin box widget */
	LabeledSpinBox* m_numInp;
};

/** Control to edit integer fields using a combo box with given values */
class IntComboBoxControl : public Mp3FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 * @param lst list of strings with possible selections, NULL terminated
	 */
	IntComboBoxControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld,
										 const char **lst) :
	    Mp3FieldControl(fl, id, fld), m_strLst(lst) {};

	/**
	 * Destructor.
	 */
	virtual ~IntComboBoxControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

 protected:
	/** Combo box widget */
	LabeledComboBox* m_ptInp;
	/** List of strings with possible selections */
	const char** m_strLst;
};

/** Control to import, export and view data from binary fields */
class BinFieldControl : public Mp3FieldControl {
 public:
	/**
	 * Constructor.
	 *
	 * @param fl  frame list
	 * @param id  ID of field
	 * @param fld ID3 field
	 */
	BinFieldControl(Mp3FrameList* fl, ID3_FieldID id, ID3_Field* fld) :
	    Mp3FieldControl(fl, id, fld) {};

	/**
	 * Destructor.
	 */
	virtual ~BinFieldControl() {}

	/**
	 * Update field from data in field control.
	 */
	virtual void updateTag();

	/**
	 * Create widget to edit field data.
	 *
	 * @param parent parent widget
	 *
	 * @return widget to edit field data.
	 */
	virtual QWidget* createWidget(QWidget* parent);

 protected:
	/** Import, Export, View buttons */
	BinaryOpenSave* m_bos;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param name   internal name or 0
 * @param fld    ID3_Field containing binary data
 */
BinaryOpenSave::BinaryOpenSave(QWidget* parent, const char* name,
			       ID3_Field* fld) :
	QWidget(parent, name), m_isChanged(false)
{
	m_byteArray.duplicate(
		(const char *)fld->GetRawBinary(), (unsigned int)fld->Size());
	m_layout = new QHBoxLayout(this);
	m_label = new QLabel(this);
	m_openButton = new QPushButton(i18n("&Import"), this);
	m_saveButton = new QPushButton(i18n("&Export"), this);
	m_viewButton = new QPushButton(i18n("&View"), this);
	if (m_layout && m_label && m_openButton && m_saveButton && m_viewButton) {
		m_layout->addWidget(m_label);
		m_layout->addWidget(m_openButton);
		m_layout->addWidget(m_saveButton);
		m_layout->addWidget(m_viewButton);
		connect(m_openButton, SIGNAL(clicked()), this, SLOT(loadData()));
		connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveData()));
		connect(m_viewButton, SIGNAL(clicked()), this, SLOT(viewData()));
	}
}

/**
 * Request name of file to import binary data from.
 * The data is imported later when Ok is pressed in the parent dialog.
 */
void BinaryOpenSave::loadData()
{
#ifdef CONFIG_USE_KDE
	QString loadfilename = KFileDialog::getOpenFileName(
		QString::null, QString::null, this);
#else
	QString loadfilename = QFileDialog::getOpenFileName(
		QString::null, QString::null, this);
#endif
	if (!loadfilename.isEmpty()) {
		QFile file(loadfilename);
		if (file.open(QCM_ReadOnly)) {
			size_t size = file.size();
			char* data = new char[size];
			if (data) {
				QDataStream stream(&file);
				stream.readRawBytes(data, size);
				m_byteArray.duplicate(data, size);
				m_isChanged = true;
				delete [] data;
			}
			file.close();
		}
	}
}

/**
 * Request name of file and export binary data.
 */
void BinaryOpenSave::saveData()
{
#ifdef CONFIG_USE_KDE
	QString fn = KFileDialog::getSaveFileName(QString::null, QString::null,
																						this);
#else
	QString fn = QFileDialog::getSaveFileName(QString::null, QString::null,
																						this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_WriteOnly)) {
			QDataStream stream(&file);
			stream.writeRawBytes(m_byteArray.data(), m_byteArray.size());
			file.close();
		}
	}
}

/**
 * Create image from binary data and display it in window.
 */
void BinaryOpenSave::viewData()
{
	QImage image;
	if (image.loadFromData(m_byteArray)) {
		ImageViewer iv(this, 0, &image);
		iv.exec();
	}
}

/**
 * Get description for ID3_Field.
 *
 * @param id ID of field
 * @return description or NULL if id unknown.
 */
const char* Mp3FieldControl::getFieldIDString(ID3_FieldID id) const
{
	static const struct id_str_s { ID3_FieldID id; const char* str; }
	id_str[] = {
		{ID3FN_TEXTENC,        I18N_NOOP("Text Encoding")},
		{ID3FN_TEXT,           I18N_NOOP("Text")},
		{ID3FN_URL,            I18N_NOOP("URL")},
		{ID3FN_DATA,           I18N_NOOP("Data")},
		{ID3FN_DESCRIPTION,    I18N_NOOP("Description")},
		{ID3FN_OWNER,          I18N_NOOP("Owner")},
		{ID3FN_EMAIL,          I18N_NOOP("Email")},
		{ID3FN_RATING,         I18N_NOOP("Rating")},
		{ID3FN_FILENAME,       I18N_NOOP("Filename")},
		{ID3FN_LANGUAGE,       I18N_NOOP("Language")},
		{ID3FN_PICTURETYPE,    I18N_NOOP("Picture Type")},
		{ID3FN_IMAGEFORMAT,    I18N_NOOP("Image format")},
		{ID3FN_MIMETYPE,       I18N_NOOP("Mimetype")},
		{ID3FN_COUNTER,        I18N_NOOP("Counter")},
		{ID3FN_ID,             I18N_NOOP("Identifier")},
		{ID3FN_VOLUMEADJ,      I18N_NOOP("Volume Adjustment")},
		{ID3FN_NUMBITS,        I18N_NOOP("Number of Bits")},
		{ID3FN_VOLCHGRIGHT,    I18N_NOOP("Volume Change Right")},
		{ID3FN_VOLCHGLEFT,     I18N_NOOP("Volume Change Left")},
		{ID3FN_PEAKVOLRIGHT,   I18N_NOOP("Peak Volume Right")},
		{ID3FN_PEAKVOLLEFT,    I18N_NOOP("Peak Volume Left")},
		{ID3FN_TIMESTAMPFORMAT,I18N_NOOP("Timestamp Format")},
		{ID3FN_CONTENTTYPE,    I18N_NOOP("Content Type")},
		{ID3FN_LASTFIELDID,    NULL}
	};

	const struct id_str_s* is = &id_str[0];
	while (is->str) {
		if (is->id == id) {
			break;
		}
		++is;
	}
	return is->str;
}

/**
 * Update field with data from dialog.
 */
void TextFieldControl::updateTag()
{
	// get encoding from selection
	ID3_TextEnc enc = m_frmLst->getSelectedEncoding();
	if (enc != ID3TE_NONE) {
		m_field->SetEncoding(enc);
	}
	Mp3File::setString(m_field, m_edit->text());
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* TextFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledTextEdit(parent);
	if (m_edit == NULL)
		return NULL;

	m_edit->setLabel(i18n(getFieldIDString(m_fieldId)));
	m_edit->setText(Mp3File::getString(m_field));
	return m_edit;
}

/**
 * Update field with data from dialog.
 */
void LineFieldControl::updateTag()
{
	m_field->Set(m_edit->text().latin1());
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* LineFieldControl::createWidget(QWidget* parent)
{
	m_edit = new LabeledLineEdit(parent);
	if (m_edit) {
		m_edit->setLabel(i18n(getFieldIDString(m_fieldId)));
		m_edit->setText(m_field->GetRawText());
	}
	return m_edit;
}

/**
 * Update field with data from dialog.
 */
void IntFieldControl::updateTag()
{
	m_field->Set(m_numInp->value());
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntFieldControl::createWidget(QWidget* parent)
{
	m_numInp = new LabeledSpinBox(parent);
	if (m_numInp) {
		m_numInp->setLabel(i18n(getFieldIDString(m_fieldId)));
		m_numInp->setValue(m_field->Get());
	}
	return m_numInp;
}

/**
 * Update field with data from dialog.
 */
void IntComboBoxControl::updateTag()
{
	m_field->Set(m_ptInp->currentItem());
	/* If this is the selected encoding, store it to be used by text fields */
	if (m_field->GetID() == ID3FN_TEXTENC) {
		m_frmLst->setSelectedEncoding((ID3_TextEnc)m_ptInp->currentItem());
	}
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* IntComboBoxControl::createWidget(QWidget* parent)
{
	m_ptInp = new LabeledComboBox(parent, 0, m_strLst);
	if (m_ptInp) {
		m_ptInp->setLabel(i18n(getFieldIDString(m_fieldId)));
		m_ptInp->setCurrentItem(m_field->Get());
	}
	return m_ptInp;
}

/**
 * Update field with data from dialog.
 */
void BinFieldControl::updateTag()
{
	if (m_bos && m_bos->isChanged()) {
		const QByteArray& ba = m_bos->getData();
		m_field->Set(reinterpret_cast<const unsigned char*>(ba.data()), ba.size());
	}
}

/**
 * Create widget for dialog.
 *
 * @param parent parent widget
 * @return widget to edit field.
 */
QWidget* BinFieldControl::createWidget(QWidget* parent)
{
	m_bos = new BinaryOpenSave(parent, 0, m_field);
	if (m_bos) {
		m_bos->setLabel(i18n(getFieldIDString(m_fieldId)));
	}
	return m_bos;
}

#ifdef CONFIG_USE_KDE
/** Field edit dialog */
class EditMp3FrameDialog : public KDialogBase { /* KDE */
public:
	EditMp3FrameDialog(QWidget* parent, QString& caption,
			Q3PtrList<FieldControl> &ctls);
};

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption window title
 * @param ctls    list with controls to edit fields
 */
EditMp3FrameDialog::EditMp3FrameDialog(QWidget* parent, QString& caption,
 Q3PtrList<FieldControl> &ctls) :
	KDialogBase(parent, "edit_frame", true, caption, Ok|Cancel, Ok)
{
	QWidget* page = new QWidget(this);
	if (page) {
		setMainWidget(page);
		QVBoxLayout* vb = new QVBoxLayout(page);
		if (vb) {
			vb->setSpacing(6);
			vb->setMargin(6);
			FieldControl* fld_ctl = ctls.first();
			while (fld_ctl != NULL) {
				vb->addWidget(fld_ctl->createWidget(page));
				fld_ctl = ctls.next();
			}
		}
	}
#if QT_VERSION < 0x040000
	// the widget is not painted correctly after resizing in Qt4
	resize(fontMetrics().maxWidth() * 30, -1);
#endif
}

#else

/** Field edit dialog */
class EditMp3FrameDialog : public QDialog {
public:
	EditMp3FrameDialog(QWidget* parent, QString& caption,
			Q3PtrList<FieldControl> &ctls);
protected:
	QVBoxLayout* m_vlayout;      /**< vertical layout */
	QHBoxLayout* m_hlayout;      /**< horizontal layout */
	QSpacerItem* m_hspacer;      /**< horizontal spacer */
	QPushButton* m_okButton;     /**< OK button */
	QPushButton* m_cancelButton; /**< Cancel button */
};

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption window title
 * @param ctls    list with controls to edit fields
 */
EditMp3FrameDialog::EditMp3FrameDialog(QWidget* parent, QString& caption,
 Q3PtrList<FieldControl> &ctls) :
	QDialog(parent, "edit_frame", true)
{
	setCaption(caption);
	m_vlayout = new QVBoxLayout(this);
	if (m_vlayout) {
		m_vlayout->setSpacing(6);
		m_vlayout->setMargin(6);
		FieldControl* fld_ctl = ctls.first();
		while (fld_ctl != NULL) {
			m_vlayout->addWidget(fld_ctl->createWidget(this));
			fld_ctl = ctls.next();
		}
	}
	m_hlayout = new QHBoxLayout(m_vlayout);
	QSpacerItem* m_hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
					   QSizePolicy::Minimum);
	m_okButton = new QPushButton(i18n("&OK"), this);
	m_cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (m_hlayout && m_okButton && m_cancelButton) {
		m_hlayout->addItem(m_hspacer);
		m_hlayout->addWidget(m_okButton);
		m_hlayout->addWidget(m_cancelButton);
		m_okButton->setDefault(true);
		connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	}
#if QT_VERSION < 0x040000
	// the widget is not painted correctly after resizing in Qt4
	resize(fontMetrics().maxWidth() * 30, -1);
#endif
}
#endif

/**
 * Constructor.
 */
Mp3FrameList::Mp3FrameList() : m_tags(0), m_selectedEnc(ID3TE_NONE),
															 m_copyFrame(0)
{
	m_fieldcontrols.setAutoDelete(true);
}

/**
 * Destructor.
 */
Mp3FrameList::~Mp3FrameList()
{
	if (m_copyFrame) {
		delete m_copyFrame;
	}
}

/**
 * Fill listbox with frame descriptions.
 * Before using this method, the listbox and file have to be set.
 * @see setListBox(), setTags()
 */
void Mp3FrameList::readTags()
{
	s_listbox->clear();
	if (m_tags) {
		ID3_Tag::Iterator* iter = m_tags->CreateIterator();
		ID3_Frame* frame;
		int i = 0;
		while ((frame = iter->GetNext()) != NULL) {
			const char* idstr = getIdString(frame->GetID());
			new FrameListItem(s_listbox, idstr ? i18n(idstr) : QString(frame->GetTextID()), i++);
		}
#ifdef WIN32
		/* allocated in Windows DLL => must be freed in the same DLL */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
		s_listbox->sort();
	}
}

/**
 * Set file and fill the list box with its frames.
 * The listbox has to be set with setListBox() before calling this
 * function.
 *
 * @param taggedFile file
 */
void Mp3FrameList::setTags(TaggedFile* taggedFile)
{
	m_file = taggedFile;
	Mp3File* mp3File = dynamic_cast<Mp3File*>(m_file);
	if (mp3File) {
		m_tags = mp3File->m_tagV2;
		readTags();
	}
}

/**
 * Get frame with index.
 *
 * @param index index in listbox
 * @return frame with index.
 */
ID3_Frame* Mp3FrameList::getFrame(int index) const
{
	ID3_Frame* frame = NULL;
	if (m_tags) {
		int i;
		ID3_Tag::Iterator* iter = m_tags->CreateIterator();
		for (i = 0;
		     i <= index && ((frame = iter->GetNext()) != NULL);
		     i++);
#ifdef WIN32
		/* allocated in Windows DLL => must be freed in the same DLL */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
	}
	return frame;
}

/**
 * Get frame which is selected in listbox.
 *
 * @param lbIndex if not 0, the listbox index of the selected frame
 *                (-1 if none selected) is returned here
 *
 * @return selected frame.
 */
ID3_Frame* Mp3FrameList::getSelectedFrame(int* lbIndex) const
{
	ID3_Frame* frame = NULL;
	int selectedId = getSelectedId();
	if (m_tags) {
		int i;
		ID3_Tag::Iterator* iter = m_tags->CreateIterator();
		for (i = 0;
		     i < (int)s_listbox->count() &&
			 ((frame = iter->GetNext()) != NULL);
		     i++) {
			if (i == selectedId) {
				break;
			}
			else {
				frame = NULL;
			}
		}
#ifdef WIN32
		/* allocated in Windows DLL => must be freed in the same DLL */
		ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
		delete iter;
#endif
	}
	if (lbIndex) {
		*lbIndex = s_listbox->currentItem();
	}
	return frame;
}

/**
 * Create dialog to edit a frame and update the fields if Ok is
 * returned.
 *
 * @param frame frame to edit
 *
 * @return true if Ok selected in dialog.
 */
bool Mp3FrameList::editFrame(ID3_Frame* frame)
{
	bool result = false;
	ID3_Frame::Iterator* iter = frame->CreateIterator();
	ID3_Field* field;
	while ((field = iter->GetNext()) != NULL) {
		ID3_FieldID id = field->GetID();
		ID3_FieldType type = field->GetType();
		if (type == ID3FTY_INTEGER) {
			if (id == ID3FN_TEXTENC) {
				static const char* strlst[] = {
					I18N_NOOP("ISO-8859-1"),
					I18N_NOOP("Unicode"),
					I18N_NOOP("UTF16BE"),
					I18N_NOOP("UTF8"),
					NULL
				};
				IntComboBoxControl* cbox =
					new IntComboBoxControl(this, id, field, strlst);
				if (cbox) {
					m_fieldcontrols.append(cbox);
				}
			}
			else if (id == ID3FN_PICTURETYPE) {
				static const char* strlst[] = {
					I18N_NOOP("Other"),
					I18N_NOOP("32x32 pixels PNG file icon"),
					I18N_NOOP("Other file icon"),
					I18N_NOOP("Cover (front)"),
					I18N_NOOP("Cover (back)"),
					I18N_NOOP("Leaflet page"),
					I18N_NOOP("Media"),
					I18N_NOOP("Lead artist/lead performer/soloist"),
					I18N_NOOP("Artist/performer"),
					I18N_NOOP("Conductor"),
					I18N_NOOP("Band/Orchestra"),
					I18N_NOOP("Composer"),
					I18N_NOOP("Lyricist/text writer"),
					I18N_NOOP("Recording Location"),
					I18N_NOOP("During recording"),
					I18N_NOOP("During performance"),
					I18N_NOOP("Movie/video screen capture"),
					I18N_NOOP("A bright coloured fish"),
					I18N_NOOP("Illustration"),
					I18N_NOOP("Band/artist logotype"),
					I18N_NOOP("Publisher/Studio logotype"),
					NULL
				};
				IntComboBoxControl* cbox =
					new IntComboBoxControl(this, id, field, strlst);
				if (cbox) {
					m_fieldcontrols.append(cbox);
				}
			}
			else if (id == ID3FN_TIMESTAMPFORMAT) {
				static const char* strlst[] = {
					I18N_NOOP("Other"),
					I18N_NOOP("MPEG frames as unit"),
					I18N_NOOP("Milliseconds as unit"),
					NULL
				};
				IntComboBoxControl* cbox =
					new IntComboBoxControl(this, id, field, strlst);
				if (cbox) {
					m_fieldcontrols.append(cbox);
				}
			}
			else if (id == ID3FN_CONTENTTYPE) {
				static const char* strlst[] = {
					I18N_NOOP("Other"),
					I18N_NOOP("Lyrics"),
					I18N_NOOP("Text transcription"),
					I18N_NOOP("Movement/part name"),
					I18N_NOOP("Events"),
					I18N_NOOP("Chord"),
					I18N_NOOP("Trivia/pop up"),
					NULL
				};
				IntComboBoxControl* cbox =
					new IntComboBoxControl(this, id, field, strlst);
				if (cbox) {
					m_fieldcontrols.append(cbox);
				}
			}
			else {
				IntFieldControl* intctl =
					new IntFieldControl(this, id, field);
				if (intctl) {
					m_fieldcontrols.append(intctl);
				}
			}
		}
		else if (type == ID3FTY_BINARY) {
			BinFieldControl* binctl =
				new BinFieldControl(this, id, field);
			if (binctl) {
				m_fieldcontrols.append(binctl);
			}
		}
		else if (type == ID3FTY_TEXTSTRING) {
			ID3_TextEnc enc = field->GetEncoding();
			if (id == ID3FN_TEXT ||
					// (ID3TE_IS_DOUBLE_BYTE_ENC(enc))
					enc == ID3TE_UTF16 || enc == ID3TE_UTF16BE) {
				// Large textedit for text fields
				TextFieldControl* textctl =
					new TextFieldControl(this, id, field);
				if (textctl) {
					m_fieldcontrols.append(textctl);
				}
			}
			else {
				LineFieldControl* textctl =
					new LineFieldControl(this, id, field);
				if (textctl) {
					m_fieldcontrols.append(textctl);
				}
			}
		}
	}
#ifdef WIN32
	/* allocated in Windows DLL => must be freed in the same DLL */
	ID3TagIterator_Delete(reinterpret_cast<ID3TagIterator*>(iter));
#else
	delete iter;
#endif
	const char* idstr = getIdString(frame->GetID());
	QString caption = idstr ? i18n(idstr) : QString(frame->GetTextID());
	EditMp3FrameDialog* dialog =
		new EditMp3FrameDialog(NULL, caption, m_fieldcontrols);
	if (dialog && dialog->exec() == QDialog::Accepted) {
		FieldControl* fld_ctl = m_fieldcontrols.first();
		// will be set if there is an encoding selector
		setSelectedEncoding(ID3TE_NONE);
		while (fld_ctl != NULL) {
			fld_ctl->updateTag();
			fld_ctl = m_fieldcontrols.next();
		}
		if (m_file) {
			m_file->markTag2Changed();
		}
		result = true;
	}
	m_fieldcontrols.clear();
	return result;
}

/**
 * Create dialog to edit the selected frame and update the fields if Ok is
 * returned.
 *
 * @return true if Ok selected in dialog.
 */
bool Mp3FrameList::editFrame()
{
	ID3_Frame* frame = getSelectedFrame();
	if (frame) {
		return editFrame(frame);
	}
	return false;
}

/**
 * Delete selected frame.
 *
 * @return false if frame not found.
 */
bool Mp3FrameList::deleteFrame()
{
	int selectedIndex;
	ID3_Frame* frame = getSelectedFrame(&selectedIndex);
	if (frame) {
		if (m_tags) {
			m_tags->RemoveFrame(frame);
			readTags(); // refresh listbox
			// select the next item (or the last if it was the last)
			if (selectedIndex >= 0) {
				const int lastIndex = s_listbox->count() - 1;
				if (lastIndex >= 0) {
					s_listbox->setSelected(
						selectedIndex <= lastIndex ? selectedIndex : lastIndex, true);
					s_listbox->ensureCurrentVisible();
				}
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
 * @param frameId ID of frame to add
 * @param edit    true to edit frame after adding it
 * @return true if frame added.
 */
bool Mp3FrameList::addFrame(int frameId, bool edit)
{
	if (frameId < 0 || frameId > ID3FID_LASTFRAMEID) {
		return false;
	}
	ID3_FrameID id = static_cast<ID3_FrameID>(frameId);
	if (id == ID3FID_METACOMPRESSION || id == ID3FID_METACRYPTO) {
		// these two do not seem to work
		return false;
	}
	ID3_Frame* frame = new ID3_Frame(id);
	if (frame) {
		if (m_tags) {
			if (edit && !editFrame(frame)) {
				delete frame;
				return false;
			}
			m_tags->AttachFrame(frame);
			readTags(); // refresh listbox
			const int lastIndex = s_listbox->count() - 1;
			if (lastIndex >= 0) {
				setSelectedId(lastIndex);
				s_listbox->ensureCurrentVisible();
			}
			if (m_file) {
				m_file->markTag2Changed();
			}
			return true;
		}
		delete frame;
	}
	return false;
}

/** Alphabetically sorted list of frame descriptions */
const char* Mp3FrameList::s_frameIdStr[NumFrameIds] = {
	I18N_NOOP("AENC - Audio encryption"),
	I18N_NOOP("APIC - Attached picture"),
	I18N_NOOP("COMM - Comments"),
	I18N_NOOP("COMR - Commercial"),
	I18N_NOOP("ENCR - Encryption method registration"),
	I18N_NOOP("EQUA - Equalization"),
	I18N_NOOP("ETCO - Event timing codes"),
	I18N_NOOP("GEOB - General encapsulated object"),
	I18N_NOOP("GRID - Group identification registration"),
	I18N_NOOP("IPLS - Involved people list"),
	I18N_NOOP("LINK - Linked information"),
	I18N_NOOP("MCDI - Music CD identifier"),
	I18N_NOOP("MLLT - MPEG location lookup table"),
	I18N_NOOP("OWNE - Ownership frame"),
	I18N_NOOP("PRIV - Private frame"),
	I18N_NOOP("PCNT - Play counter"),
	I18N_NOOP("POPM - Popularimeter"),
	I18N_NOOP("POSS - Position synchronisation frame"),
	I18N_NOOP("RBUF - Recommended buffer size"),
	I18N_NOOP("RVAD - Relative volume adjustment"),
	I18N_NOOP("RVRB - Reverb"),
	I18N_NOOP("SYLT - Synchronized lyric/text"),
	I18N_NOOP("SYTC - Synchronized tempo codes"),
	I18N_NOOP("TALB - Album/Movie/Show title"),
	I18N_NOOP("TBPM - BPM (beats per minute)"),
	I18N_NOOP("TCOM - Composer"),
	I18N_NOOP("TCON - Content type"),
	I18N_NOOP("TCOP - Copyright message"),
	I18N_NOOP("TDAT - Date"),
	I18N_NOOP("TDLY - Playlist delay"),
	I18N_NOOP("TENC - Encoded by"),
	I18N_NOOP("TEXT - Lyricist/Text writer"),
	I18N_NOOP("TFLT - File type"),
	I18N_NOOP("TIME - Time"),
	I18N_NOOP("TIT1 - Content group description"),
	I18N_NOOP("TIT2 - Title/songname/content description"),
	I18N_NOOP("TIT3 - Subtitle/Description refinement"),
	I18N_NOOP("TKEY - Initial key"),
	I18N_NOOP("TLAN - Language(s)"),
	I18N_NOOP("TLEN - Length"),
	I18N_NOOP("TMED - Media type"),
	I18N_NOOP("TOAL - Original album/movie/show title"),
	I18N_NOOP("TOFN - Original filename"),
	I18N_NOOP("TOLY - Original lyricist(s)/text writer(s)"),
	I18N_NOOP("TOPE - Original artist(s)/performer(s)"),
	I18N_NOOP("TORY - Original release year"),
	I18N_NOOP("TOWN - File owner/licensee"),
	I18N_NOOP("TPE1 - Lead performer(s)/Soloist(s)"),
	I18N_NOOP("TPE2 - Band/orchestra/accompaniment"),
	I18N_NOOP("TPE3 - Conductor/performer refinement"),
	I18N_NOOP("TPE4 - Interpreted, remixed, or otherwise modified by"),
	I18N_NOOP("TPOS - Part of a set"),
	I18N_NOOP("TPUB - Publisher"),
	I18N_NOOP("TRCK - Track number/Position in set"),
	I18N_NOOP("TRDA - Recording dates"),
	I18N_NOOP("TRSN - Internet radio station name"),
	I18N_NOOP("TRSO - Internet radio station owner"),
	I18N_NOOP("TSIZ - Size"),
	I18N_NOOP("TSRC - ISRC (international standard recording code)"),
	I18N_NOOP("TSSE - Software/Hardware and settings used for encoding"),
	I18N_NOOP("TXXX - User defined text information"),
	I18N_NOOP("TYER - Year"),
	I18N_NOOP("UFID - Unique file identifier"),
	I18N_NOOP("USER - Terms of use"),
	I18N_NOOP("USLT - Unsynchronized lyric/text transcription"),
	I18N_NOOP("WCOM - Commercial information"),
	I18N_NOOP("WCOP - Copyright/Legal information"),
	I18N_NOOP("WOAF - Official audio file webpage"),
	I18N_NOOP("WOAR - Official artist/performer webpage"),
	I18N_NOOP("WOAS - Official audio source webpage"),
	I18N_NOOP("WORS - Official internet radio station homepage"),
	I18N_NOOP("WPAY - Payment"),
	I18N_NOOP("WPUB - Official publisher webpage"),
	I18N_NOOP("WXXX - User defined URL link")
};

/** Frame IDs corresponding to s_frameIdStr[] */
const ID3_FrameID Mp3FrameList::s_frameIdCode[NumFrameIds] = {
	ID3FID_AUDIOCRYPTO,
	ID3FID_PICTURE,
	ID3FID_COMMENT,
	ID3FID_COMMERCIAL,
	ID3FID_CRYPTOREG,
	ID3FID_EQUALIZATION,
	ID3FID_EVENTTIMING,
	ID3FID_GENERALOBJECT,
	ID3FID_GROUPINGREG,
	ID3FID_INVOLVEDPEOPLE,
	ID3FID_LINKEDINFO,
	ID3FID_CDID,
	ID3FID_MPEGLOOKUP,
	ID3FID_OWNERSHIP,
	ID3FID_PRIVATE,
	ID3FID_PLAYCOUNTER,
	ID3FID_POPULARIMETER,
	ID3FID_POSITIONSYNC,
	ID3FID_BUFFERSIZE,
	ID3FID_VOLUMEADJ,
	ID3FID_REVERB,
	ID3FID_SYNCEDLYRICS,
	ID3FID_SYNCEDTEMPO,
	ID3FID_ALBUM,
	ID3FID_BPM,
	ID3FID_COMPOSER,
	ID3FID_CONTENTTYPE,
	ID3FID_COPYRIGHT,
	ID3FID_DATE,
	ID3FID_PLAYLISTDELAY,
	ID3FID_ENCODEDBY,
	ID3FID_LYRICIST,
	ID3FID_FILETYPE,
	ID3FID_TIME,
	ID3FID_CONTENTGROUP,
	ID3FID_TITLE,
	ID3FID_SUBTITLE,
	ID3FID_INITIALKEY,
	ID3FID_LANGUAGE,
	ID3FID_SONGLEN,
	ID3FID_MEDIATYPE,
	ID3FID_ORIGALBUM,
	ID3FID_ORIGFILENAME,
	ID3FID_ORIGLYRICIST,
	ID3FID_ORIGARTIST,
	ID3FID_ORIGYEAR,
	ID3FID_FILEOWNER,
	ID3FID_LEADARTIST,
	ID3FID_BAND,
	ID3FID_CONDUCTOR,
	ID3FID_MIXARTIST,
	ID3FID_PARTINSET,
	ID3FID_PUBLISHER,
	ID3FID_TRACKNUM,
	ID3FID_RECORDINGDATES,
	ID3FID_NETRADIOSTATION,
	ID3FID_NETRADIOOWNER,
	ID3FID_SIZE,
	ID3FID_ISRC,
	ID3FID_ENCODERSETTINGS,
	ID3FID_USERTEXT,
	ID3FID_YEAR,
	ID3FID_UNIQUEFILEID,
	ID3FID_TERMSOFUSE,
	ID3FID_UNSYNCEDLYRICS,
	ID3FID_WWWCOMMERCIALINFO,
	ID3FID_WWWCOPYRIGHT,
	ID3FID_WWWAUDIOFILE,
	ID3FID_WWWARTIST,
	ID3FID_WWWAUDIOSOURCE,
	ID3FID_WWWRADIOPAGE,
	ID3FID_WWWPAYMENT,
	ID3FID_WWWPUBLISHER,
	ID3FID_WWWUSER
};

/**
 * Get description of frame.
 *
 * @param id ID of frame
 * @return description or NULL if id not found.
 */
const char* Mp3FrameList::getIdString(ID3_FrameID id) const
{
	int i;
	for (i = 0; i < NumFrameIds; i++) {
		if (s_frameIdCode[i] == id) {
			return s_frameIdStr[i];
		}
	}
	return NULL;
}

/**
 * Display a dialog to select a frame type.
 *
 * @return ID of selected frame,
 *         -1 if no frame selected.
 */
int Mp3FrameList::selectFrameId()
{
	int i;
	QStringList lst;
	bool ok = false;
	for (i = 0; i < NumFrameIds; i++) {
		lst.append(i18n(s_frameIdStr[i]));
	}
	QString res = QInputDialog::getItem(
		i18n("Add Frame"),
		i18n("Select the frame ID")
#if QT_VERSION >= 0x040000
		// the dialog is too small in Qt4
		+ "                                     "
#endif
		, lst, 0, false, &ok);
	if (ok) {
		int idx = lst.findIndex(res);
		if (idx >= 0 && idx < NumFrameIds) {
			return s_frameIdCode[idx];
		}
	}
	return -1;
}

/**
 * Copy the selected frame to the copy buffer.
 *
 * @return true if frame copied.
 */
bool Mp3FrameList::copyFrame() {
	ID3_Frame* frame = getSelectedFrame();
	if (frame) {
		if (m_copyFrame) {
			delete m_copyFrame;
		}
		m_copyFrame = new ID3_Frame(*frame);
		return true;
	}
	return false;
}

/**
 * Paste the selected frame from the copy buffer.
 *
 * @return true if frame pasted.
 */
bool Mp3FrameList::pasteFrame() {
	if (m_copyFrame && m_tags) {
		ID3_Frame* frame = new ID3_Frame(*m_copyFrame);
		if (frame) {
			m_tags->AttachFrame(frame);
			if (m_file) {
				m_file->markTag2Changed();
			}
			return true;
		}
	}
	return false;
}

#else // HAVE_ID3LIB

void BinaryOpenSave::loadData() {}
void BinaryOpenSave::saveData() {}
void BinaryOpenSave::viewData() {}

#endif // HAVE_ID3LIB
