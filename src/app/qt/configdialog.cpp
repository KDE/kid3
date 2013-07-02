/**
 * \file configdialog.cpp
 * Configuration dialog.
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

#include "configdialog.h"

#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QCheckBox>
#include <QComboBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFontDialog>
#include <QStyleFactory>
#include <QTreeView>
#include <QAction>
#include <QHeaderView>
#include "shortcutsmodel.h"
#include "shortcutsdelegate.h"
#include "contexthelp.h"
#include "mainwindowconfig.h"
#include "configdialogpages.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param shortcutsModel shortcuts model
 * @param mainWindowConfig main window configuration
 */
ConfigDialog::ConfigDialog(QWidget* parent, QString& caption,
                           ShortcutsModel* shortcutsModel,
                           MainWindowConfig* mainWindowConfig) :
  QDialog(parent),
  m_pages(new ConfigDialogPages(this)),
  m_shortcutsModel(shortcutsModel), m_mainWindowConfig(mainWindowConfig)
{
  setObjectName(QLatin1String("ConfigDialog"));
  setWindowTitle(caption);
  setSizeGripEnabled(true);
  QVBoxLayout* topLayout = new QVBoxLayout(this);
  QTabWidget* tabWidget = new QTabWidget(this);
  tabWidget->setUsesScrollButtons(false);

  tabWidget->addTab(m_pages->createTagsPage(), tr("&Tags"));
  tabWidget->addTab(m_pages->createFilesPage(), tr("&Files"));
  tabWidget->addTab(m_pages->createActionsPage(), tr("&User Actions"));
  tabWidget->addTab(m_pages->createNetworkPage(), tr("&Network"));

  {
    QWidget* shortcutsPage = new QWidget;
    QVBoxLayout* vlayout = new QVBoxLayout(shortcutsPage);
    m_shortcutsTreeView = new QTreeView;
    m_shortcutsTreeView->setSelectionMode(QAbstractItemView::NoSelection);
    m_shortcutsTreeView->setItemDelegateForColumn(
          ShortcutsModel::ShortcutColumn, new ShortcutsDelegate(this));
    vlayout->addWidget(m_shortcutsTreeView);
    m_shortcutAlreadyUsedLabel = new QLabel;
    vlayout->addWidget(m_shortcutAlreadyUsedLabel);
    tabWidget->addTab(shortcutsPage, tr("&Keyboard Shortcuts"));

    connect(m_shortcutsModel,
            SIGNAL(shortcutAlreadyUsed(QString,QString,const QAction*)),
            this,
            SLOT(warnAboutAlreadyUsedShortcut(QString,QString,const QAction*)));
    connect(m_shortcutsModel,
            SIGNAL(shortcutSet(QString,QString,const QAction*)),
            this,
            SLOT(clearAlreadyUsedShortcutWarning()));
    connect(this, SIGNAL(rejected()),
            m_shortcutsModel, SLOT(discardChangedShortcuts()));
    m_shortcutsTreeView->setModel(m_shortcutsModel);
    m_shortcutsTreeView->expandAll();
    m_shortcutsTreeView->resizeColumnToContents(ShortcutsModel::ActionColumn);
#ifdef Q_OS_MAC
    m_shortcutsTreeView->header()->setStretchLastSection(false);
#endif
  }

  {
    QWidget* appearancePage = new QWidget;
    QVBoxLayout* vlayout = new QVBoxLayout(appearancePage);
    QGridLayout* fontStyleLayout = new QGridLayout;

    m_useApplicationFontCheckBox = new QCheckBox(tr("Use custom app&lication font"), appearancePage);
    m_applicationFontButton = new QPushButton(tr("A&pplication Font..."), appearancePage);
    m_useApplicationStyleCheckBox = new QCheckBox(tr("Use custom application &style"), appearancePage);
    m_applicationStyleComboBox = new QComboBox(appearancePage);
    fontStyleLayout->addWidget(m_useApplicationFontCheckBox, 0, 0);
    fontStyleLayout->addWidget(m_applicationFontButton, 0, 1);
    fontStyleLayout->addWidget(m_useApplicationStyleCheckBox, 1, 0);
    fontStyleLayout->addWidget(m_applicationStyleComboBox, 1, 1);
    m_applicationStyleComboBox->addItem(tr("Unknown"));
    m_applicationStyleComboBox->addItems(QStyleFactory::keys());
    connect(m_applicationFontButton, SIGNAL(clicked()), this, SLOT(slotSelectFont()));
    connect(m_applicationStyleComboBox, SIGNAL(activated(const QString&)), this, SLOT(slotSelectStyle(const QString&)));
    connect(m_useApplicationFontCheckBox, SIGNAL(toggled(bool)), m_applicationFontButton, SLOT(setEnabled(bool)));
    connect(m_useApplicationStyleCheckBox, SIGNAL(toggled(bool)), m_applicationStyleComboBox, SLOT(setEnabled(bool)));
    vlayout->addLayout(fontStyleLayout);

    m_useNativeDialogsCheckBox =
        new QCheckBox(tr("Use native system file &dialogs"), appearancePage);
    vlayout->addWidget(m_useNativeDialogsCheckBox);
    QSpacerItem* vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlayout->addItem(vspacer);
    tabWidget->addTab(appearancePage, tr("&Appearance"));
  }
  m_fontChanged = false;
  m_styleChanged = false;

  topLayout->addWidget(tabWidget);
  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  QPushButton* okButton = new QPushButton(tr("&OK"), this);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
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

/**
 * Destructor.
 */
ConfigDialog::~ConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 *
 * @param cfg configuration
 */
void ConfigDialog::setConfig(const ConfigStore* cfg)
{
  m_pages->setConfig(cfg);

  m_useApplicationFontCheckBox->setChecked(m_mainWindowConfig->m_useFont);
  m_applicationFontButton->setEnabled(m_mainWindowConfig->m_useFont);
  if (m_mainWindowConfig->m_style.isEmpty()) {
    m_useApplicationStyleCheckBox->setChecked(false);
    m_applicationStyleComboBox->setEnabled(false);
    m_applicationStyleComboBox->setCurrentIndex(0);
  } else {
    m_useApplicationStyleCheckBox->setChecked(true);
    m_applicationStyleComboBox->setEnabled(true);
    int idx = m_applicationStyleComboBox->findText(m_mainWindowConfig->m_style);
    if (idx >= 0) {
      m_applicationStyleComboBox->setCurrentIndex(idx);
    }
  }

  // store current font and style
  m_font = QApplication::font();
  m_style = m_mainWindowConfig->m_style;
  m_fontChanged = false;
  m_styleChanged = false;

  m_useNativeDialogsCheckBox->setChecked(!m_mainWindowConfig->m_dontUseNativeDialogs);
}

/**
 * Get values from dialog and store them in the current configuration.
 *
 * @param cfg configuration
 */
void ConfigDialog::getConfig(ConfigStore* cfg) const
{
  m_pages->getConfig(cfg);

  m_shortcutsModel->assignChangedShortcuts();
  if (m_useApplicationFontCheckBox->isChecked()) {
    QFont font = QApplication::font();
    m_mainWindowConfig->m_fontFamily = font.family();
    m_mainWindowConfig->m_fontSize = font.pointSize();
    m_mainWindowConfig->m_useFont = true;
  } else {
    m_mainWindowConfig->m_useFont = false;
  }
  if (!m_useApplicationStyleCheckBox->isChecked() ||
      m_applicationStyleComboBox->currentIndex() == 0) {
    m_mainWindowConfig->m_style = QLatin1String("");
  } else {
    m_mainWindowConfig->m_style = m_applicationStyleComboBox->currentText();
  }
  m_mainWindowConfig->m_dontUseNativeDialogs =
      !m_useNativeDialogsCheckBox->isChecked();
}

/**
 * Show help.
 */
void ConfigDialog::slotHelp()
{
  ContextHelp::displayHelp(QLatin1String("configure-kid3"));
}

/**
 * Display warning that keyboard shortcut is already used.
 *
 * @param key string representation of key sequence
 * @param context context of action
 * @param action action using @a key
 */
void ConfigDialog::warnAboutAlreadyUsedShortcut(
    const QString& key, const QString& context, const QAction* action)
{
  m_shortcutAlreadyUsedLabel->setText(
        tr("The keyboard shortcut '%1' is already assigned to '%2'.").
        arg(key).
        arg(context + QLatin1Char('/') + (action ? action->text().remove(QLatin1Char('&')) : QLatin1String("?"))));
}

/**
 * Clear warning about already used keyboard shortcut.
 */
void ConfigDialog::clearAlreadyUsedShortcutWarning()
{
  m_shortcutAlreadyUsedLabel->clear();
}

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
  if (key != tr("Unknown") &&
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
