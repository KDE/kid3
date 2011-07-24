/**
 * \file kid3form.cpp
 * GUI for kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2011  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kid3form.h"
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QLayout>
#include <QToolTip>
#include <QSplitter>
#include <QDir>
#include <QFrame>
#include <QPixmap>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include <QUrl>
#include <QApplication>
#include <QFileSystemModel>
#include "frametable.h"
#include "frametablemodel.h"
#include "trackdata.h"
#include "genres.h"
#include "kid3mainwindow.h"
#include "filelist.h"
#include "dirlist.h"
#include "picturelabel.h"
#include "configstore.h"
#include "miscconfig.h"
#include "formatconfig.h"
#include "dirproxymodel.h"
#include "fileproxymodel.h"
#include "kid3application.h"
#include "qtcompatmac.h"

/** Collapse pixmap, will be allocated in constructor */
QPixmap* Kid3Form::s_collapsePixmap = 0;
/** Expand pixmap, will be allocated in constructor */
QPixmap* Kid3Form::s_expandPixmap = 0;

/** picture for collapse pixmap */
static const char* const collapse_xpm[] = {
  "7 7 3 1",
  " \tc None",
  ".\tc #FFFFFF",
  "+\tc #000000",
  ".......",
  ".......",
  ".......",
  ".+++++.",
  ".......",
  ".......",
  "......."
};

/** picture for expand pixmap */
static const char* const expand_xpm[] = {
  "7 7 3 1",
  " \tc None",
  ".\tc #FFFFFF",
  "+\tc #000000",
  ".......",
  "...+...",
  "...+...",
  ".+++++.",
  "...+...",
  "...+...",
  "......."
};


/**
 * Event filter for double click on picture label.
 */
class PictureDblClickHandler : public QObject
{
public:
  /**
   * Constructor.
   */
  PictureDblClickHandler(Kid3Application* app, IFrameEditor* frameEditor) :
    m_app(app), m_frameEditor(frameEditor) {}
  virtual ~PictureDblClickHandler() {}

protected:
  /**
   * Event filter function, calls Kid3MainWindow::editOrAddPicture().
   *
   * @param obj watched object
   * @param event event for object
   *
   * @return true if event is filtered.
   */
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  Kid3Application* m_app;
  IFrameEditor* m_frameEditor;
};

/**
 * Event filter function, calls Kid3MainWindow::editOrAddPicture() on double click.
 *
 * @param obj watched object
 * @param event event for object
 *
 * @return true if event is filtered.
 */
bool PictureDblClickHandler::eventFilter(QObject* obj, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonDblClick) {
    m_app->editOrAddPicture(m_frameEditor);
    return true;
  } else {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }
}


/**
 * Constructs an Id3Form as a child of 'parent', with the
 * name 'name' and widget flags set to 'f'.
 * @param app application
 * @param parent parent widget
 */
