/**
 * \file configdialog.cpp
 * Configuration dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "configdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#endif

#include <cstring>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QSpinBox>
#include "qtcompatmac.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QStringListModel>
#ifndef CONFIG_USE_KDE
#include <QApplication>
#include <QFontDialog>
#include <QStyleFactory>
#endif

#include "formatconfig.h"
#include "formatbox.h"
#include "miscconfig.h"
#include "stringlistedit.h"
#include "configtable.h"
#include "commandstablemodel.h"
#include "contexthelp.h"

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
  int braceIdx = comboEntry.indexOf(" (");
  return braceIdx == -1 ? comboEntry : comboEntry.left(braceIdx);
}
#endif

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param configSkeleton KDE config skeleton
 */
#ifdef CONFIG_USE_KDE
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption,
                           KConfigSkeleton* configSkeleton) :
  KConfigDialog(parent, "configure", configSkeleton)
#else
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption) :
  QDialog(parent)
#endif
{
  setWindowTitle(caption);
#ifndef CONFIG_USE_KDE
  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(6);
  topLayout->setMargin(6);
  QTabWidget* tabWidget = new QTabWidget(this);
#endif

  QWidget* tagsPage = new QWidget;
  if (tagsPage) {
    QVBoxLayout* vlayout = new QVBoxLayout(tagsPage);
    if (vlayout) {
      vlayout->setMargin(6);
      vlayout->setSpacing(6);
      QGroupBox* v1GroupBox = new QGroupBox(i18n("ID3v1"), tagsPage);
      QGridLayout* v1GroupBoxLayout = new QGridLayout(v1GroupBox);
      v1GroupBoxLayout->setMargin(2);
      v1GroupBoxLayout->setSpacing(4);
      if (v1GroupBox) {
        m_markTruncationsCheckBox = new QCheckBox(i18n("&Mark truncated fields"), v1GroupBox);
        v1GroupBoxLayout->addWidget(m_markTruncationsCheckBox, 0, 0, 1, 2);
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
        QLabel* textEncodingV1Label = new QLabel(i18n("Text &encoding:"), v1GroupBox);
        m_textEncodingV1ComboBox = new QComboBox(v1GroupBox);
#endif
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
        if (textEncodingV1Label && m_textEncodingV1ComboBox) {
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
            m_textEncodingV1List += *str++;
          }
          m_textEncodingV1ComboBox->addItems(m_textEncodingV1List);
          m_textEncodingV1ComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
          textEncodingV1Label->setBuddy(m_textEncodingV1ComboBox);
          v1GroupBoxLayout->addWidget(textEncodingV1Label, 1, 0);
          v1GroupBoxLayout->addWidget(m_textEncodingV1ComboBox, 1, 1);
        }
#endif
        vlayout->addWidget(v1GroupBox);
      }
      QGroupBox* v2GroupBox = new QGroupBox(i18n("ID3v2"), tagsPage);
      QGridLayout* v2GroupBoxLayout = new QGridLayout(v2GroupBox);
      v2GroupBoxLayout->setMargin(2);
      v2GroupBoxLayout->setSpacing(4);
      if (v2GroupBox) {
        m_totalNumTracksCheckBox = new QCheckBox(i18n("Use &track/total number of tracks format"), v2GroupBox);
        v2GroupBoxLayout->addWidget(m_totalNumTracksCheckBox, 0, 0, 1, 2);
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
        m_genreNotNumericCheckBox = new QCheckBox(i18n("&Genre as text instead of numeric string"), v2GroupBox);
        QLabel* textEncodingLabel = new QLabel(i18n("Text &encoding:"), v2GroupBox);
        m_textEncodingComboBox = new QComboBox(v2GroupBox);
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
        QLabel* id3v2VersionLabel = new QLabel(i18n("&Version used for new tags:"), v2GroupBox);
        m_id3v2VersionComboBox = new QComboBox(v2GroupBox);
#endif
        QLabel* trackNumberDigitsLabel = new QLabel(i18n("Track number &digits:"), v2GroupBox);
        m_trackNumberDigitsSpinBox = new QSpinBox(v2GroupBox);
        m_trackNumberDigitsSpinBox->setMaximum(5);
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
        if (textEncodingLabel && m_textEncodingComboBox) {
          m_textEncodingComboBox->insertItem(MiscConfig::TE_ISO8859_1, i18n("ISO-8859-1"));
          m_textEncodingComboBox->insertItem(MiscConfig::TE_UTF16, i18n("UTF16"));
          m_textEncodingComboBox->insertItem(MiscConfig::TE_UTF8, i18n("UTF8"));
          m_textEncodingComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
          textEncodingLabel->setBuddy(m_textEncodingComboBox);
          v2GroupBoxLayout->addWidget(m_genreNotNumericCheckBox, 1, 0, 1, 2);
          v2GroupBoxLayout->addWidget(textEncodingLabel, 2, 0);
          v2GroupBoxLayout->addWidget(m_textEncodingComboBox, 2, 1);
        }
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
        if (id3v2VersionLabel && m_id3v2VersionComboBox) {
          m_id3v2VersionComboBox->insertItem(MiscConfig::ID3v2_3_0, i18n("ID3v2.3.0 (id3lib)"));
          m_id3v2VersionComboBox->insertItem(MiscConfig::ID3v2_4_0, i18n("ID3v2.4.0 (TagLib)"));
          m_id3v2VersionComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
          id3v2VersionLabel->setBuddy(m_id3v2VersionComboBox);
          v2GroupBoxLayout->addWidget(id3v2VersionLabel, 3, 0);
          v2GroupBoxLayout->addWidget(m_id3v2VersionComboBox, 3, 1);
        }
#endif
        if (trackNumberDigitsLabel && m_trackNumberDigitsSpinBox) {
          trackNumberDigitsLabel->setBuddy(m_trackNumberDigitsSpinBox);
          v2GroupBoxLayout->addWidget(trackNumberDigitsLabel, 4, 0);
          v2GroupBoxLayout->addWidget(m_trackNumberDigitsSpinBox, 4, 1);
        }
        vlayout->addWidget(v2GroupBox);
      }
#ifdef HAVE_VORBIS
      QGroupBox* vorbisGroupBox = new QGroupBox(i18n("Ogg/Vorbis"), tagsPage);
      if (vorbisGroupBox) {
        QLabel* commentNameLabel = new QLabel(i18n("Comment field &name:"), vorbisGroupBox);
        m_commentNameComboBox = new QComboBox(vorbisGroupBox);
        QLabel* pictureNameLabel = new QLabel(i18n("&Picture field name:"), vorbisGroupBox);
        m_pictureNameComboBox = new QComboBox(vorbisGroupBox);
        if (commentNameLabel && m_commentNameComboBox &&
            pictureNameLabel && m_pictureNameComboBox) {
          m_commentNameComboBox->setEditable(true);
          QStringList items;
          items += "COMMENT";
          items += "DESCRIPTION";
          m_commentNameComboBox->addItems(items);
          m_commentNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
          commentNameLabel->setBuddy(m_commentNameComboBox);
          m_pictureNameComboBox->addItems(QStringList() << "METADATA_BLOCK_PICTURE" << "COVERART");
          m_pictureNameComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
          pictureNameLabel->setBuddy(m_pictureNameComboBox);
        }
        QGridLayout* vorbisGroupBoxLayout = new QGridLayout(vorbisGroupBox);
        vorbisGroupBoxLayout->setMargin(2);
        vorbisGroupBoxLayout->setSpacing(4);
        vorbisGroupBoxLayout->addWidget(commentNameLabel, 0, 0);
        vorbisGroupBoxLayout->addWidget(m_commentNameComboBox, 0, 1);
        vorbisGroupBoxLayout->addWidget(pictureNameLabel, 1, 0);
        vorbisGroupBoxLayout->addWidget(m_pictureNameComboBox, 1, 1);
        vorbisGroupBox->setLayout(vorbisGroupBoxLayout);
        vlayout->addWidget(vorbisGroupBox);
      }
#endif
      QHBoxLayout* hlayout = new QHBoxLayout;
      if (hlayout) {
        QGroupBox* genresGroupBox = new QGroupBox(i18n("Custom &Genres"), tagsPage);
        if (genresGroupBox) {
          m_onlyCustomGenresCheckBox = new QCheckBox(i18n("&Show only custom genres"), genresGroupBox);
          m_genresEditModel = new QStringListModel(genresGroupBox);
          StringListEdit* genresEdit = new StringListEdit(m_genresEditModel, genresGroupBox);
          QVBoxLayout* vbox = new QVBoxLayout;
          vbox->setMargin(2);
          vbox->addWidget(m_onlyCustomGenresCheckBox);
          vbox->addWidget(genresEdit);
          genresGroupBox->setLayout(vbox);
          hlayout->addWidget(genresGroupBox);
        }
        QString id3FormatTitle(i18n("&Tag Format"));
        m_id3FormatBox = new FormatBox(id3FormatTitle, tagsPage);
        if (m_id3FormatBox) {
          hlayout->addWidget(m_id3FormatBox);
        }
        vlayout->addLayout(hlayout);
      }
    }
#ifdef CONFIG_USE_KDE
    addPage(tagsPage, i18n("Tags"), "applications-multimedia");
#else
    tabWidget->addTab(tagsPage, i18n("&Tags"));
#endif
  }

  QWidget* filesPage = new QWidget;
  if (filesPage) {
    QVBoxLayout* vlayout = new QVBoxLayout(filesPage);
    if (vlayout) {
      vlayout->setMargin(6);
      vlayout->setSpacing(6);
      QGroupBox* saveGroupBox = new QGroupBox(i18n("Save"), filesPage);
      if (saveGroupBox) {
        m_preserveTimeCheckBox = new QCheckBox(i18n("&Preserve file timestamp"), saveGroupBox);
        m_markChangesCheckBox = new QCheckBox(i18n("&Mark changes"), saveGroupBox);
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->setMargin(2);
        vbox->addWidget(m_preserveTimeCheckBox);
        vbox->addWidget(m_markChangesCheckBox);
        saveGroupBox->setLayout(vbox);
        vlayout->addWidget(saveGroupBox);
      }
      QString fnFormatTitle(i18n("&Filename Format"));
      m_fnFormatBox = new FormatBox(fnFormatTitle, filesPage);
      if (m_fnFormatBox) {
        vlayout->addWidget(m_fnFormatBox);
      }
    }
#ifdef CONFIG_USE_KDE
    addPage(filesPage, i18n("Files"), "document-save");
#else
    tabWidget->addTab(filesPage, i18n("&Files"));
#endif
  }

  QWidget* actionsPage = new QWidget;
  if (actionsPage) {
    QVBoxLayout* vlayout = new QVBoxLayout(actionsPage);
    if (vlayout) {
      vlayout->setMargin(6);
      vlayout->setSpacing(6);
      QGroupBox* browserGroupBox = new QGroupBox(i18n("Browser"), actionsPage);
      if (browserGroupBox) {
        QLabel* browserLabel = new QLabel(i18n("Web &browser:"), browserGroupBox);
        m_browserLineEdit = new QLineEdit(browserGroupBox);
        if (browserLabel && m_browserLineEdit) {
          browserLabel->setBuddy(m_browserLineEdit);
        }
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->setMargin(2);
        hbox->addWidget(browserLabel);
        hbox->addWidget(m_browserLineEdit);
        browserGroupBox->setLayout(hbox);
        vlayout->addWidget(browserGroupBox);
      }

      QGroupBox* commandsGroupBox = new QGroupBox(i18n("Context &Menu Commands"), actionsPage);
      if (commandsGroupBox) {
        m_commandsTableModel = new CommandsTableModel(commandsGroupBox);
        m_commandsTable = new ConfigTable(commandsGroupBox);
        m_commandsTable->setModel(m_commandsTableModel);
        m_commandsTable->setHorizontalResizeModes(
          m_commandsTableModel->getHorizontalResizeModes());
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->setMargin(2);
        hbox->addWidget(m_commandsTable);
        commandsGroupBox->setLayout(hbox);
        vlayout->addWidget(commandsGroupBox);
      }
    }
#ifdef CONFIG_USE_KDE
    addPage(actionsPage, i18n("User Actions"), "preferences-other");
#else
    tabWidget->addTab(actionsPage, i18n("&User Actions"));
#endif
  }

  QWidget* networkPage = new QWidget;
  if (networkPage) {
    QVBoxLayout* vlayout = new QVBoxLayout(networkPage);
    if (vlayout) {
      vlayout->setMargin(6);
      vlayout->setSpacing(6);
      QGroupBox* proxyGroupBox = new QGroupBox(i18n("Proxy"), networkPage);
      if (proxyGroupBox) {
        m_proxyCheckBox = new QCheckBox(i18n("&Proxy:"), proxyGroupBox);
        m_proxyLineEdit = new QLineEdit(proxyGroupBox);
        m_proxyAuthenticationCheckBox = new QCheckBox(i18n("&Use authentication with proxy"), proxyGroupBox);
        QLabel* proxyUserNameLabel = new QLabel(i18n("Proxy user &name:"), proxyGroupBox);
        m_proxyUserNameLineEdit = new QLineEdit(proxyGroupBox);
        proxyUserNameLabel->setBuddy(m_proxyUserNameLineEdit);
        QLabel* proxyPasswordLabel = new QLabel(i18n("Proxy pass&word:"), proxyGroupBox);
        m_proxyPasswordLineEdit = new QLineEdit(proxyGroupBox);
        proxyPasswordLabel->setBuddy(m_proxyPasswordLineEdit);
        m_proxyPasswordLineEdit->setEchoMode(QLineEdit::Password);
        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->setMargin(2);
        QHBoxLayout* proxyHbox = new QHBoxLayout;
        proxyHbox->setMargin(2);
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
      }

      QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      vlayout->addItem(vspacer);
    }
#ifdef CONFIG_USE_KDE
    addPage(networkPage, i18n("Network"), "preferences-system-network");
#else
    tabWidget->addTab(networkPage, i18n("&Network"));
#endif
  }

