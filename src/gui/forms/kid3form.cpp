/**
 * \file kid3form.cpp
 * GUI for kid3.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 8 Apr 2003
 *
 * Copyright (C) 2003-2014  Urs Fleisch
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
#include <QMimeData>
#include "frametable.h"
#include "frametablemodel.h"
#include "trackdata.h"
#include "genres.h"
#include "basemainwindow.h"
#include "filelist.h"
#include "configurabletreeview.h"
#include "picturelabel.h"
#include "fileconfig.h"
#include "guiconfig.h"
#include "formatconfig.h"
#include "dirproxymodel.h"
#include "fileproxymodel.h"
#include "kid3application.h"
#if defined Q_OS_MAC && QT_VERSION >= 0x050200
#include <CoreFoundation/CFURL.h>
#endif

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
    QObject(app), m_app(app), m_frameEditor(frameEditor) {}
  virtual ~PictureDblClickHandler() {}

protected:
  /**
   * Event filter function, calls Kid3Application::editOrAddPicture().
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
 * Event filter function, calls Kid3Application::editOrAddPicture() on double click.
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
Kid3Form::Kid3Form(Kid3Application* app, BaseMainWindowImpl* mainWin,
                   QWidget* parent)
  : QSplitter(parent), m_app(app), m_mainWin(mainWin)
{
  setObjectName(QLatin1String("Kid3Form"));

  if (!s_collapsePixmap) {
    s_collapsePixmap = new QPixmap((const char**)collapse_xpm);
  }
  if (!s_expandPixmap) {
    s_expandPixmap = new QPixmap((const char**)expand_xpm);
  }

  setAcceptDrops(true);
  setWindowTitle(tr("Kid3"));

  m_vSplitter = new QSplitter(Qt::Vertical, this);
  m_fileListBox = new FileList(m_vSplitter, m_mainWin);
  m_fileListBox->setModel(m_app->getFileProxyModel());
  m_fileListBox->setSelectionModel(m_app->getFileSelectionModel());
  m_dirListBox = new ConfigurableTreeView(m_vSplitter);
  m_dirListBox->setObjectName(QLatin1String("DirList"));
  m_dirListBox->setItemsExpandable(false);
  m_dirListBox->setRootIsDecorated(false);
  m_dirListBox->setModel(m_app->getDirProxyModel());
  m_dirListBox->setSelectionModel(m_app->getDirSelectionModel());

  connect(m_app, SIGNAL(fileRootIndexChanged(QModelIndex)),
          this, SLOT(setFileRootIndex(QModelIndex)));
  connect(m_app, SIGNAL(dirRootIndexChanged(QModelIndex)),
          this, SLOT(setDirRootIndex(QModelIndex)));

  m_rightHalfVBox = new QWidget;
  QScrollArea* scrollView = new QScrollArea(this);
  scrollView->setWidget(m_rightHalfVBox);
  scrollView->setWidgetResizable(true);
  QVBoxLayout* rightHalfLayout = new QVBoxLayout(m_rightHalfVBox);
  rightHalfLayout->setSpacing(0);

  m_fileButton = new QToolButton(m_rightHalfVBox);
  m_fileButton->setIcon(*s_collapsePixmap);
  m_fileButton->setAutoRaise(true);
#ifdef Q_OS_MAC
  m_fileButton->setStyleSheet(QLatin1String("border: 0;"));
#endif
  m_fileLabel = new QLabel(tr("F&ile"), m_rightHalfVBox);
  QHBoxLayout* fileButtonLayout = new QHBoxLayout;
  fileButtonLayout->addWidget(m_fileButton);
  fileButtonLayout->addWidget(m_fileLabel);
  rightHalfLayout->addLayout(fileButtonLayout);

  m_fileWidget = new QWidget(m_rightHalfVBox);
  m_fileWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  rightHalfLayout->addWidget(m_fileWidget);
  QGridLayout* fileLayout = new QGridLayout(m_fileWidget);

  m_nameLabel = new QLabel(tr("Name:"), m_fileWidget);
  fileLayout->addWidget(m_nameLabel, 0, 0);

  m_nameLineEdit = new QLineEdit(m_fileWidget);
  fileLayout->addWidget(m_nameLineEdit, 0, 1, 1, 4);
  m_fileLabel->setBuddy(m_nameLineEdit);

  QLabel* formatLabel = new QLabel(tr("Format:") + QChar(0x2191),
                                   m_fileWidget);
  fileLayout->addWidget(formatLabel, 1, 0);

  m_formatComboBox = new QComboBox(m_fileWidget);
  m_formatComboBox->setEditable(true);
  m_formatComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatComboBox->setToolTip(TrackDataFormatReplacer::getToolTip());
  connect(m_formatComboBox, SIGNAL(editTextChanged(QString)),
          this, SLOT(onFormatEditTextChanged(QString)));
  fileLayout->addWidget(m_formatComboBox, 1, 1);

  QLabel* fromTagLabel = new QLabel(tr("From:"), m_fileWidget);
  fileLayout->addWidget(fromTagLabel, 1, 2);
  m_fnV1Button = new QPushButton(tr("Tag 1"), m_fileWidget);
  m_fnV1Button->setToolTip(tr("Filename from Tag 1"));
  fileLayout->addWidget(m_fnV1Button, 1, 3);
  QPushButton* fnV2Button = new QPushButton(tr("Tag 2"), m_fileWidget);
  fnV2Button->setToolTip(tr("Filename from Tag 2"));
  fileLayout->addWidget(fnV2Button, 1, 4);

  QLabel* formatFromFilenameLabel = new QLabel(tr("Format:") + QChar(0x2193),
                                               m_fileWidget);
  fileLayout->addWidget(formatFromFilenameLabel, 2, 0);

  m_formatFromFilenameComboBox = new QComboBox(m_fileWidget);
  m_formatFromFilenameComboBox->setEditable(true);
  m_formatFromFilenameComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  m_formatFromFilenameComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_formatFromFilenameComboBox->setToolTip(FrameFormatReplacer::getToolTip());
  connect(m_formatFromFilenameComboBox, SIGNAL(editTextChanged(QString)),
          this, SLOT(onFormatFromFilenameEditTextChanged(QString)));
  fileLayout->addWidget(m_formatFromFilenameComboBox, 2, 1);

  QLabel* toTagLabel = new QLabel(tr("To:"), m_fileWidget);
  fileLayout->addWidget(toTagLabel, 2, 2);
  m_toTagV1Button =
    new QPushButton(tr("Tag 1"), m_fileWidget);
  m_toTagV1Button->setToolTip(tr("Tag 1 from Filename"));
  fileLayout->addWidget(m_toTagV1Button, 2, 3);
  QPushButton* toTagV2Button =
    new QPushButton(tr("Tag 2"), m_fileWidget);
  toTagV2Button->setToolTip(tr("Tag 2 from Filename"));
  fileLayout->addWidget(toTagV2Button, 2, 4);

  m_tag1Button = new QToolButton(m_rightHalfVBox);
  m_tag1Button->setIcon(*s_collapsePixmap);
  m_tag1Button->setAutoRaise(true);
#ifdef Q_OS_MAC
  m_tag1Button->setStyleSheet(QLatin1String("border: 0;"));
#endif
  m_tag1Label = new QLabel(tr("Tag &1"), m_rightHalfVBox);
  QHBoxLayout* tag1ButtonLayout = new QHBoxLayout;
  tag1ButtonLayout->addWidget(m_tag1Button);
  tag1ButtonLayout->addWidget(m_tag1Label);
  rightHalfLayout->addLayout(tag1ButtonLayout);

  m_tag1Widget = new QWidget(m_rightHalfVBox);
  m_tag1Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  rightHalfLayout->addWidget(m_tag1Widget, 100);

  QHBoxLayout* idV1HBoxLayout = new QHBoxLayout(m_tag1Widget);
  m_framesV1Table = new FrameTable(m_app->frameModelV1(), m_app->genreModelV1(),
                                   m_tag1Widget);
  m_framesV1Table->setSelectionModel(m_app->getFramesV1SelectionModel());
  idV1HBoxLayout->addWidget(m_framesV1Table, 100);
  m_tag1Label->setBuddy(m_framesV1Table);

  QVBoxLayout* buttonsV1VBoxLayout = new QVBoxLayout;
  idV1HBoxLayout->addLayout(buttonsV1VBoxLayout);

  QPushButton* id3V1PushButton =
    new QPushButton(tr("From Tag 2"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(id3V1PushButton);

  QPushButton* copyV1PushButton = new QPushButton(tr("Copy"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(copyV1PushButton);

  QPushButton* pasteV1PushButton =
    new QPushButton(tr("Paste"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(pasteV1PushButton);

  QPushButton* removeV1PushButton =
    new QPushButton(tr("Remove"), m_tag1Widget);
  buttonsV1VBoxLayout->addWidget(removeV1PushButton);

  buttonsV1VBoxLayout->addItem(
    new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

  m_tag2Button = new QToolButton(m_rightHalfVBox);
  m_tag2Button->setIcon(*s_collapsePixmap);
  m_tag2Button->setAutoRaise(true);
#ifdef Q_OS_MAC
  m_tag2Button->setStyleSheet(QLatin1String("border: 0;"));
#endif
  m_tag2Label = new QLabel(tr("Tag &2"), m_rightHalfVBox);
  QHBoxLayout* tag2ButtonLayout = new QHBoxLayout;
  tag2ButtonLayout->addWidget(m_tag2Button);
  tag2ButtonLayout->addWidget(m_tag2Label);
  rightHalfLayout->addLayout(tag2ButtonLayout);

  m_tag2Widget = new QWidget(m_rightHalfVBox);
  rightHalfLayout->addWidget(m_tag2Widget, 100);

  QHBoxLayout* idV2HBoxLayout = new QHBoxLayout(m_tag2Widget);
  m_framesV2Table = new FrameTable(m_app->frameModelV2(), m_app->genreModelV2(),
                                   m_tag2Widget);
  m_framesV2Table->setSelectionModel(m_app->getFramesV2SelectionModel());
  idV2HBoxLayout->addWidget(m_framesV2Table);
  m_tag2Label->setBuddy(m_framesV2Table);

  QVBoxLayout* buttonsV2VBoxLayout = new QVBoxLayout;
  idV2HBoxLayout->addLayout(buttonsV2VBoxLayout);

  m_id3V2PushButton = new QPushButton(tr("From Tag 1"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(m_id3V2PushButton);

  QPushButton* copyV2PushButton =
    new QPushButton(tr("Copy"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(copyV2PushButton);

  QPushButton* pasteV2PushButton =
    new QPushButton(tr("Paste"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(pasteV2PushButton);

  QPushButton* removeV2PushButton =
    new QPushButton(tr("Remove"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(removeV2PushButton);

  QFrame* frameLine = new QFrame;
  frameLine->setFrameShape(QFrame::HLine);
  frameLine->setFrameShadow(QFrame::Sunken);
  buttonsV2VBoxLayout->addWidget(frameLine);

  QPushButton* editFramesPushButton =
    new QPushButton(tr("Edit..."), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(editFramesPushButton);
  QPushButton* framesAddPushButton =
    new QPushButton(tr("Add..."), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(framesAddPushButton);
  QPushButton* deleteFramesPushButton =
    new QPushButton(tr("Delete"), m_tag2Widget);
  buttonsV2VBoxLayout->addWidget(deleteFramesPushButton);

  m_pictureLabel = new PictureLabel(this);
  m_pictureLabel->installEventFilter(new PictureDblClickHandler(m_app, m_mainWin));
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
  connect(m_nameLineEdit, SIGNAL(textChanged(QString)), this,
      SLOT(nameLineEditChanged(QString)));
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
  if (ev->mimeData()->hasFormat(QLatin1String("text/uri-list")) ||
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
#if defined Q_OS_MAC && QT_VERSION >= 0x050200
  // workaround for https://bugreports.qt-project.org/browse/QTBUG-40449
  for (QList<QUrl>::iterator it = urls.begin(); it != urls.end(); ++it) {
    if (it->host().isEmpty() &&
        it->path().startsWith(QLatin1String("/.file/id="))) {
      *it = QUrl::fromCFURL(CFURLCreateFilePathURL(NULL, it->toCFURL(), NULL));
    }
  }
#endif
  if (urls.isEmpty())
    return;
  if (
#if QT_VERSION >= 0x040800
    urls.first().isLocalFile()
#else
    !urls.first().toLocalFile().isEmpty()
#endif
    ) {
    QStringList localFiles;
    foreach (const QUrl& url, urls) {
      localFiles.append(url.toLocalFile());
    }
    m_app->openDrop(localFiles);
  } else {
    QString text = urls.first().toString();
    m_app->dropUrl(text);
  }
}

/**
 * Frame list button Edit.
 */
