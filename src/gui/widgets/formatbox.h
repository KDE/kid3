/**
 * \file formatbox.h
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2018  Urs Fleisch
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

#pragma once

#include <QGroupBox>

class QFormLayout;
class QComboBox;
class QCheckBox;
class QString;
class FormatConfig;
class ConfigTable;
class ConfigTableModel;

/**
 * Group box containing format options.
 */
class FormatBox : public QGroupBox {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param title  title
   * @param parent parent widget
   */
  explicit FormatBox(const QString& title, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~FormatBox() override = default;

  /**
   * Set the values from a format configuration.
   *
   * @param cfg format configuration
   */
  virtual void fromFormatConfig(const FormatConfig& cfg);

  /**
   * Store the values in a format configuration.
   *
   * @param cfg format configuration
   */
  virtual void toFormatConfig(FormatConfig& cfg) const;

protected:
  /**
   * Get form layout.
   * @return form layout.
   */
  QFormLayout* getFormLayout() const { return m_formLayout; }

private:
  QFormLayout* m_formLayout;
  QComboBox* m_caseConvComboBox;
  QComboBox* m_localeComboBox;
  QCheckBox* m_strRepCheckBox;
  ConfigTable* m_strReplTable;
  ConfigTableModel* m_strReplTableModel;
  QCheckBox* m_formatEditingCheckBox;
};
