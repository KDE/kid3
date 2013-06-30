/**
 * \file configdialogpages.cpp
 * Pages for configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2013  Urs Fleisch
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

#include "configdialogpages.h"
#include <cstring>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QListView>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStringListModel>
#include "formatconfig.h"
#include "formatbox.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "networkconfig.h"
#include "stringlistedit.h"
#include "configtable.h"
#include "commandstablemodel.h"
#include "checkablestringlistmodel.h"
#include "contexthelp.h"
#include "configstore.h"

enum { TextEncodingV1Latin1Index = 13 };

#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
/**
 * Remove aliases in braces from text encoding combo box entry.
 *
 * @param comboEntry text encoding combo box entry
 *
 * @return codec name.
 */
static QString getTextEncodingV1CodecName(const QString& comboEntry)
{
  int braceIdx = comboEntry.indexOf(QLatin1String(" ("));
  return braceIdx == -1 ? comboEntry : comboEntry.left(braceIdx);
}
#endif

/**
 * Constructor.
 */
ConfigDialogPages::ConfigDialogPages(QObject* parent) : QObject(parent)
{
}

/**
 * Destructor.
 */
ConfigDialogPages::~ConfigDialogPages()
{
}

/**
 * Create page with tags settings.
 * @return tags page.
 */
QWidget* ConfigDialogPages::createTagsPage()
{
  QWidget* tagsPage = new QWidget;
  QVBoxLayout* vlayout = new QVBoxLayout(tagsPage);

  QWidget* tag1Page = new QWidget;
  QVBoxLayout* tag1Layout = new QVBoxLayout(tag1Page);
  QGroupBox* v1GroupBox = new QGroupBox(tr("ID3v1"), tag1Page);
  QGridLayout* v1GroupBoxLayout = new QGridLayout(v1GroupBox);
  m_markTruncationsCheckBox = new QCheckBox(tr("&Mark truncated fields"), v1GroupBox);
  v1GroupBoxLayout->addWidget(m_markTruncationsCheckBox, 0, 0, 1, 2);
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  QLabel* textEncodingV1Label = new QLabel(tr("Text &encoding:"), v1GroupBox);
  m_textEncodingV1ComboBox = new QComboBox(v1GroupBox);
  static const char* const codecs[] = {
    "Apple Roman (macintosh)",
    "Big5",
    "big5-0",
    "Big5-HKSCS",
    "big5hkscs-0",
    "EUC-JP",
    "EUC-KR",
    "GB18030",
    "GBK (windows-936)",
    "hp-roman8",
    "IBM850",
    "IBM866",
    "ISO-2022-JP (JIS7)",
    "ISO-8859-1 (latin1)",
    "ISO-8859-2 (latin2)",
    "ISO-8859-3 (latin3)",
    "ISO-8859-4 (latin4)",
    "ISO-8859-5 (cyrillic)",
    "ISO-8859-6 (arabic)",
    "ISO-8859-7 (greek)",
    "ISO-8859-8 (hebrew)",
    "ISO-8859-9 (latin5)",
    "ISO-8859-10 (latin6)",
    "ISO-8859-13 (baltic)",
    "ISO-8859-14 (latin8, iso-celtic)",
    "ISO-8859-15 (latin9)",
    "ISO-8859-16 (latin10)",
    "ISO-10646-UCS-2 (UTF-16)",
    "Iscii-Bng",
    "Iscii-Dev",
    "Iscii-Gjr",
    "Iscii-Knd",
    "Iscii-Mlm",
    "Iscii-Ori",
    "Iscii-Pnj",
    "Iscii-Tlg",
    "Iscii-Tml",
    "jisx0201*-0",
    "KOI8-R",
    "KOI8-U",
    "ksc5601.1987-0",
    "mulelao-1",
    "Shift_JIS (SJIS, MS_Kanji)",
    "TIS-620 (ISO 8859-11)",
    "TSCII",
    "UTF-8",
    "windows-1250",
    "windows-1251",
    "windows-1252",
    "windows-1253",
    "windows-1254",
    "windows-1255",
    "windows-1256",
    "windows-1257",
    "windows-1258",
    "WINSAMI2 (WS2)",
    0
  };
  Q_ASSERT(std::strcmp(codecs[TextEncodingV1Latin1Index], "ISO-8859-1 (latin1)") == 0);
  const char* const* str = codecs;
  m_textEncodingV1List.clear();
  while (*str) {
    m_textEncodingV1List += QString::fromLatin1(*str++);
  }
  m_textEncodingV1ComboBox->addItems(m_textEncodingV1List);
  m_textEncodingV1ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  textEncodingV1Label->setBuddy(m_textEncodingV1ComboBox);
  v1GroupBoxLayout->addWidget(textEncodingV1Label, 1, 0);
  v1GroupBoxLayout->addWidget(m_textEncodingV1ComboBox, 1, 1);
#endif
  tag1Layout->addWidget(v1GroupBox);
  tag1Layout->addStretch();

  QWidget* tag2Page = new QWidget;
  QVBoxLayout* tag2Layout = new QVBoxLayout(tag2Page);
  QGroupBox* v2GroupBox = new QGroupBox(tr("ID3v2"), tag2Page);
  QGridLayout* v2GroupBoxLayout = new QGridLayout(v2GroupBox);
  m_totalNumTracksCheckBox = new QCheckBox(tr("Use &track/total number of tracks format"), v2GroupBox);
  v2GroupBoxLayout->addWidget(m_totalNumTracksCheckBox, 0, 0, 1, 2);
  QLabel* trackNumberDigitsLabel = new QLabel(tr("Track number &digits:"), v2GroupBox);
  m_trackNumberDigitsSpinBox = new QSpinBox(v2GroupBox);
  m_trackNumberDigitsSpinBox->setMaximum(5);
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  m_genreNotNumericCheckBox = new QCheckBox(tr("&Genre as text instead of numeric string"), v2GroupBox);
  QLabel* textEncodingLabel = new QLabel(tr("Text &encoding:"), v2GroupBox);
  m_textEncodingComboBox = new QComboBox(v2GroupBox);
  m_textEncodingComboBox->insertItem(TagConfig::TE_ISO8859_1, tr("ISO-8859-1"));
  m_textEncodingComboBox->insertItem(TagConfig::TE_UTF16, tr("UTF16"));
  m_textEncodingComboBox->insertItem(TagConfig::TE_UTF8, tr("UTF8"));
  m_textEncodingComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  textEncodingLabel->setBuddy(m_textEncodingComboBox);
  v2GroupBoxLayout->addWidget(m_genreNotNumericCheckBox, 1, 0, 1, 2);
  v2GroupBoxLayout->addWidget(textEncodingLabel, 2, 0);
  v2GroupBoxLayout->addWidget(m_textEncodingComboBox, 2, 1);
#endif
#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  QLabel* id3v2VersionLabel = new QLabel(tr("&Version used for new tags:"), v2GroupBox);
  m_id3v2VersionComboBox = new QComboBox(v2GroupBox);
#ifdef HAVE_ID3LIB
  m_id3v2VersionComboBox->addItem(tr("ID3v2.3.0 (id3lib)"), TagConfig::ID3v2_3_0);
#endif
  m_id3v2VersionComboBox->addItem(tr("ID3v2.4.0 (TagLib)"), TagConfig::ID3v2_4_0);
#ifdef HAVE_TAGLIB_ID3V23_SUPPORT
  m_id3v2VersionComboBox->addItem(tr("ID3v2.3.0 (TagLib)"), TagConfig::ID3v2_3_0_TAGLIB);
#endif
  m_id3v2VersionComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  id3v2VersionLabel->setBuddy(m_id3v2VersionComboBox);
  v2GroupBoxLayout->addWidget(id3v2VersionLabel, 3, 0);
  v2GroupBoxLayout->addWidget(m_id3v2VersionComboBox, 3, 1);
#endif
  trackNumberDigitsLabel->setBuddy(m_trackNumberDigitsSpinBox);
  v2GroupBoxLayout->addWidget(trackNumberDigitsLabel, 4, 0);
  v2GroupBoxLayout->addWidget(m_trackNumberDigitsSpinBox, 4, 1);
  tag2Layout->addWidget(v2GroupBox);
#ifdef HAVE_VORBIS
  QGroupBox* vorbisGroupBox = new QGroupBox(tr("Ogg/Vorbis"), tag2Page);
  QLabel* commentNameLabel = new QLabel(tr("Co&mment field name:"), vorbisGroupBox);
  m_commentNameComboBox = new QComboBox(vorbisGroupBox);
  QLabel* pictureNameLabel = new QLabel(tr("&Picture field name:"), vorbisGroupBox);
  m_pictureNameComboBox = new QComboBox(vorbisGroupBox);
  m_commentNameComboBox->setEditable(true);
  QStringList items;
  items += QLatin1String("COMMENT");
  items += QLatin1String("DESCRIPTION");
  m_commentNameComboBox->addItems(items);
  m_commentNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  commentNameLabel->setBuddy(m_commentNameComboBox);
  m_pictureNameComboBox->addItems(QStringList() << QLatin1String("METADATA_BLOCK_PICTURE") << QLatin1String("COVERART"));
  m_pictureNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  pictureNameLabel->setBuddy(m_pictureNameComboBox);
  QGridLayout* vorbisGroupBoxLayout = new QGridLayout(vorbisGroupBox);
  vorbisGroupBoxLayout->addWidget(commentNameLabel, 0, 0);
  vorbisGroupBoxLayout->addWidget(m_commentNameComboBox, 0, 1);
  vorbisGroupBoxLayout->addWidget(pictureNameLabel, 1, 0);
  vorbisGroupBoxLayout->addWidget(m_pictureNameComboBox, 1, 1);
  vorbisGroupBox->setLayout(vorbisGroupBoxLayout);
  tag2Layout->addWidget(vorbisGroupBox);
#endif
  QHBoxLayout* genresQuickAccessLayout = new QHBoxLayout;
  QGroupBox* genresGroupBox = new QGroupBox(tr("Custom &Genres"), tag2Page);
  m_onlyCustomGenresCheckBox = new QCheckBox(tr("&Show only custom genres"), genresGroupBox);
  m_genresEditModel = new QStringListModel(genresGroupBox);
  StringListEdit* genresEdit = new StringListEdit(m_genresEditModel, genresGroupBox);
  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget(m_onlyCustomGenresCheckBox);
  vbox->addWidget(genresEdit);
  genresGroupBox->setLayout(vbox);
  genresQuickAccessLayout->addWidget(genresGroupBox);

  QGroupBox* quickAccessTagsGroupBox = new QGroupBox(tr("&Quick Access Tags"));
  QVBoxLayout* quickAccessTagsLayout = new QVBoxLayout(quickAccessTagsGroupBox);
  QListView* quickAccessTagsListView = new QListView;
  m_quickAccessTagsModel = new CheckableStringListModel(quickAccessTagsGroupBox);
  QStringList unifiedFrameNames;
  for (int i = Frame::FT_FirstFrame; i< Frame::FT_LastFrame; ++i) {
    unifiedFrameNames.append(
        Frame::ExtendedType(static_cast<Frame::Type>(i)).getTranslatedName());
  }
  m_quickAccessTagsModel->setStringList(unifiedFrameNames);
  quickAccessTagsListView->setModel(m_quickAccessTagsModel);
  quickAccessTagsLayout->addWidget(quickAccessTagsListView);
  genresQuickAccessLayout->addWidget(quickAccessTagsGroupBox);
  tag2Layout->addLayout(genresQuickAccessLayout);

  QWidget* tag1AndTag2Page = new QWidget;
  QVBoxLayout* tag1AndTag2Layout = new QVBoxLayout(tag1AndTag2Page);
  QString id3FormatTitle(tr("&Tag Format"));
  m_id3FormatBox = new FormatBox(id3FormatTitle, tag1AndTag2Page);
  tag1AndTag2Layout->addWidget(m_id3FormatBox);

  QTabWidget* tagsTabWidget = new QTabWidget;
  tagsTabWidget->addTab(tag1Page, tr("Tag &1"));
  tagsTabWidget->addTab(tag2Page, tr("Tag &2"));
  tagsTabWidget->addTab(tag1AndTag2Page, tr("Tag 1 a&nd Tag 2"));
  tagsTabWidget->setCurrentIndex(1);
  vlayout->addWidget(tagsTabWidget);
  return tagsPage;
}