void Kid3Form::editFrame()
{
  m_app->editFrame(m_mainWin);
}

/**
 * Frame list button Add.
 */
void Kid3Form::addFrame()
{
  m_app->addFrame(0, m_mainWin);
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
  formatLineEdit(m_nameLineEdit, txt, &FilenameFormatConfig::instance());
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
  if (fcfg->formatWhileEditing()) {
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
    m_app->setDirUpIndex(
        dirPath.endsWith(QLatin1String("..")) ? index.parent() : QModelIndex());
    m_mainWin->updateCurrentSelection();
    m_mainWin->confirmedOpenDirectory(QStringList() << dirPath);
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
  QString txt = tr("Tag &1");
  if (!str.isEmpty()) {
    txt += QLatin1String(": ");
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
  QString txt = tr("Tag &2");
  if (!str.isEmpty()) {
    txt += QLatin1String(": ");
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
 * Set format text configuration when format edit text is changed.
 * @param text format text
 */
void Kid3Form::onFormatEditTextChanged(const QString& text)
{
  FileConfig::instance().setToFilenameFormat(text);
}

/**
 * Set format from filename text configuration when edit text is changed.
 * @param text format text
 */
void Kid3Form::onFormatFromFilenameEditTextChanged(const QString& text)
{
  FileConfig::instance().setFromFilenameFormat(text);
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
 * Set focus on file list.
 */
void Kid3Form::setFocusFileList()
{
  m_fileListBox->setFocus();
}

/**
 * Set focus on directory list.
 */
void Kid3Form::setFocusDirList()
{
  m_dirListBox->setFocus();
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
  GuiConfig& guiCfg = GuiConfig::instance();
  FileConfig& fileCfg = FileConfig::instance();
  guiCfg.setSplitterSizes(sizes());
  guiCfg.setVSplitterSizes(m_vSplitter->sizes());
  fileCfg.setToFilenameFormatIndex(m_formatComboBox->currentIndex());
  fileCfg.setToFilenameFormat(m_formatComboBox->currentText());
  fileCfg.setToFilenameFormats(getItemsFromComboBox(m_formatComboBox));
  fileCfg.setFromFilenameFormatIndex(m_formatFromFilenameComboBox->currentIndex());
  fileCfg.setFromFilenameFormat(m_formatFromFilenameComboBox->currentText());
  fileCfg.setFromFilenameFormats(getItemsFromComboBox(m_formatFromFilenameComboBox));
  if (!guiCfg.autoHideTags()) {
    guiCfg.setHideFile(m_fileWidget->isHidden());
    guiCfg.setHideV1(m_tag1Widget->isHidden());
    guiCfg.setHideV2(m_tag2Widget->isHidden());
  }
  int column;
  Qt::SortOrder order;
  m_fileListBox->getSortByColumn(column, order);
  guiCfg.setFileListSortColumn(column);
  guiCfg.setFileListSortOrder(order);
  guiCfg.setFileListVisibleColumns(m_fileListBox->getVisibleColumns());
  m_dirListBox->getSortByColumn(column, order);
  guiCfg.setDirListSortColumn(column);
  guiCfg.setDirListSortOrder(order);
  guiCfg.setDirListVisibleColumns(m_dirListBox->getVisibleColumns());
}

/**
 * Read the local settings from the configuration.
 */
void Kid3Form::readConfig()
{
  const GuiConfig& guiCfg = GuiConfig::instance();
  const FileConfig& fileCfg = FileConfig::instance();
  if (!guiCfg.splitterSizes().isEmpty()) {
    setSizes(guiCfg.splitterSizes());
  } else {
    setSizes(QList<int>() << 307 << 601);
  }
  if (!guiCfg.vSplitterSizes().isEmpty()) {
    m_vSplitter->setSizes(guiCfg.vSplitterSizes());
  } else {
    m_vSplitter->setSizes(QList<int>() << 451 << 109);
  }

  // Block signals on combo boxes while setting contents to avoid
  // editTextChanged() signals causing configuration changes.
  m_formatComboBox->blockSignals(true);
  m_formatFromFilenameComboBox->blockSignals(true);
  if (!fileCfg.toFilenameFormats().isEmpty()) {
    m_formatComboBox->clear();
    m_formatComboBox->addItems(fileCfg.toFilenameFormats());
  }
  if (!fileCfg.fromFilenameFormats().isEmpty()) {
    m_formatFromFilenameComboBox->clear();
    m_formatFromFilenameComboBox->addItems(fileCfg.fromFilenameFormats());
  }
  m_formatComboBox->setItemText(fileCfg.toFilenameFormatIndex(),
                                fileCfg.toFilenameFormat());
  m_formatComboBox->setCurrentIndex(fileCfg.toFilenameFormatIndex());
  m_formatFromFilenameComboBox->setItemText(
    fileCfg.fromFilenameFormatIndex(),
    fileCfg.fromFilenameFormat());
  m_formatFromFilenameComboBox->setCurrentIndex(
    fileCfg.fromFilenameFormatIndex());
  m_formatComboBox->blockSignals(false);
  m_formatFromFilenameComboBox->blockSignals(false);

  if (!guiCfg.autoHideTags()) {
    hideFile(guiCfg.hideFile());
    hideV1(guiCfg.hideV1());
    hideV2(guiCfg.hideV2());
  }
  hidePicture(guiCfg.hidePicture());
  m_fileListBox->sortByColumn(guiCfg.fileListSortColumn(),
                              guiCfg.fileListSortOrder());
  m_fileListBox->setVisibleColumns(guiCfg.fileListVisibleColumns());
  m_dirListBox->sortByColumn(guiCfg.dirListSortColumn(),
                             guiCfg.dirListSortOrder());
  m_dirListBox->setVisibleColumns(guiCfg.dirListVisibleColumns());
}

/**
 * Set preview picture data.
 * @param data picture data, empty if no picture is available
 */
void Kid3Form::setPictureData(const QByteArray& data)
{
  m_pictureLabel->setData(data);
}

/**
 * Set details info text.
 *
 * @param str detail information summary as string
 */
void Kid3Form::setDetailInfo(const QString& str)
{
  m_fileLabel->setText(!str.isEmpty()
                       ? tr("F&ile") + QLatin1String(": ") + str
                       : tr("F&ile"));
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
 * Set the next file as the current file.
 *
 * @param select true to select the file
 *
 * @return true if a next file exists.
 */
bool Kid3Form::nextFile(bool select)
{
  FrameTable* editingFrameTable = getEditingFrameTable();
  bool ok = m_app->nextFile(select);
  if (ok && editingFrameTable) {
    editingFrameTable->edit(editingFrameTable->currentIndex());
  }
  return ok;
}

/**
 * Set the previous file as the current file.
 *
 * @param select true to select the file
 *
 * @return true if a previous file exists.
 */
bool Kid3Form::previousFile(bool select)
{
  FrameTable* editingFrameTable = getEditingFrameTable();
  bool ok = m_app->previousFile(select);
  if (ok && editingFrameTable) {
    editingFrameTable->edit(editingFrameTable->currentIndex());
  }
  return ok;
}

/**
 * Get frame table which is currently in editing state.
 * The returned frame table can be used to restore the editing state after
 * changing the current file.
 * @return frame table which is in editing state, 0 if none.
 */
FrameTable* Kid3Form::getEditingFrameTable() const
{
  if (QWidget* focusWidget = QApplication::focusWidget()) {
    if (m_framesV1Table->getCurrentEditor() == focusWidget) {
      return m_framesV1Table;
    } else if (m_framesV2Table->getCurrentEditor() == focusWidget) {
      return m_framesV2Table;
    }
  }
  return 0;
}

/**
 * Set the root index of the file list.
 *
 * @param index root index of directory in file proxy model
 */
void Kid3Form::setFileRootIndex(const QModelIndex& index)
{
  if (index.isValid()) {
    m_fileListBox->setRootIndex(index);
    m_fileListBox->scrollTo(m_fileListBox->currentIndex());
  }
}

/**
 * Set the root index of the directory list.
 *
 * @param index root index of directory in directory model
 */
void Kid3Form::setDirRootIndex(const QModelIndex& index)
{
  if (index.isValid()) {
    m_dirListBox->setRootIndex(index);
  }
}
