/**
 * \file browserdialog.cpp
 * Help browser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2009  Urs Fleisch
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
#include "qtcompatmac.h"
#include <QVBoxLayout>

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param caption dialog title
 */
BrowserDialog::BrowserDialog(QWidget* parent, QString& caption)
  : QDialog(parent)
{
  setWindowTitle(caption);
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  if (!vlayout) {
    return ;
  }
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
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  QPushButton* backButton = new QPushButton(i18n("&Back"), this);
  QPushButton* forwardButton = new QPushButton(i18n("&Forward"), this);
  QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
  if (hlayout && backButton && forwardButton && closeButton) {
    hlayout->addWidget(backButton);
    hlayout->addWidget(forwardButton);
    hlayout->addItem(hspacer);
    hlayout->addWidget(closeButton);
    closeButton->setDefault(true);
    backButton->setEnabled(false);
    forwardButton->setEnabled(false);
    connect(backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
    connect(forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
    connect(m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
    connect(m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
    vlayout->addLayout(hlayout);
  }
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

#endif // CONFIG_USE_KDE
