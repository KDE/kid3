/**
 * \file frameitemdelegate.cpp
 * Delegate for table widget items.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "frameitemdelegate.h"
#include <cmath>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QMouseEvent>
#include "frametablemodel.h"
#include "genremodel.h"
#include "formatconfig.h"
#include "tagconfig.h"
#include "tracknumbervalidator.h"
#include "framenotice.h"

namespace {

const int MAX_STAR_COUNT = 5;
const int STAR_SCALE_FACTOR = 20;

QString ratingTypeName(const QModelIndex& index) {
  QString name = index.data(FrameTableModel::InternalNameRole).toString();
  if (name.startsWith(QLatin1String("POPM"))) {
    name.truncate(4);
    QVariantList fieldIds = index.data(FrameTableModel::FieldIdsRole).toList();
    int emailIdx = fieldIds.indexOf(Frame::ID_Email);
    if (emailIdx != -1) {
      QVariantList fieldValues =
          index.data(FrameTableModel::FieldValuesRole).toList();
      QString emailValue;
      if (emailIdx < fieldValues.size() &&
          !(emailValue = fieldValues.at(emailIdx).toString()).isEmpty()) {
        name += QLatin1Char('.');
        name += emailValue;
      }
    }
  }
  return name;
}

int starCountFromRating(int rating, const QModelIndex& index)
{
  return rating < 1
      ? 0
      : TagConfig::instance().starCountFromRating(rating, ratingTypeName(index));
}

int starCountToRating(int starCount, const QModelIndex& index) {
  return starCount < 1
      ? 0
      : TagConfig::instance().starCountToRating(starCount, ratingTypeName(index));
}


/**
 * Validator for date/time values.
 */
class DateTimeValidator : public QValidator {
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit DateTimeValidator(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~DateTimeValidator() override = default;

  /**
   * Validate input string.
   * @param input input string
   * @param pos current position in input string
   * @return validation state.
   */
  virtual State validate(QString& input, int& pos) const override;

  /**
   * Attempt to change input string to be valid.
   * @param input input string
   */
  virtual void fixup(QString& input) const override;

private:
  Q_DISABLE_COPY(DateTimeValidator)

  const QRegExp m_re;
  mutable QString m_lastValidInput;
};

DateTimeValidator::DateTimeValidator(QObject* parent)
  : QValidator(parent), m_re(FrameNotice::isoDateTimeRexExp())
{
}

QValidator::State DateTimeValidator::validate(QString& input, int& pos) const
{
  QRegExp dateTimeRe = m_re;
  if (dateTimeRe.exactMatch(input)) {
    m_lastValidInput = input;
    return Acceptable;
  } else {
    const int len = dateTimeRe.matchedLength();
    if (len == input.size()) {
      return Intermediate;
    } else if (len > 0 && m_lastValidInput.endsWith(input.midRef(len))) {
      return Intermediate;
    } else {
      pos = input.size();
      return Invalid;
    }
  }
}

void DateTimeValidator::fixup(QString& input) const
{
  if (!m_lastValidInput.isEmpty()) {
    input = m_lastValidInput;
  }
}


/**
 * Helper class providing methods to paint stars for a rating.
 */
class StarPainter {
public:
  /**
   * Constructor.
   * @param starCount number of stars to paint
   * @param maxStarCount maximum number of stars
   */
  explicit StarPainter(int starCount, int maxStarCount)
    : m_starCount(starCount), m_maxStarCount(maxStarCount) {
  }

  /**
   * Get size needed for stars.
   */
  QSize sizeHint() const;

  /**
   * Paint stars to @a painter.
   */
  void paint(QPainter& painter, const QRect& rect,
             const QPalette& palette, bool editable) const;

private:
  const int m_starCount;
  const int m_maxStarCount;

  static QPolygonF s_starPolygon;
};

QPolygonF StarPainter::s_starPolygon;

QSize StarPainter::sizeHint() const {
  return STAR_SCALE_FACTOR * QSize(m_maxStarCount, 1);
}

void StarPainter::paint(QPainter& painter, const QRect& rect,
                        const QPalette& palette, bool editable) const
{
  if (s_starPolygon.isEmpty()) {
    // First time initialization.
    qreal a = -0.314;
    for (int i = 0; i < 5; ++i) {
      s_starPolygon << QPointF(0.5 + 0.5 * std::cos(a), 0.5 + 0.5 * std::sin(a));
      a += 2.513;
    }
  }

  painter.save();

  QBrush brush(editable ? palette.highlight() : palette.foreground());
  QPen starPen(Qt::NoPen);
  QPen dotPen(brush, 0.2);
  dotPen.setCapStyle(Qt::RoundCap);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setBrush(brush);

  int yOffset = (rect.height() - STAR_SCALE_FACTOR) / 2;
  painter.translate(rect.x(), rect.y() + yOffset);
  painter.scale(STAR_SCALE_FACTOR, STAR_SCALE_FACTOR);

  for (int i = 0; i < m_maxStarCount; ++i) {
    if (i < m_starCount) {
      painter.setPen(starPen);
      painter.drawPolygon(s_starPolygon, Qt::WindingFill);
    } else if (editable) {
      painter.setPen(dotPen);
      painter.drawPoint(QPointF(0.5, 0.5));
    }
    painter.translate(1.0, 0.0);
  }

  painter.restore();
}

}

