/**
 * \file frameobjectmodel.h
 * Object model with frame information.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 02 Sep 2014
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

#ifndef FRAMEOBJECTMODEL_H
#define FRAMEOBJECTMODEL_H

#include <QObject>
#include "frame.h"

class FrameFieldObject;

/**
 * Object model with frame information.
 */
class FrameObjectModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString internalName READ internalName CONSTANT)
  Q_PROPERTY(int type READ type CONSTANT)
  Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(QList<QObject*> fields READ fields NOTIFY fieldsChanged)
public:
  /**
   * Constructor.
   * @param parent parent object
   */
  explicit FrameObjectModel(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~FrameObjectModel();

  /**
   * Get frame name.
   * @return translated frame name.
   */
  QString name() const;

  /**
   * Get internal frame name.
   * @return internal frame name, e.g. "TXXX - User defined text information"
   */
  QString internalName() const;

  /**
   * Get frame type.
   * @return type, type Frame::Type.
   */
  int type() const;

  /**
   * Get frame value.
   * @return frame value.
   */
  QString value() const;

  /**
   * Set frame value.
   * @param value value
   */
  void setValue(const QString& value);

  /**
   * Get field list.
   * @return fields.
   */
  QList<QObject*> fields();

  /**
   * Set from frame.
   * @param frame frame
   */
  void setFrame(const Frame& frame);

  /**
   * Get frame from object information.
   * @return frame.
   */
  Frame getFrame() const;

signals:
  /** Emitted when value is changed. */
  void valueChanged(const QString& value);

  /** Emitted when any of the fields is changed. */
  void fieldsChanged();

private:
  friend class FrameFieldObject;

  Frame m_frame;
};

/**
 * Object with frame field information.
 */
class FrameFieldObject : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(int id READ id CONSTANT)
  Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
public:
  /**
   * Constructor.
   * @param index index in field list
   * @param parent parent object
   */
  FrameFieldObject(int index, FrameObjectModel* parent);

  /**
   * Destructor.
   */
  virtual ~FrameFieldObject();

  /**
   * Get field name.
   * @return translated field name.
   */
  QString name() const;

  /**
   * Get field ID.
   * @return id, type Frame::Field::Id.
   */
  int id() const;

  /**
   * Get field value.
   * @return field value.
   */
  QVariant value() const;

  /**
   * Set field value.
   * @param value value
   */
  void setValue(const QVariant& value);

signals:
  /** Emitted when the value is changed. */
  void valueChanged(const QVariant& value);

private:
  FrameObjectModel* frameObject() const {
    return static_cast<FrameObjectModel*>(parent());
  }

  Frame::Field& field() {
    return frameObject()->m_frame.fieldList()[m_index];
  }

  const Frame::Field& constField() const {
    return frameObject()->m_frame.getFieldList().at(m_index);
  }

  const int m_index;
};

#endif // FRAMEOBJECTMODEL_H
