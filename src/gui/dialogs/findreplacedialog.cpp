/**
 * \file findreplacedialog.h
 * Find and replace dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 06 Feb 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "findreplacedialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCompleter>
#include <QGroupBox>
#include <QCheckBox>
#include <QListView>
#include <QStatusBar>
#include "checkablestringlistmodel.h"
#include "frame.h"
#include "trackdatamodel.h"
#include "findreplaceconfig.h"
#include "contexthelp.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 */
FindReplaceDialog::FindReplaceDialog(QWidget* parent) : QDialog(parent)
{
  setObjectName(QLatin1String("FindReplaceDialog"));
  setWindowTitle(tr("Find and Replace"));

  auto vlayout = new QVBoxLayout(this);

  auto findReplaceLayout = new QGridLayout;
  QLabel* findLabel = new QLabel(tr("F&ind:"));
  m_findEdit = new QComboBox;
  m_findEdit->setEditable(true);
  m_findEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);
  connect(m_findEdit->lineEdit(), &QLineEdit::returnPressed,
          this, &FindReplaceDialog::onReturnPressedInFind);
  findLabel->setBuddy(m_findEdit);
  m_findButton = new QPushButton(tr("&Find"));
  m_findButton->setAutoDefault(true);
  connect(m_findButton, &QAbstractButton::clicked, this, &FindReplaceDialog::find);
  m_replaceLabel = new QLabel(tr("Re&place:"));
  m_replaceEdit = new QComboBox;
  m_replaceEdit->setEditable(true);
  m_replaceEdit->completer()->setCaseSensitivity(Qt::CaseSensitive);
  connect(m_replaceEdit->lineEdit(), &QLineEdit::returnPressed,
          this, &FindReplaceDialog::onReturnPressedInReplace);
  m_replaceLabel->setBuddy(m_replaceEdit);
  m_replaceButton = new QPushButton(tr("&Replace"));
  m_replaceButton->setAutoDefault(true);
  connect(m_replaceButton, &QAbstractButton::clicked, this, &FindReplaceDialog::replace);
  m_replaceAllButton = new QPushButton(tr("Replace &all"));
  m_replaceAllButton->setAutoDefault(false);
  connect(m_replaceAllButton, &QAbstractButton::clicked, this, &FindReplaceDialog::replaceAll);

  findReplaceLayout->addWidget(findLabel, 0, 0);
  findReplaceLayout->addWidget(m_findEdit, 0, 1);
  findReplaceLayout->addWidget(m_findButton, 0, 2);
  findReplaceLayout->addWidget(m_replaceLabel, 1, 0);
  findReplaceLayout->addWidget(m_replaceEdit, 1, 1);
  findReplaceLayout->addWidget(m_replaceButton, 1, 2);
  findReplaceLayout->addWidget(m_replaceAllButton, 2, 2);
  findReplaceLayout->setColumnStretch(1, 1);
  vlayout->addLayout(findReplaceLayout);

  auto optionsTagsLayout = new QHBoxLayout;
  QGroupBox* optionsBox = new QGroupBox(tr("Options"));
  auto optionsLayout = new QVBoxLayout(optionsBox);
  m_matchCaseCheckBox = new QCheckBox(tr("&Match case"));
  optionsLayout->addWidget(m_matchCaseCheckBox);
  m_backwardsCheckBox = new QCheckBox(tr("&Backwards"));
  optionsLayout->addWidget(m_backwardsCheckBox);
  m_regExpCheckBox = new QCheckBox(tr("Regular &expression"));
  optionsLayout->addWidget(m_regExpCheckBox);
  optionsLayout->addStretch();
  optionsTagsLayout->addWidget(optionsBox);

  QGroupBox* tagsGroupBox = new QGroupBox(tr("&Tags"));
  auto tagsLayout = new QVBoxLayout(tagsGroupBox);
  m_allFramesCheckBox = new QCheckBox(tr("Select a&ll"));
  m_allFramesCheckBox->setChecked(true);
  tagsLayout->addWidget(m_allFramesCheckBox);
  auto tagsListView = new QListView;
  tagsListView->setDisabled(true);
  connect(m_allFramesCheckBox, &QAbstractButton::toggled,
          tagsListView, &QWidget::setDisabled);
  m_tagsModel = new CheckableStringListModel(tagsGroupBox);
  QStringList unifiedFrameNames;
  unifiedFrameNames.append(tr("Filename"));
  for (int i = Frame::FT_FirstFrame; i< Frame::FT_LastFrame; ++i) {
    unifiedFrameNames.append(
        Frame::ExtendedType(static_cast<Frame::Type>(i)).getTranslatedName());
  }
  m_tagsModel->setStringList(unifiedFrameNames);
  tagsListView->setModel(m_tagsModel);
  tagsLayout->addWidget(tagsListView);
  optionsTagsLayout->addWidget(tagsGroupBox);
  vlayout->addLayout(optionsTagsLayout);

  auto hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"));
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &FindReplaceDialog::showHelp);

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"));
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &FindReplaceDialog::saveConfig);

  hlayout->addStretch();

  QPushButton* closeButton = new QPushButton(tr("&Close"));
  closeButton->setAutoDefault(false);
  hlayout->addWidget(closeButton);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::reject);

  vlayout->addLayout(hlayout);

  m_statusBar = new QStatusBar;
  vlayout->addWidget(m_statusBar);

  setTabOrder(this, m_findEdit);
  setTabOrder(m_findEdit, m_replaceEdit);
  setTabOrder(m_replaceEdit, m_findButton);
  setTabOrder(m_findButton, m_replaceButton);

  readConfig();
}