/**
 * Constructor.
 * @param parent parent widget
 */
StarEditor::StarEditor(QWidget* parent)
  : QWidget(parent), m_starCount(0), m_paintedStarCount(0),
    m_starCountEdited(false)
{
  setMouseTracking(true);
  setAutoFillBackground(true);
}

/**
 * Get size needed by editor.
 * @return size needed by editor.
 */
QSize StarEditor::sizeHint() const
{
  return StarPainter(0, MAX_STAR_COUNT).sizeHint();
}

/**
 * Set star rating.
 * @param starCount number of stars
 */
void StarEditor::setStarCount(int starCount)
{
  m_starCount = starCount;
  m_paintedStarCount = starCount;
  m_starCountEdited = false;
}

void StarEditor::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  StarPainter(m_paintedStarCount, MAX_STAR_COUNT)
      .paint(painter, rect(), palette(), true);
}

void StarEditor::mouseMoveEvent(QMouseEvent* event)
{
  int starNr = starAtPosition(event->x());

  if (starNr != m_paintedStarCount && starNr != -1) {
    m_paintedStarCount = starNr;
    update();
  }
}

void StarEditor::mouseReleaseEvent(QMouseEvent*)
{
  modifyStarCount(m_paintedStarCount);
  emit editingFinished();
}

void StarEditor::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    modifyStarCount(m_paintedStarCount);
    emit editingFinished();
  } else if (event->key() == Qt::Key_Escape) {
    emit editingFinished();
  } else if (event->key() == Qt::Key_Left) {
    if (m_paintedStarCount > 0) {
      --m_paintedStarCount;
      update();
    }
  } else if (event->key() == Qt::Key_Right) {
    if (m_paintedStarCount < MAX_STAR_COUNT) {
      ++m_paintedStarCount;
      update();
    }
  } else {
    QWidget::keyPressEvent(event);
  }
}

int StarEditor::starAtPosition(int x)
{
  int star = (x / (StarPainter(0, MAX_STAR_COUNT).sizeHint().width()
                   / MAX_STAR_COUNT)) + 1;
  if (star <= 0 || star > MAX_STAR_COUNT)
    return -1;

  return star;
}

void StarEditor::modifyStarCount(int starCount)
{
  if (m_starCount != starCount) {
    m_starCount = starCount;
    m_starCountEdited = true;
  }
}


/**
 * Constructor.
 * @param genreModel genre model
 * @param parent parent QTableView
 */
FrameItemDelegate::FrameItemDelegate(GenreModel* genreModel, QObject* parent) :
  QItemDelegate(parent),
  m_genreModel(genreModel),
  m_trackNumberValidator(new TrackNumberValidator(this)),
  m_dateTimeValidator(new DateTimeValidator(this)) {
  setObjectName(QLatin1String("FrameItemDelegate"));
}

/**
 * Render delegate.
 * @param painter painter to be used
 * @param option style
 * @param index index of item
 */
void FrameItemDelegate::paint(
    QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
  if (index.row() >= 0 && index.column() == FrameTableModel::CI_Value &&
      index.data(FrameTableModel::FrameTypeRole).toInt() == Frame::FT_Rating) {
    int starCount = starCountFromRating(index.data().toInt(), index);
    if (option.state & QStyle::State_Selected) {
      painter->fillRect(option.rect, option.palette.highlight());
    }
    StarPainter(starCount, MAX_STAR_COUNT).paint(*painter, option.rect,
                                                 option.palette, false);
    return;
  }
  QItemDelegate::paint(painter, option, index);
}

/**
 * Get size needed by delegate.
 * @param option style
 * @param index  index of item
 * @return size needed by delegate.
 */
QSize FrameItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
  if (index.row() >= 0 && index.column() == FrameTableModel::CI_Value &&
      index.data(FrameTableModel::FrameTypeRole).toInt() == Frame::FT_Rating) {
    int starCount = starCountFromRating(index.data().toInt(), index);
    return StarPainter(starCount, MAX_STAR_COUNT).sizeHint();
  }
  return QItemDelegate::sizeHint(option, index);
}