/**
 * Create page with files settings.
 * @return files page.
 */
QWidget* ConfigDialogPages::createFilesPage()
{
  QWidget* filesPage = new QWidget;
  QVBoxLayout* vlayout = new QVBoxLayout(filesPage);
  QGroupBox* startupGroupBox = new QGroupBox(tr("Startup"), filesPage);
  m_loadLastOpenedFileCheckBox = new QCheckBox(tr("&Load last-opened files"),
                                               startupGroupBox);
  QVBoxLayout* startupLayout = new QVBoxLayout;
  startupLayout->addWidget(m_loadLastOpenedFileCheckBox);
  startupGroupBox->setLayout(startupLayout);
  vlayout->addWidget(startupGroupBox);
  QGroupBox* saveGroupBox = new QGroupBox(tr("Save"), filesPage);
  m_preserveTimeCheckBox = new QCheckBox(tr("&Preserve file timestamp"), saveGroupBox);
  m_markChangesCheckBox = new QCheckBox(tr("&Mark changes"), saveGroupBox);
  QLabel* coverFileNameLabel = new QLabel(tr("F&ilename for cover:"), saveGroupBox);
  m_coverFileNameLineEdit = new QLineEdit(saveGroupBox);
  coverFileNameLabel->setBuddy(m_coverFileNameLineEdit);
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setContentsMargins(2, 0, 2, 0);
  hbox->addWidget(coverFileNameLabel);
  hbox->addWidget(m_coverFileNameLineEdit);
  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget(m_preserveTimeCheckBox);
  vbox->addWidget(m_markChangesCheckBox);
  vbox->addLayout(hbox);
  saveGroupBox->setLayout(vbox);
  vlayout->addWidget(saveGroupBox);
  QString fnFormatTitle(tr("&Filename Format"));
  m_fnFormatBox = new FormatBox(fnFormatTitle, filesPage);
  vlayout->addWidget(m_fnFormatBox);
  return filesPage;
}

