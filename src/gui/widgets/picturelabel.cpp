/**
 * \file picturelabel.cpp
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 *
 * Copyright (C) 2009-2024  Urs Fleisch
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
#include <QToolButton>
#include <QStyle>
#include <QAction>
#include <QCoreApplication>
#include "pictureframe.h"

namespace {

class PictureLabelIntern : public QLabel {
public:
  explicit PictureLabelIntern(QWidget* parent = nullptr);
  ~PictureLabelIntern() override = default;
  int heightForWidth(int w) const override;

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
  : QWidget(parent), m_pixmapHash(0), m_index(-1)
{
  setObjectName(QLatin1String("PictureLabel"));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  m_pictureLabel = new PictureLabelIntern;
  layout->addWidget(m_pictureLabel);
  m_sizeLabel = new QLabel;
  m_sizeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  layout->addWidget(m_sizeLabel);

  m_indexWidget = new QWidget;
  auto hlayout = new QHBoxLayout(m_indexWidget);
  hlayout->setContentsMargins(0, 0, 0, 0);
  auto previousAction = new QAction(this);
#if QT_VERSION >= 0x060700
  previousAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious,
      QIcon(style()->standardIcon(QStyle::SP_ArrowBack))));
#else
  previousAction->setIcon(
      QIcon(style()->standardIcon(QStyle::SP_ArrowBack)));
#endif
  previousAction->setText(tr("Previous"));
  connect(previousAction, &QAction::triggered,
          this, &PictureLabel::previous);
  m_previousButton = new QToolButton(m_indexWidget);
  const QString borderlessStyle = QLatin1String("QToolButton { border: 0; }");
  m_previousButton->setStyleSheet(borderlessStyle);
  m_previousButton->setDefaultAction(previousAction);
  hlayout->addWidget(m_previousButton);
  m_indexLabel = new QLabel;
  m_indexLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  hlayout->addWidget(m_indexLabel);
  auto nextAction = new QAction(this);
#if QT_VERSION >= 0x060700
  nextAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext,
      QIcon(style()->standardIcon(QStyle::SP_ArrowForward))));
#else
  nextAction->setIcon(
      QIcon(style()->standardIcon(QStyle::SP_ArrowForward)));
#endif
  nextAction->setText(tr("Next"));
  connect(nextAction, &QAction::triggered,
          this, &PictureLabel::next);
  m_nextButton = new QToolButton(m_indexWidget);
  m_nextButton->setStyleSheet(borderlessStyle);
  m_nextButton->setDefaultAction(nextAction);
  hlayout->addWidget(m_nextButton);
  layout->addWidget(m_indexWidget);

  updateControls();
}

/**
 * Destructor.
 */
PictureLabel::~PictureLabel()
{
  // not inline or default to silence weak-vtables warning
}

/**
 * Set picture data.
 *
 * @param pictures picture frames, empty if no picture is available
 */
void PictureLabel::setData(const QList<PictureFrame>& pictures)
{
  m_pictures = pictures;
  if (m_pictures.isEmpty()) {
    m_index = -1;
  } else if (m_index < 0 || m_index >= m_pictures.size()) {
    m_index = 0;
  }
  updateControls();
}

/**
 * Set picture index.
 *
 * @param index index of picture to show
 */
void PictureLabel::setIndex(int index)
{
  if (index >= 0 && index < m_pictures.size() && index != m_index) {
    m_index = index;
    updateControls();
  }
}

/**
 * Set picture index to last picture.
 */
void PictureLabel::setLastIndex()
{
  setIndex(m_pictures.size() - 1);
}

/**
 * Select previous picture.
 */
void PictureLabel::previous()
{
  setIndex(m_index - 1);
}

/**
 * Select next picture.
 */
void PictureLabel::next()
{
  setIndex(m_index + 1);
}

/**
 * Update UI controls.
 */
void PictureLabel::updateControls()
{
  auto picturesSize = m_pictures.size();
  if (picturesSize >= 2) {
    m_indexLabel->setText(QString(QLatin1String("%1/%2"))
                              .arg(m_index + 1).arg(picturesSize));
    m_previousButton->setEnabled(m_index > 0);
    m_nextButton->setEnabled(m_index < picturesSize - 1);
    m_indexWidget->show();
  } else {
    m_indexWidget->hide();
  }
  if (picturesSize > 0) {
    if (m_index >= 0 && m_index < picturesSize) {
      const auto& picture = m_pictures.at(m_index);
      QString pictureTypeText;
      Frame::PictureType pictureType;
      if (PictureFrame::getPictureType(picture, pictureType)) {
        pictureTypeText =
            QLatin1Char('\n') + PictureFrame::getPictureTypeName(pictureType);
      }
      QByteArray data;
      PictureFrame::getData(picture, data);

      if (!data.isEmpty()) {
        uint hash = qHash(data);
#if QT_VERSION >= 0x050f00
        if (m_pictureLabel->pixmap(Qt::ReturnByValue).isNull() || hash != m_pixmapHash)
#else
        if (!m_pictureLabel->pixmap() || hash != m_pixmapHash)
#endif
        {
          // creating new pixmap
          if (QPixmap pm; pm.loadFromData(data)) {
            int dimension = m_pictureLabel->width();
            if (QPixmap scaledPm = pm.scaled(dimension, dimension, Qt::KeepAspectRatio);
                !scaledPm.isNull()) {
              m_pixmapHash = hash;
              m_pictureLabel->setContentsMargins(0, 0, 0, 0);
              m_pictureLabel->setPixmap(scaledPm);
              m_sizeLabel->setText(QString::number(pm.width()) + QLatin1Char('x') +
                                   QString::number(pm.height()) + pictureTypeText);
            }
          }
        }
      } else {
        m_pictureLabel->clear();
        m_sizeLabel->setText(QLatin1String("0x0") + pictureTypeText);
      }
    }
  } else {
    const char* const msg = QT_TRANSLATE_NOOP("@default", "Drag album\nartwork\nhere");
    m_pictureLabel->setText(QCoreApplication::translate("@default", msg));
    m_sizeLabel->clear();
  }
}
