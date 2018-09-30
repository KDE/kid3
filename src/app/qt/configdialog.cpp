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
 * @param platformTools platform specific tools
 * @param parent  parent widget
 * @param caption dialog title
 * @param shortcutsModel shortcuts model
 */
ConfigDialog::ConfigDialog(IPlatformTools* platformTools, QWidget* parent,
                           QString& caption, ShortcutsModel* shortcutsModel) :
  QDialog(parent),
  m_pages(new ConfigDialogPages(platformTools, this)),
  m_shortcutsModel(shortcutsModel)
{
  setObjectName(QLatin1String("ConfigDialog"));
  setWindowTitle(caption);
  setSizeGripEnabled(true);
  auto topLayout = new QVBoxLayout(this);
  auto tabWidget = new QTabWidget(this);
  tabWidget->setUsesScrollButtons(false);

  tabWidget->addTab(m_pages->createTagsPage(), tr("&Tags"));
  tabWidget->addTab(m_pages->createFilesPage(), tr("&Files"));
  tabWidget->addTab(m_pages->createActionsPage(), tr("&User Actions"));
  tabWidget->addTab(m_pages->createNetworkPage(), tr("&Network"));
  tabWidget->addTab(m_pages->createPluginsPage(), tr("&Plugins"));

  {
    QWidget* shortcutsPage = new QWidget;
    auto vlayout = new QVBoxLayout(shortcutsPage);
    m_shortcutsTreeView = new QTreeView;
    m_shortcutsTreeView->setSelectionMode(QAbstractItemView::NoSelection);
    m_shortcutsTreeView->setItemDelegateForColumn(
          ShortcutsModel::ShortcutColumn, new ShortcutsDelegate(this));
    vlayout->addWidget(m_shortcutsTreeView);
    m_shortcutAlreadyUsedLabel = new QLabel;
    vlayout->addWidget(m_shortcutAlreadyUsedLabel);
    tabWidget->addTab(shortcutsPage, tr("&Keyboard Shortcuts"));

    connect(m_shortcutsModel,
            &ShortcutsModel::shortcutAlreadyUsed,
            this,
            &ConfigDialog::warnAboutAlreadyUsedShortcut);
    connect(m_shortcutsModel,
            &ShortcutsModel::shortcutSet,
            this,
            &ConfigDialog::clearAlreadyUsedShortcutWarning);
    connect(this, &QDialog::rejected,
            m_shortcutsModel, &ShortcutsModel::discardChangedShortcuts);
    m_shortcutsTreeView->setModel(m_shortcutsModel);
    m_shortcutsTreeView->expandAll();
    m_shortcutsTreeView->resizeColumnToContents(ShortcutsModel::ActionColumn);
#ifdef Q_OS_MAC
    m_shortcutsTreeView->header()->setStretchLastSection(false);
#endif
  }

  {
    QWidget* appearancePage = new QWidget;
    auto vlayout = new QVBoxLayout(appearancePage);
    auto fontStyleLayout = new QGridLayout;

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
    connect(m_applicationFontButton, &QAbstractButton::clicked, this, &ConfigDialog::slotSelectFont);
    connect(m_applicationStyleComboBox,
            static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),
            this, &ConfigDialog::slotSelectStyle);
    connect(m_useApplicationFontCheckBox, &QAbstractButton::toggled, m_applicationFontButton, &QWidget::setEnabled);
    connect(m_useApplicationStyleCheckBox, &QAbstractButton::toggled, m_applicationStyleComboBox, &QWidget::setEnabled);
    vlayout->addLayout(fontStyleLayout);

    m_useNativeDialogsCheckBox =
        new QCheckBox(tr("Use native system file &dialogs"), appearancePage);
    vlayout->addWidget(m_useNativeDialogsCheckBox);
    auto vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlayout->addItem(vspacer);
    tabWidget->addTab(appearancePage, tr("&Appearance"));
  }
  m_fontChanged = false;
  m_styleChanged = false;

  topLayout->addWidget(tabWidget);
  auto hlayout = new QHBoxLayout;
  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  QPushButton* defaultsButton = new QPushButton(tr("Restore Defaults"), this);
  QPushButton* okButton = new QPushButton(tr("&OK"), this);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
  hlayout->addWidget(helpButton);
  hlayout->addWidget(defaultsButton);
  hlayout->addItem(hspacer);
  hlayout->addWidget(okButton);
  hlayout->addWidget(cancelButton);
  okButton->setDefault(true);
  connect(helpButton, &QAbstractButton::clicked, this, &ConfigDialog::slotHelp);
  connect(defaultsButton, &QAbstractButton::clicked,
          m_pages, &ConfigDialogPages::setDefaultConfig);
  connect(defaultsButton, &QAbstractButton::clicked,
          this, &ConfigDialog::setDefaultConfig);
  connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
  connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
  connect(cancelButton, &QAbstractButton::clicked, this, &ConfigDialog::slotRevertFontAndStyle);
  topLayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