Kid3Form::Kid3Form(Kid3Application* app, QWidget* parent)
 : QSplitter(parent), m_app(app)
{
  const int margin = 6;
  const int spacing = 2;

  if (!s_collapsePixmap) {
    s_collapsePixmap = new QPixmap((const char**)collapse_xpm);
  }
  if (!s_expandPixmap) {
    s_expandPixmap = new QPixmap((const char**)expand_xpm);
  }

  setAcceptDrops(true);
  setWindowTitle(i18n("Kid3"));

  m_vSplitter = new QSplitter(Qt::Vertical, this);
  m_fileListBox = new FileList(m_vSplitter, mainWin());
  m_fileListBox->setModel(m_app->getFileProxyModel());
  m_fileListBox->setSelectionModel(m_app->getFileSelectionModel());
  m_dirListBox = new DirList(m_vSplitter);
  m_dirListBox->setModel(m_app->getDirProxyModel());

  connect(m_app, SIGNAL(directoryOpened(QModelIndex,QModelIndex)),
          this, SLOT(setDirectoryIndex(QModelIndex,QModelIndex)));

  m_rightHalfVBox = new QWidget;
  QScrollArea* scrollView = new QScrollArea(this);
  scrollView->setWidget(m_rightHalfVBox);
  scrollView->setWidgetResizable(true);
  QVBoxLayout* rightHalfLayout = new QVBoxLayout(m_rightHalfVBox);
  rightHalfLayout->setSpacing(2);
  rightHalfLayout->setMargin(2);

  m_fileButton = new QToolButton(m_rightHalfVBox);
  m_fileButton->setIcon(*s_collapsePixmap);
  m_fileButton->setAutoRaise(true);
  m_fileLabel = new QLabel(i18n("F&ile"), m_rightHalfVBox);
  QHBoxLayout* fileButtonLayout = new QHBoxLayout;
  fileButtonLayout->addWidget(m_fileButton);
  fileButtonLayout->addWidget(m_fileLabel);
  rightHalfLayout->addLayout(fileButtonLayout);

  m_fileWidget = new QWidget(m_rightHalfVBox);
  m_fileWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  rightHalfLayout->addWidget(m_fileWidget);
  QGridLayout* fileLayout = new QGridLayout(m_fileWidget);
  fileLayout->setMargin(margin);
  fileLayout->setSpacing(spacing);

  m_nameLabel = new QLabel(i18n("Name:"), m_fileWidget);
  fileLayout->addWidget(m_nameLabel, 0, 0);

  m_nameLineEdit = new QLineEdit(m_fileWidget);
  fileLayout->addWidget(m_nameLineEdit, 0, 1, 1, 4);
  m_fileLabel->setBuddy(m_nameLineEdit);

  QLabel* formatLabel = new QLabel(i18n("Format:") + QChar(0x2191),
                                   m_fileWidget);
  fileLayout->addWidget(formatLabel, 1, 0);

  m_formatComboBox = new QComboBox(m_fileWidget);
  m_formatComboBox->setEditable(true);
  m_formatComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
  connect(m_formatComboBox, SIGNAL(editTextChanged(QString)),
          m_app, SLOT(setTagsToFilenameFormat(QString)));
  connect(m_app, SIGNAL(tagsToFilenameFormatChanged(QString)),
          m_formatComboBox, SLOT(setEditText(QString)));
  fileLayout->addWidget(m_formatComboBox, 1, 1);

  QLabel* fromTagLabel = new QLabel(i18n("From:"), m_fileWidget);
  fileLayout->addWidget(fromTagLabel, 1, 2);
  m_fnV1Button = new QPushButton(i18n("Tag 1"), m_fileWidget);
  m_fnV1Button->setToolTip(i18n("Filename from Tag 1"));
  fileLayout->addWidget(m_fnV1Button, 1, 3);
  QPushButton* fnV2Button = new QPushButton(i18n("Tag 2"), m_fileWidget);
  fnV2Button->setToolTip(i18n("Filename from Tag 2"));
  fileLayout->addWidget(fnV2Button, 1, 4);

  QLabel* formatFromFilenameLabel = new QLabel(i18n("Format:") + QChar(0x2193),
                                               m_fileWidget);
  fileLayout->addWidget(formatFromFilenameLabel, 2, 0);

  m_formatFromFilenameComboBox = new QComboBox(m_fileWidget);
  m_formatFromFilenameComboBox->setEditable(true);
  m_formatFromFilenameComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatFromFilenameComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatFromFilenameComboBox->setToolTip(FrameFormatReplacer::getToolTip());
  connect(m_formatFromFilenameComboBox, SIGNAL(editTextChanged(QString)),
          m_app, SLOT(setFilenameToTagsFormat(QString)));
  connect(m_app, SIGNAL(filenameToTagsFormatChanged(QString)),
          m_formatFromFilenameComboBox, SLOT(setEditText(QString)));
  fileLayout->addWidget(m_formatFromFilenameComboBox, 2, 1);

  QLabel* toTagLabel = new QLabel(i18n("To:"), m_fileWidget);
  fileLayout->addWidget(toTagLabel, 2, 2);
  m_toTagV1Button =
    new QPushButton(i18n("Tag 1"), m_fileWidget);
  m_toTagV1Button->setToolTip(i18n("Tag 1 from Filename"));
  fileLayout->addWidget(m_toTagV1Button, 2, 3);
  QPushButton* toTagV2Button =
    new QPushButton(i18n("Tag 2"), m_fileWidget);
  toTagV2Button->setToolTip(i18n("Tag 2 from Filename"));
  fileLayout->addWidget(toTagV2Button, 2, 4);

  m_tag1Button = new QToolButton(m_rightHalfVBox);
  m_tag1Button->setIcon(*s_collapsePixmap);
  m_tag1Button->setAutoRaise(true);
  m_tag1Label = new QLabel(i18n("Tag &1"), m_rightHalfVBox);
  QHBoxLayout* tag1ButtonLayout = new QHBoxLayout;
  tag1ButtonLayout->addWidget(m_tag1Button);
  tag1ButtonLayout->addWidget(m_tag1Label);
  rightHalfLayout->addLayout(tag1ButtonLayout);

  m_tag1Widget = new QWidget(m_rightHalfVBox);
  m_tag1Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  rightHalfLayout->addWidget(m_tag1Widget, 100);

  QHBoxLayout* idV1HBoxLayout = new QHBoxLayout(m_tag1Widget);
  idV1HBoxLayout->setMargin(margin);
  idV1HBoxLayout->setSpacing(spacing);
  m_framesV1Table = new FrameTable(m_app->frameModelV1(), m_tag1Widget);
  m_framesV1Table->setSelectionModel(m_app->getFramesV1SelectionModel());
  idV1HBoxLayout->addWidget(m_framesV1Table, 100);
  m_tag1Label->setBuddy(m_framesV1Table);

  QVBoxLayout* buttonsV1VBoxLayout = new QVBoxLayout;
  idV1HBoxLayout->addLayout(buttonsV1VBoxLayout);

  QPushButton* id3V1PushButton =
    new QPushButton(i18n("From Tag 2"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(id3V1PushButton);

  QPushButton* copyV1PushButton = new QPushButton(i18n("Copy"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(copyV1PushButton);

  QPushButton* pasteV1PushButton =
    new QPushButton(i18n("Paste"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(pasteV1PushButton);

  QPushButton* removeV1PushButton =
    new QPushButton(i18n("Remove"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(removeV1PushButton);

  buttonsV1VBoxLayout->addItem(
    new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

  m_tag2Button = new QToolButton(m_rightHalfVBox);
  m_tag2Button->setIcon(*s_collapsePixmap);
  m_tag2Button->setAutoRaise(true);
  m_tag2Label = new QLabel(i18n("Tag &2"), m_rightHalfVBox);
  QHBoxLayout* tag2ButtonLayout = new QHBoxLayout;
  tag2ButtonLayout->addWidget(m_tag2Button);
  tag2ButtonLayout->addWidget(m_tag2Label);
  rightHalfLayout->addLayout(tag2ButtonLayout);

  m_tag2Widget = new QWidget(m_rightHalfVBox);
  rightHalfLayout->addWidget(m_tag2Widget, 100);

  QHBoxLayout* idV2HBoxLayout = new QHBoxLayout(m_tag2Widget);
  idV2HBoxLayout->setMargin(margin);
  idV2HBoxLayout->setSpacing(spacing);
  m_framesV2Table = new FrameTable(m_app->frameModelV2(), m_tag2Widget);
  m_framesV2Table->setSelectionModel(m_app->getFramesV2SelectionModel());
  idV2HBoxLayout->addWidget(m_framesV2Table);
  m_tag2Label->setBuddy(m_framesV2Table);

  QVBoxLayout* buttonsV2VBoxLayout = new QVBoxLayout;
  idV2HBoxLayout->addLayout(buttonsV2VBoxLayout);

  m_id3V2PushButton = new QPushButton(i18n("From Tag 1"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(m_id3V2PushButton);

  QPushButton* copyV2PushButton =
    new QPushButton(i18n("Copy"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(copyV2PushButton);

  QPushButton* pasteV2PushButton =
    new QPushButton(i18n("Paste"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(pasteV2PushButton);

  QPushButton* removeV2PushButton =
    new QPushButton(i18n("Remove"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(removeV2PushButton);

  buttonsV2VBoxLayout->insertSpacing(-1, 8);

  QPushButton* editFramesPushButton =
    new QPushButton(i18n("Edit"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(editFramesPushButton);
  QPushButton* framesAddPushButton =
    new QPushButton(i18n("Add"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(framesAddPushButton);
  QPushButton* deleteFramesPushButton =
    new QPushButton(i18n("Delete"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(deleteFramesPushButton);

  m_pictureLabel = new PictureLabel(this);
  m_pictureLabel->installEventFilter(new PictureDblClickHandler(m_app, mainWin()));
  buttonsV2VBoxLayout->addWidget(m_pictureLabel);

  buttonsV2VBoxLayout->addItem(
    new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

  rightHalfLayout->insertStretch(-1);

  // signals and slots connections
  connect(id3V1PushButton, SIGNAL(clicked()), m_app, SLOT(copyV2ToV1()));
  connect(copyV1PushButton, SIGNAL(clicked()), m_app, SLOT(copyTagsV1()));
  connect(pasteV1PushButton, SIGNAL(clicked()), m_app, SLOT(pasteTagsV1()));
  connect(removeV1PushButton, SIGNAL(clicked()), m_app, SLOT(removeTagsV1()));
  connect(m_id3V2PushButton, SIGNAL(clicked()), m_app, SLOT(copyV1ToV2()));
  connect(copyV2PushButton, SIGNAL(clicked()), m_app, SLOT(copyTagsV2()));
  connect(pasteV2PushButton, SIGNAL(clicked()), m_app, SLOT(pasteTagsV2()));
  connect(removeV2PushButton, SIGNAL(clicked()), m_app, SLOT(removeTagsV2()));
  connect(framesAddPushButton, SIGNAL(clicked()), this, SLOT(addFrame()));
  connect(deleteFramesPushButton, SIGNAL(clicked()), this,
      SLOT(deleteFrame()));
  connect(editFramesPushButton, SIGNAL(clicked()), this, SLOT(editFrame()));
  connect(m_fnV1Button, SIGNAL(clicked()), this, SLOT(fnFromID3V1()));
  connect(fnV2Button, SIGNAL(clicked()), this, SLOT(fnFromID3V2()));
  connect(m_toTagV1Button, SIGNAL(clicked()),
          m_app, SLOT(getTagsFromFilenameV1()));
  connect(toTagV2Button, SIGNAL(clicked()),
          m_app, SLOT(getTagsFromFilenameV2()));
  connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), this,
      SLOT(nameLineEditChanged(const QString&)));
  connect(m_dirListBox, SIGNAL(activated(QModelIndex)), this,
      SLOT(dirSelected(QModelIndex)));
  connect(m_fileButton, SIGNAL(clicked()), this, SLOT(showHideFile()));
  connect(m_tag1Button, SIGNAL(clicked()), this, SLOT(showHideTag1()));
  connect(m_tag2Button, SIGNAL(clicked()), this, SLOT(showHideTag2()));

  // tab order
  setTabOrder(m_fileListBox, m_dirListBox);
  setTabOrder(m_dirListBox, m_nameLineEdit);
  setTabOrder(m_nameLineEdit, m_formatComboBox);
  setTabOrder(m_formatComboBox, m_formatFromFilenameComboBox);
  setTabOrder(m_formatFromFilenameComboBox, m_fnV1Button);
  setTabOrder(m_fnV1Button, fnV2Button);
  setTabOrder(fnV2Button, m_toTagV1Button);
  setTabOrder(m_toTagV1Button, toTagV2Button);
  setTabOrder(toTagV2Button, id3V1PushButton);
  setTabOrder(id3V1PushButton, copyV1PushButton);
  setTabOrder(copyV1PushButton, pasteV1PushButton);
  setTabOrder(pasteV1PushButton, removeV1PushButton);
  setTabOrder(removeV1PushButton, m_id3V2PushButton);
  setTabOrder(m_id3V2PushButton, copyV2PushButton);
  setTabOrder(copyV2PushButton, pasteV2PushButton);
  setTabOrder(pasteV2PushButton, removeV2PushButton);
  setTabOrder(removeV2PushButton, editFramesPushButton);
  setTabOrder(editFramesPushButton, framesAddPushButton);
  setTabOrder(framesAddPushButton, deleteFramesPushButton);
  setTabOrder(deleteFramesPushButton, m_framesV1Table);
  setTabOrder(m_framesV1Table, m_framesV2Table);
}

/**
 * Destructor.
 */
Kid3Form::~Kid3Form()
{
}

/**
 * Accept drag.
 *
 * @param ev drag event.
 */
void Kid3Form::dragEnterEvent(QDragEnterEvent* ev)
{
  if (ev->mimeData()->hasFormat("text/uri-list") ||
      ev->mimeData()->hasImage())
    ev->acceptProposedAction();
}

/**
 * Handle drop event.
 *
 * @param ev drop event.
 */
void Kid3Form::dropEvent(QDropEvent* ev)
{
  if (ev->mimeData()->hasImage()) {
    QImage image = qvariant_cast<QImage>(ev->mimeData()->imageData());
    m_app->dropImage(image);
    return;
  }
  QList<QUrl> urls = ev->mimeData()->urls();
  if (urls.isEmpty())
    return;
  QString text = urls.first().toLocalFile();
  if (!text.isEmpty()) {
    m_app->openDrop(text);
  } else {
    text = urls.first().toString();
    if (text.startsWith("http://")) {
      m_app->dropUrl(text);
    }
  }
}

/**
 * Frame list button Edit.
 */
void Kid3Form::editFrame()
{
  m_app->editFrame(mainWin());
}

/**
 * Frame list button Add.
 */
void Kid3Form::addFrame()
{
  m_app->addFrame(0, mainWin());
}

/**
 * Frame list button Delete.
 */
void Kid3Form::deleteFrame()
{
  m_app->deleteFrame();
}

/**
 * Set filename according to ID3v1 tags.
 */

void Kid3Form::fnFromID3V1()
{
  m_app->getFilenameFromTags(TrackData::TagV1);
}

/**
 * Set filename according to ID3v1 tags.
 */

void Kid3Form::fnFromID3V2()
{
  m_app->getFilenameFromTags(TrackData::TagV2);
}

/**
 * Filename line edit is changed.
 * @param txt contents of line edit
 */
void Kid3Form::nameLineEditChanged(const QString& txt)
{
  formatLineEdit(m_nameLineEdit, txt, &ConfigStore::s_fnFormatCfg);
}

/**
 * Mark the filename as changed.
 * @param en true to mark as changed
 */
void Kid3Form::markChangedFilename(bool en)
{
  if (en) {
    QPalette changedPalette(m_nameLabel->palette());
    changedPalette.setBrush(QPalette::Active, QPalette::Window, changedPalette.mid());
    m_nameLabel->setPalette(changedPalette);
  } else {
    m_nameLabel->setPalette(QPalette());
  }
  m_nameLabel->setAutoFillBackground(en);
}

/**
 * Format string within line edit.
 *
 * @param le   line edit
 * @param txt  text in line edit
 * @param fcfg format configuration
 */
void Kid3Form::formatLineEdit(QLineEdit* le, const QString& txt,
               const FormatConfig* fcfg)
{
  if (fcfg->m_formatWhileEditing) {
    QString str(txt);
    fcfg->formatString(str);
    if (str != txt) {
      int curPos = le->cursorPosition();
      le->setText(str);
      le->setCursorPosition(curPos);
    }
  }
}

/**
 * Directory list box directory selected.
 *
 * @param index selected item
 */
void Kid3Form::dirSelected(const QModelIndex& index)
{
  QString dirPath = index.data(QFileSystemModel::FilePathRole).toString();
  if (!dirPath.isEmpty()) {
    m_dirListBox->setEntryToSelect(
        dirPath.endsWith("..") ? index.parent() : QModelIndex());
    mainWin()->updateCurrentSelection();
    mainWin()->confirmedOpenDirectory(dirPath);
  }
}

/**
 * Enable or disable controls requiring ID3v1 tags.
 *
 * @param enable true to enable
 */
void Kid3Form::enableControlsV1(bool enable)
{
  m_fnV1Button->setEnabled(enable);
  m_toTagV1Button->setEnabled(enable);
  m_id3V2PushButton->setEnabled(enable);
  m_tag1Widget->setEnabled(enable);
}

/**
 * Display the format of tag 1.
 *
 * @param str string describing format, e.g. "ID3v1.1"
 */
void Kid3Form::setTagFormatV1(const QString& str)
{
  QString txt = i18n("Tag &1");
  if (!str.isEmpty()) {
    txt += ": ";
    txt += str;
  }
  m_tag1Label->setText(txt);
}

/**
 * Display the format of tag 2.
 *
 * @param str string describing format, e.g. "ID3v2.4"
 */
void Kid3Form::setTagFormatV2(const QString& str)
{
  QString txt = i18n("Tag &2");
  if (!str.isEmpty()) {
    txt += ": ";
    txt += str;
  }
  m_tag2Label->setText(txt);
}

/**
 * Adjust the size of the right half box.
 */
void Kid3Form::adjustRightHalfBoxSize()
{
  m_rightHalfVBox->adjustSize();
}

/**
 * Hide or show file controls.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hideFile(bool hide)
{
  if (hide) {
    m_fileWidget->hide();
    m_fileButton->setIcon(*s_expandPixmap);
  } else {
    m_fileWidget->show();
    m_fileButton->setIcon(*s_collapsePixmap);
  }
}

/**
 * Hide or show tag 1 controls.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hideV1(bool hide)
{
  if (hide) {
    m_tag1Widget->hide();
    m_tag1Button->setIcon(*s_expandPixmap);
  } else {
    m_tag1Widget->show();
    m_tag1Button->setIcon(*s_collapsePixmap);
  }
}

/**
 * Hide or show tag 2 controls.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hideV2(bool hide)
{
  if (hide) {
    m_tag2Widget->hide();
    m_tag2Button->setIcon(*s_expandPixmap);
  } else {
    m_tag2Widget->show();
    m_tag2Button->setIcon(*s_collapsePixmap);
  }
}

/**
 * Toggle visibility of file controls.
 */
void Kid3Form::showHideFile()
{
  hideFile(!m_fileWidget->isHidden());
}

/**
 * Toggle visibility of tag 1 controls.
 */
void Kid3Form::showHideTag1()
{
  hideV1(!m_tag1Widget->isHidden());
}

/**
 * Toggle visibility of tag 2 controls.
 */
void Kid3Form::showHideTag2()
{
  hideV2(!m_tag2Widget->isHidden());
}

/**
 * Hide or show picture.
 *
 * @param hide true to hide, false to show
 */
void Kid3Form::hidePicture(bool hide)
{
  if (hide) {
    m_pictureLabel->hide();
  } else {
    m_pictureLabel->show();
  }
}

/**
 * Set focus on filename controls.
 */
void Kid3Form::setFocusFilename()
{
  m_nameLineEdit->setFocus();
}

/**
 * Set focus on tag 1 controls.
 */
void Kid3Form::setFocusV1()
{
  m_framesV1Table->setFocus();
}

/**
 * Set focus on tag 2 controls.
 */
void Kid3Form::setFocusV2()
{
  m_framesV2Table->setFocus();
}

/**
 * Get the items from a combo box.
 *
 * @param comboBox combo box
 *
 * @return item texts from combo box.
 */
static QStringList getItemsFromComboBox(const QComboBox* comboBox)
{
  QStringList lst;
  for (int i = 0; i < comboBox->count(); ++i) {
    lst += comboBox->itemText(i);
  }
  return lst;
}

/**
 * Save the local settings to the configuration.
 */
void Kid3Form::saveConfig()
{
  ConfigStore::s_miscCfg.m_splitterSizes = sizes();
  ConfigStore::s_miscCfg.m_vSplitterSizes = m_vSplitter->sizes();
  ConfigStore::s_miscCfg.m_formatItem = m_formatComboBox->currentIndex();
  ConfigStore::s_miscCfg.m_formatText = m_formatComboBox->currentText();
  ConfigStore::s_miscCfg.m_formatItems = getItemsFromComboBox(m_formatComboBox);
  ConfigStore::s_miscCfg.m_formatFromFilenameItem = m_formatFromFilenameComboBox->currentIndex();
  ConfigStore::s_miscCfg.m_formatFromFilenameText = m_formatFromFilenameComboBox->currentText();
  ConfigStore::s_miscCfg.m_formatFromFilenameItems = getItemsFromComboBox(m_formatFromFilenameComboBox);
  if (!ConfigStore::s_miscCfg.m_autoHideTags) {
    ConfigStore::s_miscCfg.m_hideFile = m_fileWidget->isHidden();
    ConfigStore::s_miscCfg.m_hideV1 = m_tag1Widget->isHidden();
    ConfigStore::s_miscCfg.m_hideV2 = m_tag2Widget->isHidden();
  }
}

/**
 * Read the local settings from the configuration.
 */
void Kid3Form::readConfig()
{
  if (!ConfigStore::s_miscCfg.m_splitterSizes.empty()) {
    setSizes(ConfigStore::s_miscCfg.m_splitterSizes);
  } else {
    setSizes(QList<int>() << 307 << 601);
  }
  if (!ConfigStore::s_miscCfg.m_vSplitterSizes.empty()) {
    m_vSplitter->setSizes(ConfigStore::s_miscCfg.m_vSplitterSizes);
  } else {
    m_vSplitter->setSizes(QList<int>() << 451 << 109);
  }
  if (!ConfigStore::s_miscCfg.m_formatItems.isEmpty()) {
    m_formatComboBox->clear();
    m_formatComboBox->addItems(ConfigStore::s_miscCfg.m_formatItems);
  }
  if (!ConfigStore::s_miscCfg.m_formatFromFilenameItems.isEmpty()) {
    m_formatFromFilenameComboBox->clear();
    m_formatFromFilenameComboBox->addItems(ConfigStore::s_miscCfg.m_formatFromFilenameItems);
  }
  m_formatComboBox->setItemText(ConfigStore::s_miscCfg.m_formatItem,
                                ConfigStore::s_miscCfg.m_formatText);
  m_formatComboBox->setCurrentIndex(ConfigStore::s_miscCfg.m_formatItem);
  m_formatFromFilenameComboBox->setItemText(
    ConfigStore::s_miscCfg.m_formatFromFilenameItem,
    ConfigStore::s_miscCfg.m_formatFromFilenameText);
  m_formatFromFilenameComboBox->setCurrentIndex(
    ConfigStore::s_miscCfg.m_formatFromFilenameItem);
  if (!ConfigStore::s_miscCfg.m_autoHideTags) {
    hideFile(ConfigStore::s_miscCfg.m_hideFile);
    hideV1(ConfigStore::s_miscCfg.m_hideV1);
    hideV2(ConfigStore::s_miscCfg.m_hideV2);
  }
  hidePicture(ConfigStore::s_miscCfg.m_hidePicture);
}

/**
 * Init GUI.
 */
void Kid3Form::initView()
{
  QStringList strList;
  m_formatComboBox->setEditable(true);
  for (const char** sl = MiscConfig::s_defaultFnFmtList; *sl != 0; ++sl) {
    strList += *sl;
  }
  m_formatComboBox->addItems(strList);
  m_formatFromFilenameComboBox->addItems(strList);
}

/**
 * Set preview picture data.
 * @param data picture data, 0 if no picture is available
 */
void Kid3Form::setPictureData(const QByteArray* data)
{
  m_pictureLabel->setData(data);
}

/**
 * Set details info text.
 *
 * @param info detail information
 */
void Kid3Form::setDetailInfo(const TaggedFile::DetailInfo& info)
{
  QString str;
  if (info.valid) {
    str = info.format;
    str += ' ';
    if (info.bitrate > 0 && info.bitrate < 999) {
      if (info.vbr) str += "VBR ";
      str += QString::number(info.bitrate);
      str += " kbps ";
    }
    if (info.sampleRate > 0) {
      str += QString::number(info.sampleRate);
      str += " Hz ";
    }
    switch (info.channelMode) {
      case TaggedFile::DetailInfo::CM_Stereo:
        str += "Stereo ";
        break;
      case TaggedFile::DetailInfo::CM_JointStereo:
        str += "Joint Stereo ";
        break;
      default:
        if (info.channels > 0) {
          str += QString::number(info.channels);
          str += " Channels ";
        }
    }
    if (info.duration > 0) {
      str += TaggedFile::formatTime(info.duration);
    }
  }
  if (!str.isEmpty()) {
    str = i18n("F&ile") + ": " + str;
  } else {
    str = i18n("F&ile");
  }
  m_fileLabel->setText(str);
}

/**
 * Select all files.
 */
void Kid3Form::selectAllFiles()
{
  m_fileListBox->selectAll();
}

/**
 * Deselect all files.
 */
void Kid3Form::deselectAllFiles()
{
  m_fileListBox->clearSelection();
}

/**
 * Set the root index of the directory and file lists.
 *
 * @param directoryIndex root index of directory in file system model
 * @param fileIndex index of file to select
 */
void Kid3Form::setDirectoryIndex(const QModelIndex& directoryIndex,
                                 const QModelIndex& fileIndex)
{
  m_fileListBox->readDir(directoryIndex, fileIndex);
  m_dirListBox->readDir(directoryIndex);
}
