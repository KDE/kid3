/**
 * \file picturelabel.cpp
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 *
 * Copyright (C) 2009-2018  Urs Fleisch
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

#include "picturelabel.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHash>
#include <QByteArray>
#include <QPixmap>
#include <QCoreApplication>

namespace {

class PictureLabelIntern : public QLabel {
public:
  explicit PictureLabelIntern(QWidget* parent = nullptr);
  virtual ~PictureLabelIntern() override = default;
  virtual int heightForWidth(int w) const override;

private:
  Q_DISABLE_COPY(PictureLabelIntern)
};

PictureLabelIntern::PictureLabelIntern(QWidget* parent) : QLabel(parent)
{
  setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  setWordWrap(true);
}

int PictureLabelIntern::heightForWidth(int w) const
{
  return w;
}

}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
PictureLabel::PictureLabel(QWidget* parent)
  : QWidget(parent), m_pixmapHash(0)
{
  setObjectName(QLatin1String("PictureLabel"));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  m_pictureLabel = new PictureLabelIntern;
  layout->addWidget(m_pictureLabel);
  m_sizeLabel = new QLabel;
  m_sizeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addWidget(m_sizeLabel);
  clearPicture();
}

/**
 * Destructor.
 */
PictureLabel::~PictureLabel()
{
  // not inline or default to silence weak-vtables warning
}

/**
 * Clear picture.
 */
void PictureLabel::clearPicture()
{
  const char* const msg = QT_TRANSLATE_NOOP("@default", "Drag album\nartwork\nhere");
  m_pictureLabel->setText(QCoreApplication::translate("@default", msg));
  m_sizeLabel->clear();
}

/**
 * Set picture data.
 *
 * @param data picture data, empty if no picture is available
 */
void PictureLabel::setData(const QByteArray& data)
{
  if (!data.isEmpty()) {
    uint hash = qHash(data);
    if (m_pictureLabel->pixmap() && hash == m_pixmapHash)
      return; // keep existing pixmap

    // creating new pixmap
    QPixmap pm;
    if (pm.loadFromData(data)) {
      int dimension = m_pictureLabel->width();
      QPixmap scaledPm = pm.scaled(dimension, dimension, Qt::KeepAspectRatio);
      if (!scaledPm.isNull()) {
        m_pixmapHash = hash;
        m_pictureLabel->setContentsMargins(0, 0, 0, 0);
        m_pictureLabel->setPixmap(scaledPm);
        m_sizeLabel->setText(QString::number(pm.width()) + QLatin1Char('x') +
                             QString::number(pm.height()));
        return;
      }
    }
  }

  clearPicture();
}