#ifndef CONFIG_USE_KDE
  QWidget* appearancePage = new QWidget;
  if (appearancePage) {
    QVBoxLayout* vlayout = new QVBoxLayout(appearancePage);
    if (vlayout) {
      vlayout->setMargin(6);
      vlayout->setSpacing(6);
      QGridLayout* fontStyleLayout = new QGridLayout;
      fontStyleLayout->setMargin(2);
      fontStyleLayout->setSpacing(4);

      m_useApplicationFontCheckBox = new QCheckBox(i18n("Use custom app&lication font"), appearancePage);
      m_applicationFontButton = new QPushButton(i18n("A&pplication Font..."), appearancePage);
      m_useApplicationStyleCheckBox = new QCheckBox(i18n("Use custom application &style"), appearancePage);
      m_applicationStyleComboBox = new QComboBox(appearancePage);
      if (fontStyleLayout &&
          m_useApplicationFontCheckBox && m_applicationFontButton &&
          m_useApplicationStyleCheckBox && m_applicationStyleComboBox) {
        fontStyleLayout->addWidget(m_useApplicationFontCheckBox, 0, 0);
        fontStyleLayout->addWidget(m_applicationFontButton, 0, 1);
        fontStyleLayout->addWidget(m_useApplicationStyleCheckBox, 1, 0);
        fontStyleLayout->addWidget(m_applicationStyleComboBox, 1, 1);
        m_applicationStyleComboBox->addItem(i18n("Unknown"));
        m_applicationStyleComboBox->addItems(QStyleFactory::keys());
        connect(m_applicationFontButton, SIGNAL(clicked()), this, SLOT(slotSelectFont()));
        connect(m_applicationStyleComboBox, SIGNAL(activated(const QString&)), this, SLOT(slotSelectStyle(const QString&)));
        connect(m_useApplicationFontCheckBox, SIGNAL(toggled(bool)), m_applicationFontButton, SLOT(setEnabled(bool)));
        connect(m_useApplicationStyleCheckBox, SIGNAL(toggled(bool)), m_applicationStyleComboBox, SLOT(setEnabled(bool)));
        vlayout->addLayout(fontStyleLayout);
      }
      QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      vlayout->addItem(vspacer);
    }
    tabWidget->addTab(appearancePage, i18n("&Appearance"));
  }
  m_fontChanged = false;
  m_styleChanged = false;

  topLayout->addWidget(tabWidget);
  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
  QPushButton* okButton = new QPushButton(i18n("&OK"), this);
  QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
  if (hlayout && helpButton && okButton && cancelButton) {
    hlayout->addWidget(helpButton);
    hlayout->addItem(hspacer);
    hlayout->addWidget(okButton);
    hlayout->addWidget(cancelButton);
    okButton->setDefault(true);
    connect(helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(slotRevertFontAndStyle()));
    topLayout->addLayout(hlayout);
  }
