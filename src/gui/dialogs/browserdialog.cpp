/**
 * \file browserdialog.cpp
 * Help browser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2012  Urs Fleisch
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

#include "browserdialog.h"

#ifndef CONFIG_USE_KDE
#include <QTextBrowser>
#include <QLocale>
#include <QDir>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QStyle>
#include <QAction>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param caption dialog title
 */
BrowserDialog::BrowserDialog(QWidget* parent, QString& caption)
  : QDialog(parent)
{
  setObjectName("BrowserDialog");
  setWindowTitle(caption);
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setSpacing(6);
  vlayout->setMargin(6);

  QString lang(QLocale::system().name().left(2));
  QStringList docPaths;
#ifdef CFG_DOCDIR
  docPaths += QString(CFG_DOCDIR) + "/kid3_" + lang + ".html";
  docPaths += QString(CFG_DOCDIR) + "/kid3_en.html";
#endif
  docPaths += QDir::currentPath() + "/kid3_" + lang + ".html";
  docPaths += QDir::currentPath() + "/kid3_en.html";
  for (QStringList::const_iterator it = docPaths.begin();
       it != docPaths.end();
       ++it) {
    m_filename = *it;
    if (QFile::exists(m_filename)) break;
  }
  m_textBrowser = new QTextBrowser(this);
  m_textBrowser->setSource(QUrl::fromLocalFile(m_filename));
  vlayout->addWidget(m_textBrowser);

  QHBoxLayout* hlayout = new QHBoxLayout;
  QPushButton* backButton = new QPushButton(i18n("&Back"), this);
  backButton->setEnabled(false);
  connect(backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
  connect(m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
  hlayout->addWidget(backButton);
  QPushButton* forwardButton = new QPushButton(i18n("&Forward"), this);
  forwardButton->setEnabled(false);
  connect(forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
  connect(m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
  hlayout->addWidget(forwardButton);
  QLabel* findLabel = new QLabel(i18n("&Find:"), this);
  hlayout->addWidget(findLabel);
  m_findLineEdit = new QLineEdit(this);
  m_findLineEdit->setFocus();
  findLabel->setBuddy(m_findLineEdit);
  connect(m_findLineEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
  hlayout->addWidget(m_findLineEdit);
  QAction* findAction = new QAction(this);
  findAction->setShortcut(QKeySequence::Find);
  connect(findAction, SIGNAL(triggered()), m_findLineEdit, SLOT(setFocus()));
  m_findLineEdit->addAction(findAction);
  QAction* findPreviousAction = new QAction(this);
  findPreviousAction->setIcon(
        QIcon(style()->standardIcon(QStyle::SP_ArrowBack)));
  findPreviousAction->setText(i18n("Find Previous"));
  findPreviousAction->setShortcut(QKeySequence::FindPrevious);
  connect(findPreviousAction, SIGNAL(triggered()),
          this, SLOT(findPrevious()));
  QToolButton* findPreviousButton = new QToolButton(this);
  findPreviousButton->setDefaultAction(findPreviousAction);
  hlayout->addWidget(findPreviousButton);
  QAction* findNextAction = new QAction(this);
  findNextAction->setIcon(
        QIcon(style()->standardIcon(QStyle::SP_ArrowForward)));
  findNextAction->setText(i18n("Find Next"));
  findNextAction->setShortcut(QKeySequence::FindNext);
  connect(findNextAction, SIGNAL(triggered()),
          this, SLOT(findNext()));
  QToolButton* findNextButton = new QToolButton(this);
  findNextButton->setDefaultAction(findNextAction);
  hlayout->addWidget(findNextButton);
  hlayout->addStretch();
  QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
  closeButton->setAutoDefault(false);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  hlayout->addWidget(closeButton);
  vlayout->addLayout(hlayout);
  resize(500, 500);
}

/**
 * Destructor.
 */
BrowserDialog::~BrowserDialog()
{}

/**
 * Display help document at anchor.
 *
 * @param anchor anchor
 */
void BrowserDialog::goToAnchor(const QString& anchor)
{
  QUrl url = QUrl::fromLocalFile(m_filename);
  url.setFragment(anchor);
  m_textBrowser->setSource(url);
}

/**
 * Find previous occurrence of search text.
 */
void BrowserDialog::findPrevious()
{
  m_textBrowser->find(m_findLineEdit->text(), QTextDocument::FindBackward);
}

/**
 * Find next occurrence of search text.
 */
void BrowserDialog::findNext()
{
  m_textBrowser->find(m_findLineEdit->text());
}

#endif // CONFIG_USE_KDE