/**
 * Create page with actions settings.
 * @return actions page.
 */
QWidget* ConfigDialogPages::createActionsPage()
{
  QWidget* actionsPage = new QWidget;
  QVBoxLayout* vlayout = new QVBoxLayout(actionsPage);
  QGroupBox* browserGroupBox = new QGroupBox(tr("Browser"), actionsPage);
  QLabel* browserLabel = new QLabel(tr("Web &browser:"), browserGroupBox);
  m_browserLineEdit = new QLineEdit(browserGroupBox);
  browserLabel->setBuddy(m_browserLineEdit);
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(browserLabel);
  hbox->addWidget(m_browserLineEdit);
  browserGroupBox->setLayout(hbox);
  vlayout->addWidget(browserGroupBox);

  QGroupBox* commandsGroupBox = new QGroupBox(tr("Context &Menu Commands"), actionsPage);
  m_playOnDoubleClickCheckBox =
      new QCheckBox(tr("&Play on double click"), commandsGroupBox);
  m_commandsTableModel = new CommandsTableModel(commandsGroupBox);
  m_commandsTable = new ConfigTable(m_commandsTableModel, commandsGroupBox);
  m_commandsTable->setHorizontalResizeModes(
    m_commandsTableModel->getHorizontalResizeModes());
  QVBoxLayout* commandsLayout = new QVBoxLayout;
  commandsLayout->addWidget(m_playOnDoubleClickCheckBox);
  commandsLayout->addWidget(m_commandsTable);
  commandsGroupBox->setLayout(commandsLayout);
  vlayout->addWidget(commandsGroupBox);
  return actionsPage;
}