#else
  setButtons(Ok | Cancel | Help);
  setHelp("configure-kid3");
#endif
}

/**
 * Destructor.
 */
ConfigDialog::~ConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::setConfig(const FormatConfig* fnCfg,
               const FormatConfig* id3Cfg,
               const MiscConfig* miscCfg)
{
  m_fnFormatBox->fromFormatConfig(fnCfg);
  m_id3FormatBox->fromFormatConfig(id3Cfg);
  m_markTruncationsCheckBox->setChecked(miscCfg->m_markTruncations);
  m_totalNumTracksCheckBox->setChecked(miscCfg->m_enableTotalNumberOfTracks);
  m_preserveTimeCheckBox->setChecked(miscCfg->m_preserveTime);
  m_markChangesCheckBox->setChecked(miscCfg->m_markChanges);
  m_onlyCustomGenresCheckBox->setChecked(miscCfg->m_onlyCustomGenres);
  m_genresEditModel->setStringList(miscCfg->m_customGenres);
  m_commandsTableModel->setCommandList(miscCfg->m_contextMenuCommands);
#ifdef HAVE_VORBIS
  int idx = m_commentNameComboBox->findText(miscCfg->m_commentName);
  if (idx >= 0) {
    m_commentNameComboBox->setCurrentIndex(idx);
  } else {
    m_commentNameComboBox->addItem(miscCfg->m_commentName);
    m_commentNameComboBox->setCurrentIndex(m_commentNameComboBox->count() - 1);
  }
  m_pictureNameComboBox->setCurrentIndex(miscCfg->m_pictureNameItem);
#endif
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  m_genreNotNumericCheckBox->setChecked(miscCfg->m_genreNotNumeric);
  int textEncodingV1Index = TextEncodingV1Latin1Index;
  int index = 0;
  for (QStringList::const_iterator it = m_textEncodingV1List.begin();
       it != m_textEncodingV1List.end();
       ++it) {
    if (getTextEncodingV1CodecName(*it) == miscCfg->m_textEncodingV1) {
      textEncodingV1Index = index;
      break;
    }
    ++index;
  }
  m_textEncodingV1ComboBox->setCurrentIndex(textEncodingV1Index);
  m_textEncodingComboBox->setCurrentIndex(miscCfg->m_textEncoding);
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
  m_id3v2VersionComboBox->setCurrentIndex(miscCfg->m_id3v2Version);
#endif
  m_trackNumberDigitsSpinBox->setValue(miscCfg->m_trackNumberDigits);
  m_browserLineEdit->setText(miscCfg->m_browser);
  m_proxyCheckBox->setChecked(miscCfg->m_useProxy);
  m_proxyLineEdit->setText(miscCfg->m_proxy);
  m_proxyAuthenticationCheckBox->setChecked(miscCfg->m_useProxyAuthentication);
  m_proxyUserNameLineEdit->setText(miscCfg->m_proxyUserName);
  m_proxyPasswordLineEdit->setText(miscCfg->m_proxyPassword);
#ifndef CONFIG_USE_KDE
  m_useApplicationFontCheckBox->setChecked(miscCfg->m_useFont);
  m_applicationFontButton->setEnabled(miscCfg->m_useFont);
  if (miscCfg->m_style.isEmpty()) {
    m_useApplicationStyleCheckBox->setChecked(false);
    m_applicationStyleComboBox->setEnabled(false);
    m_applicationStyleComboBox->setCurrentIndex(0);
  } else {
    m_useApplicationStyleCheckBox->setChecked(true);
    m_applicationStyleComboBox->setEnabled(true);
    int idx = m_applicationStyleComboBox->findText(miscCfg->m_style);
    if (idx >= 0) {
      m_applicationStyleComboBox->setCurrentIndex(idx);
    }
  }

  // store current font and style
  m_font = QApplication::font();
  m_style = miscCfg->m_style;
  m_fontChanged = false;
  m_styleChanged = false;
#endif
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param fnCfg   filename format configuration
 * @param fnCfg   ID3 format configuration
 * @param miscCfg misc. configuration
 */
void ConfigDialog::getConfig(FormatConfig* fnCfg,
               FormatConfig* id3Cfg,
               MiscConfig* miscCfg) const
{
  m_fnFormatBox->toFormatConfig(fnCfg);
  m_id3FormatBox->toFormatConfig(id3Cfg);
  miscCfg->m_markTruncations = m_markTruncationsCheckBox->isChecked();
  miscCfg->m_enableTotalNumberOfTracks = m_totalNumTracksCheckBox->isChecked();
  miscCfg->m_preserveTime = m_preserveTimeCheckBox->isChecked();
  miscCfg->m_markChanges = m_markChangesCheckBox->isChecked();
  miscCfg->m_onlyCustomGenres = m_onlyCustomGenresCheckBox->isChecked();
  miscCfg->m_customGenres = m_genresEditModel->stringList();
  miscCfg->m_contextMenuCommands = m_commandsTableModel->getCommandList();
#ifdef HAVE_VORBIS
  miscCfg->m_commentName = m_commentNameComboBox->currentText();
  miscCfg->m_pictureNameItem = m_pictureNameComboBox->currentIndex();
#endif
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  miscCfg->m_genreNotNumeric = m_genreNotNumericCheckBox->isChecked();
  miscCfg->m_textEncodingV1 =
    getTextEncodingV1CodecName(m_textEncodingV1ComboBox->currentText());
  miscCfg->m_textEncoding = m_textEncodingComboBox->currentIndex();
#endif
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
  miscCfg->m_id3v2Version = m_id3v2VersionComboBox->currentIndex();
#endif
  miscCfg->m_trackNumberDigits = m_trackNumberDigitsSpinBox->value();
  miscCfg->m_browser = m_browserLineEdit->text();
  miscCfg->m_useProxy = m_proxyCheckBox->isChecked();
  miscCfg->m_proxy = m_proxyLineEdit->text();
  miscCfg->m_useProxyAuthentication = m_proxyAuthenticationCheckBox->isChecked();
  miscCfg->m_proxyUserName = m_proxyUserNameLineEdit->text();
  miscCfg->m_proxyPassword = m_proxyPasswordLineEdit->text();
#ifndef CONFIG_USE_KDE
  if (m_useApplicationFontCheckBox->isChecked()) {
    QFont font = QApplication::font();
    miscCfg->m_fontFamily = font.family();
    miscCfg->m_fontSize = font.pointSize();
    miscCfg->m_useFont = true;
  } else {
    miscCfg->m_useFont = false;
  }
  if (!m_useApplicationStyleCheckBox->isChecked() ||
      m_applicationStyleComboBox->currentIndex() == 0) {
    miscCfg->m_style = "";
  } else {
    miscCfg->m_style = m_applicationStyleComboBox->currentText();
  }
#endif
}

/**
 * Show help.
 */
void ConfigDialog::slotHelp()
{
  ContextHelp::displayHelp("configure-kid3");
}

#ifndef CONFIG_USE_KDE
/**
 * Select custom application font.
 */
void ConfigDialog::slotSelectFont()
{
  bool ok;
  QFont font = QFontDialog::getFont(&ok, QApplication::font(), this);
  if (ok) {
    font.setWeight(QFont::Normal);
    font.setItalic(false);
    font.setBold(false);
    font.setUnderline(false);
    font.setOverline(false);
    font.setStrikeOut(false);
    QApplication::setFont(font);
    m_fontChanged = true;
  }
}

/**
 * Select custom application style.
 *
 * @param key style key
 */
void ConfigDialog::slotSelectStyle(const QString& key)
{
  if (key != i18n("Unknown") &&
      QApplication::setStyle(key)) {
    m_styleChanged = true;
  }
}

/**
 * Revert the font and style to the values in the settings.
 */
void ConfigDialog::slotRevertFontAndStyle()
{
  if (m_fontChanged) {
    QApplication::setFont(m_font);
    m_fontChanged = false;
  }
  if (m_styleChanged && !m_style.isEmpty()) {
    QApplication::setStyle(m_style);
    m_styleChanged = false;
  }
}

#else
void ConfigDialog::slotSelectFont() {}
void ConfigDialog::slotSelectStyle(const QString&) {}
void ConfigDialog::slotRevertFontAndStyle() {}
#endif