/**
 * Format text if enabled.
 * @param txt text to format and set in line edit
 */
void FrameItemDelegate::formatTextIfEnabled(const QString& txt)
{
  QLineEdit* le;
  if (TagFormatConfig::instance().formatWhileEditing() &&
      (le = qobject_cast<QLineEdit*>(sender())) != nullptr) {
    QString str(txt);
    TagFormatConfig::instance().formatString(str);
    if (str != txt) {
      int curPos = le->cursorPosition();
      le->setText(str);
      le->setCursorPosition(curPos + str.length() - txt.length());
    }
  }
}

/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* FrameItemDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  int row = index.row();
  int col = index.column();
  const auto ftModel =
    qobject_cast<const FrameTableModel*>(index.model());
  if (row >= 0 && (col == FrameTableModel::CI_Value || !ftModel)) {
    Frame::Type type = static_cast<Frame::Type>(
      index.data(FrameTableModel::FrameTypeRole).toInt());
    bool id3v1 = ftModel && ftModel->isId3v1();
    if (type == Frame::FT_Genre) {
      auto cb = new QComboBox(parent);
      if (!id3v1) {
        cb->setEditable(true);
        cb->setAutoCompletion(true);
        cb->setDuplicatesEnabled(false);
      }

      cb->setModel(m_genreModel);
      return cb;
    } else if (type == Frame::FT_Rating) {
      auto editor = new StarEditor(parent);
      connect(editor, &StarEditor::editingFinished,
              this, &FrameItemDelegate::commitAndCloseEditor);
      return editor;
    }
    QWidget* editor = QItemDelegate::createEditor(parent, option, index);
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (id3v1 &&
        (type == Frame::FT_Comment || type == Frame::FT_Title ||
         type == Frame::FT_Artist || type == Frame::FT_Album)) {
      if (lineEdit) {
        if (TagFormatConfig::instance().formatWhileEditing()) {
          connect(lineEdit, &QLineEdit::textEdited,
                  this, &FrameItemDelegate::formatTextIfEnabled);
        }
        lineEdit->setMaxLength(type == Frame::FT_Comment ? 28 : 30);
      }
    } else {
      if (lineEdit) {
        if (TagFormatConfig::instance().formatWhileEditing()) {
          connect(lineEdit, &QLineEdit::textEdited,
                  this, &FrameItemDelegate::formatTextIfEnabled);
        }
        if (TagFormatConfig::instance().enableValidation()) {
          if (type == Frame::FT_Track || type == Frame::FT_Disc) {
            lineEdit->setValidator(m_trackNumberValidator);
          } else if (type == Frame::FT_Date || type == Frame::FT_OriginalDate) {
            lineEdit->setValidator(m_dateTimeValidator);
          }
        }
      }
    }
    return editor;
  }
  return QItemDelegate::createEditor(parent, option, index);
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void FrameItemDelegate::setEditorData(
  QWidget* editor, const QModelIndex& index) const
{
  if (index.row() >= 0 && index.column() == FrameTableModel::CI_Value &&
      index.data(FrameTableModel::FrameTypeRole).toInt() == Frame::FT_Rating) {
    if (auto starEditor = qobject_cast<StarEditor*>(editor)) {
      int starCount = starCountFromRating(index.data().toInt(), index);
      starEditor->setStarCount(starCount);
      return;
    }
  }
  auto cb = qobject_cast<QComboBox*>(editor);
  if (cb) {
    QString genreStr(index.model()->data(index).toString());
    cb->setCurrentIndex(m_genreModel->getRowForGenre(genreStr));
  } else {
    QItemDelegate::setEditorData(editor, index);
  }
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void FrameItemDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
  if (index.row() >= 0 && index.column() == FrameTableModel::CI_Value &&
      index.data(FrameTableModel::FrameTypeRole).toInt() == Frame::FT_Rating) {
    if (auto starEditor = qobject_cast<StarEditor*>(editor)) {
      if (starEditor->isStarCountEdited()) {
        model->setData(index, starCountToRating(starEditor->starCount(), index));
      }
      return;
    }
  }
  auto cb = qobject_cast<QComboBox *>(editor);
  if (cb) {
    model->setData(index, cb->currentText());
  } else {
    QItemDelegate::setModelData(editor, model, index);
  }
}

/**
 * Commit data and close the editor.
 */
void FrameItemDelegate::commitAndCloseEditor()
{
  if (auto editor = qobject_cast<StarEditor*>(sender())) {
    emit commitData(editor);
    emit closeEditor(editor);
  }
}