/**
 * Destructor.
 */
FindReplaceDialog::~FindReplaceDialog()
{
}

/**
 * Initialize dialog before it is displayed.
 * @param findOnly true to display only find part of dialog
 */
void FindReplaceDialog::init(bool findOnly)
{
  m_statusBar->clearMessage();
  m_findEdit->setFocus();
  setWindowTitle(findOnly ? tr("Find") : tr("Find and Replace"));
  m_replaceLabel->setHidden(findOnly);
  m_replaceEdit->setHidden(findOnly);
  m_replaceButton->setHidden(findOnly);
  m_replaceAllButton->setHidden(findOnly);
}


/**
 * Get search parameters from GUI controls.
 * @param params the search parameters are returned here
 */
void FindReplaceDialog::getParameters(TagSearcher::Parameters& params) const
{
  params.setSearchText(m_findEdit->currentText());
  params.setReplaceText(m_replaceEdit->currentText());
  TagSearcher::SearchFlags flags;
  if (m_matchCaseCheckBox->isChecked())  flags |= TagSearcher::CaseSensitive;
  if (m_backwardsCheckBox->isChecked())  flags |= TagSearcher::Backwards;
  if (m_regExpCheckBox->isChecked())     flags |= TagSearcher::RegExp;
  if (m_allFramesCheckBox->isChecked())  flags |= TagSearcher::AllFrames;
  params.setFlags(flags);
  quint64 frameMask = m_tagsModel->getBitMask();
  bool fileNameIsSet = frameMask & 1;
  frameMask >>= 1;
  if (fileNameIsSet) {
    frameMask |= 1ULL << TrackDataModel::FT_FileName;
  }
  params.setFrameMask(frameMask);
}

/**
 * Set search parameters in GUI controls.
 * @param params search parameters
 */
void FindReplaceDialog::setParameters(const TagSearcher::Parameters& params)
{
  if (!params.getSearchText().isEmpty()) {
    m_findEdit->lineEdit()->setText(params.getSearchText());
  }
  if (!params.getReplaceText().isEmpty()) {
    m_replaceEdit->lineEdit()->setText(params.getReplaceText());
  }
  TagSearcher::SearchFlags flags = params.getFlags();
  m_matchCaseCheckBox->setChecked(flags & TagSearcher::CaseSensitive);
  m_backwardsCheckBox->setChecked(flags & TagSearcher::Backwards);
  m_regExpCheckBox->setChecked(flags & TagSearcher::RegExp);
  m_allFramesCheckBox->setChecked(flags & TagSearcher::AllFrames);
  quint64 frameMask = params.getFrameMask();
  bool fileNameIsSet = (frameMask & (1ULL << TrackDataModel::FT_FileName)) != 0;
  frameMask <<= 1;
  if (fileNameIsSet) {
    frameMask |= 1;
  }
  m_tagsModel->setBitMask(frameMask);
}

/**
 * Find next occurrence.
 */
void FindReplaceDialog::find()
{
  QString text(m_findEdit->currentText());
  if (!text.isEmpty()) {
    TagSearcher::Parameters params;
    getParameters(params);
    emit findRequested(params);
  }
}

/**
 * Replace found text.
 */
void FindReplaceDialog::replace()
{
  TagSearcher::Parameters params;
  getParameters(params);
  emit replaceRequested(params);
}

/**
 * Replace all occurrences.
 */
void FindReplaceDialog::replaceAll()
{
  TagSearcher::Parameters params;
  getParameters(params);
  emit replaceAllRequested(params);
}

/**
 * Read the local settings from the configuration.
 */
void FindReplaceDialog::readConfig()
{
  const FindReplaceConfig& findCfg = FindReplaceConfig::instance();
  setParameters(findCfg.getParameters());
  QByteArray geometry = findCfg.windowGeometry();
  if (!geometry.isEmpty()) {
    restoreGeometry(geometry);
  }
}

/**
 * Save the local settings to the configuration.
 */
void FindReplaceDialog::saveConfig()
{
  FindReplaceConfig& findCfg = FindReplaceConfig::instance();
  TagSearcher::Parameters params;
  getParameters(params);
  findCfg.setParameters(params);
  QByteArray geometry = saveGeometry();
  findCfg.setWindowGeometry(geometry);
  restoreGeometry(geometry); // Keep geometry when dialog is reopened
}

/**
 * Show help.
 */
void FindReplaceDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("find-replace"));
}

/**
 * Show progress message.
 * @param msg message
 */
void FindReplaceDialog::showProgress(const QString& msg)
{
  m_statusBar->showMessage(msg);
}

/**
 * Called when Return is pressed in the Find combo box.
 */
void FindReplaceDialog::onReturnPressedInFind()
{
  m_findButton->setDefault(true);
}

/**
 * Called when Return is pressed in the Replace combo box.
 */
void FindReplaceDialog::onReturnPressedInReplace()
{
  m_replaceButton->setDefault(true);
}