ConfigDialog::~ConfigDialog()
{}

/**
 * Set values in dialog from current configuration.
 */
void ConfigDialog::setConfig()
{
  m_pages->setConfig();

  const MainWindowConfig& mainWindowConfig = MainWindowConfig::instance();
  setConfigs(mainWindowConfig);
}

/**
 * Set values in dialog from given configuration.
 */
void ConfigDialog::setConfigs(const MainWindowConfig& mainWindowConfig)
{
  m_useApplicationFontCheckBox->setChecked(mainWindowConfig.useFont());
  m_applicationFontButton->setEnabled(mainWindowConfig.useFont());
  if (mainWindowConfig.style().isEmpty()) {
    m_useApplicationStyleCheckBox->setChecked(false);
    m_applicationStyleComboBox->setEnabled(false);
    m_applicationStyleComboBox->setCurrentIndex(0);
  } else {
    m_useApplicationStyleCheckBox->setChecked(true);
    m_applicationStyleComboBox->setEnabled(true);
    int idx = m_applicationStyleComboBox->findText(mainWindowConfig.style());
    if (idx >= 0) {
      m_applicationStyleComboBox->setCurrentIndex(idx);
    }
  }

  // store current font and style
  m_font = QApplication::font();
  m_style = mainWindowConfig.style();
  m_fontChanged = false;
  m_styleChanged = false;

  m_useNativeDialogsCheckBox->setChecked(!mainWindowConfig.dontUseNativeDialogs());
}

/**
 * Get values from dialog and store them in the current configuration.
 */
void ConfigDialog::getConfig() const
{
  m_pages->getConfig();

  MainWindowConfig& mainWindowConfig = MainWindowConfig::instance();
  m_shortcutsModel->assignChangedShortcuts();
  if (m_useApplicationFontCheckBox->isChecked()) {
    QFont font = QApplication::font();
    mainWindowConfig.setFontFamily(font.family());
    mainWindowConfig.setFontSize(font.pointSize());
    mainWindowConfig.setUseFont(true);
  } else {
    mainWindowConfig.setUseFont(false);
  }
  if (!m_useApplicationStyleCheckBox->isChecked() ||
      m_applicationStyleComboBox->currentIndex() == 0) {
    mainWindowConfig.setStyle(QLatin1String(""));
  } else {
    mainWindowConfig.setStyle(m_applicationStyleComboBox->currentText());
  }
  mainWindowConfig.setDontUseNativeDialogs(
      !m_useNativeDialogsCheckBox->isChecked());
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
 * Set additional configurations to their defaults.
 */
void ConfigDialog::setDefaultConfig()
{
  m_shortcutsModel->clearShortcuts();
  m_shortcutsTreeView->expandAll();

  MainWindowConfig mainWindowConfig;
  setConfigs(mainWindowConfig);
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