/**
 * Create page with network settings.
 * @return network page.
 */
QWidget* ConfigDialogPages::createNetworkPage()
{
  QWidget* networkPage = new QWidget;
  QVBoxLayout* vlayout = new QVBoxLayout(networkPage);
  QGroupBox* proxyGroupBox = new QGroupBox(tr("Proxy"), networkPage);
  m_proxyCheckBox = new QCheckBox(tr("&Proxy:"), proxyGroupBox);
  m_proxyLineEdit = new QLineEdit(proxyGroupBox);
  m_proxyAuthenticationCheckBox = new QCheckBox(tr("&Use authentication with proxy"), proxyGroupBox);
  QLabel* proxyUserNameLabel = new QLabel(tr("Proxy user &name:"), proxyGroupBox);
  m_proxyUserNameLineEdit = new QLineEdit(proxyGroupBox);
  proxyUserNameLabel->setBuddy(m_proxyUserNameLineEdit);
  QLabel* proxyPasswordLabel = new QLabel(tr("Proxy pass&word:"), proxyGroupBox);
  m_proxyPasswordLineEdit = new QLineEdit(proxyGroupBox);
  proxyPasswordLabel->setBuddy(m_proxyPasswordLineEdit);
  m_proxyPasswordLineEdit->setEchoMode(QLineEdit::Password);
  QVBoxLayout* vbox = new QVBoxLayout;
  QHBoxLayout* proxyHbox = new QHBoxLayout;
  proxyHbox->addWidget(m_proxyCheckBox);
  proxyHbox->addWidget(m_proxyLineEdit);
  vbox->addLayout(proxyHbox);
  vbox->addWidget(m_proxyAuthenticationCheckBox);
  QGridLayout* authLayout = new QGridLayout;
  authLayout->addWidget(proxyUserNameLabel, 0, 0);
  authLayout->addWidget(m_proxyUserNameLineEdit, 0, 1);
  authLayout->addWidget(proxyPasswordLabel, 1, 0);
  authLayout->addWidget(m_proxyPasswordLineEdit, 1, 1);
  vbox->addLayout(authLayout);
  proxyGroupBox->setLayout(vbox);
  vlayout->addWidget(proxyGroupBox);

  QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  vlayout->addItem(vspacer);
  return networkPage;
}

