/**
 * \file imageviewer.cpp
 * Window to view image.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
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

#include "imageviewer.h"
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param img    image to display in window
 */
ImageViewer::ImageViewer(QWidget* parent, const QImage& img) :
  QDialog(parent)
{
  setModal(true);
  setWindowTitle(i18n("View Picture"));
  QVBoxLayout* vlayout = new QVBoxLayout(this);
  if (!vlayout) {
    return ;
  }
  vlayout->setSpacing(6);
  vlayout->setMargin(6);
  QHBoxLayout* hlayout = new QHBoxLayout;
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  m_image = new QLabel(this);
  QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
  if (vlayout && hlayout && m_image && closeButton) {
    m_image->setScaledContents(true);
    QSize imageSize(img.size());
    QSize desktopSize(QApplication::desktop()->availableGeometry().size());
    desktopSize -= QSize(12, 12);
    if (imageSize.width() > desktopSize.width() ||
        imageSize.height() > desktopSize.height()) {
      m_image->setPixmap(QPixmap::fromImage(img.scaled(desktopSize, Qt::KeepAspectRatio)));
    } else {
      m_image->setPixmap(QPixmap::fromImage(img));
    }
    vlayout->addWidget(m_image);
    hlayout->addItem(hspacer);
    hlayout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
    vlayout->addLayout(hlayout);
  }
}