/**
 * Set values in dialog from current configuration.
 *
 * @param cfg configuration
 */
void ConfigDialogPages::setConfig(const ConfigStore* cfg)
{
  const FormatConfig* fnCfg = &cfg->s_fnFormatCfg;
  const FormatConfig* id3Cfg = &cfg->s_id3FormatCfg;
  const TagConfig* tagCfg = &cfg->s_tagCfg;
  const FileConfig* fileCfg = &cfg->s_fileCfg;
  const UserActionsConfig* userActionsCfg = &cfg->s_userActionsCfg;
  const GuiConfig* guiCfg = &cfg->s_guiCfg;
  const NetworkConfig* networkCfg = &cfg->s_networkCfg;

  m_fnFormatBox->fromFormatConfig(fnCfg);
  m_id3FormatBox->fromFormatConfig(id3Cfg);
  m_markTruncationsCheckBox->setChecked(tagCfg->m_markTruncations);
  m_totalNumTracksCheckBox->setChecked(tagCfg->m_enableTotalNumberOfTracks);
  m_loadLastOpenedFileCheckBox->setChecked(fileCfg->m_loadLastOpenedFile);
  m_preserveTimeCheckBox->setChecked(fileCfg->m_preserveTime);
  m_markChangesCheckBox->setChecked(fileCfg->m_markChanges);
  m_coverFileNameLineEdit->setText(fileCfg->m_defaultCoverFileName);
  m_onlyCustomGenresCheckBox->setChecked(tagCfg->m_onlyCustomGenres);
  m_genresEditModel->setStringList(tagCfg->m_customGenres);
  m_quickAccessTagsModel->setBitMask(tagCfg->m_quickAccessFrames);
  m_commandsTableModel->setCommandList(userActionsCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
  int idx = m_commentNameComboBox->findText(tagCfg->m_commentName);
  if (idx >= 0) {
    m_commentNameComboBox->setCurrentIndex(idx);
  } else {
    m_commentNameComboBox->addItem(tagCfg->m_commentName);
    m_commentNameComboBox->setCurrentIndex(m_commentNameComboBox->count() - 1);
  }
  m_pictureNameComboBox->setCurrentIndex(tagCfg->m_pictureNameItem);
#endif
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  m_genreNotNumericCheckBox->setChecked(tagCfg->m_genreNotNumeric);
  int textEncodingV1Index = TextEncodingV1Latin1Index;
  int index = 0;
  for (QStringList::const_iterator it = m_textEncodingV1List.begin();
       it != m_textEncodingV1List.end();
       ++it) {
    if (getTextEncodingV1CodecName(*it) == tagCfg->m_textEncodingV1) {
      textEncodingV1Index = index;
      break;
    }
    ++index;
  }
  m_textEncodingV1ComboBox->setCurrentIndex(textEncodingV1Index);
  m_textEncodingComboBox->setCurrentIndex(tagCfg->m_textEncoding);
#endif
#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  m_id3v2VersionComboBox->setCurrentIndex(
        m_id3v2VersionComboBox->findData(tagCfg->m_id3v2Version));
#endif
  m_trackNumberDigitsSpinBox->setValue(tagCfg->m_trackNumberDigits);
  m_browserLineEdit->setText(networkCfg->m_browser);
  m_playOnDoubleClickCheckBox->setChecked(guiCfg->m_playOnDoubleClick);
  m_proxyCheckBox->setChecked(networkCfg->m_useProxy);
  m_proxyLineEdit->setText(networkCfg->m_proxy);
  m_proxyAuthenticationCheckBox->setChecked(networkCfg->m_useProxyAuthentication);
  m_proxyUserNameLineEdit->setText(networkCfg->m_proxyUserName);
  m_proxyPasswordLineEdit->setText(networkCfg->m_proxyPassword);
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param cfg configuration
 */
void ConfigDialogPages::getConfig(ConfigStore* cfg) const
{
  FormatConfig* fnCfg = &cfg->s_fnFormatCfg;
  FormatConfig* id3Cfg = &cfg->s_id3FormatCfg;
  TagConfig* tagCfg = &cfg->s_tagCfg;
  FileConfig* fileCfg = &cfg->s_fileCfg;
  UserActionsConfig* userActionsCfg = &cfg->s_userActionsCfg;
  GuiConfig* guiCfg = &cfg->s_guiCfg;
  NetworkConfig* networkCfg = &cfg->s_networkCfg;

  m_fnFormatBox->toFormatConfig(fnCfg);
  m_id3FormatBox->toFormatConfig(id3Cfg);
  tagCfg->m_markTruncations = m_markTruncationsCheckBox->isChecked();
  tagCfg->m_enableTotalNumberOfTracks = m_totalNumTracksCheckBox->isChecked();
  fileCfg->m_loadLastOpenedFile = m_loadLastOpenedFileCheckBox->isChecked();
  fileCfg->m_preserveTime = m_preserveTimeCheckBox->isChecked();
  fileCfg->m_markChanges = m_markChangesCheckBox->isChecked();
  fileCfg->m_defaultCoverFileName = m_coverFileNameLineEdit->text();
  tagCfg->m_onlyCustomGenres = m_onlyCustomGenresCheckBox->isChecked();
  tagCfg->m_customGenres = m_genresEditModel->stringList();
  tagCfg->m_quickAccessFrames = m_quickAccessTagsModel->getBitMask();
  userActionsCfg->m_contextMenuCommands = m_commandsTableModel->getCommandList();
#ifdef HAVE_VORBIS
  tagCfg->m_commentName = m_commentNameComboBox->currentText();
  tagCfg->m_pictureNameItem = m_pictureNameComboBox->currentIndex();
#endif
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  tagCfg->m_genreNotNumeric = m_genreNotNumericCheckBox->isChecked();
  tagCfg->m_textEncodingV1 =
    getTextEncodingV1CodecName(m_textEncodingV1ComboBox->currentText());
  tagCfg->m_textEncoding = m_textEncodingComboBox->currentIndex();
#endif
#if defined HAVE_TAGLIB && (defined HAVE_ID3LIB || defined HAVE_TAGLIB_ID3V23_SUPPORT)
  tagCfg->m_id3v2Version = m_id3v2VersionComboBox->itemData(
        m_id3v2VersionComboBox->currentIndex()).toInt();
#endif
  tagCfg->m_trackNumberDigits = m_trackNumberDigitsSpinBox->value();
  networkCfg->m_browser = m_browserLineEdit->text();
  guiCfg->m_playOnDoubleClick = m_playOnDoubleClickCheckBox->isChecked();
  networkCfg->m_useProxy = m_proxyCheckBox->isChecked();
  networkCfg->m_proxy = m_proxyLineEdit->text();
  networkCfg->m_useProxyAuthentication = m_proxyAuthenticationCheckBox->isChecked();
  networkCfg->m_proxyUserName = m_proxyUserNameLineEdit->text();
  networkCfg->m_proxyPassword = m_proxyPasswordLineEdit->text();
}
