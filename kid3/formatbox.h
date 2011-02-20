/**
 * \file formatbox.h
 * Group box containing format options.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
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

#ifndef FORMATBOX_H
#define FORMATBOX_H

#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QGroupBox>
#else
#include <qgroupbox.h>
#endif

class QComboBox;
class QCheckBox;
class QString;
class FormatConfig;
class ConfigTable;

/**
 * Group box containing format options.
 */
class FormatBox : public QGroupBox
{
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param title  title
	 * @param parent parent widget
	 */
	FormatBox(const QString& title, QWidget* parent = 0);

	/**
	 * Destructor.
	 */
	~FormatBox();

	/**
	 * Set the values from a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void fromFormatConfig(const FormatConfig* cfg);

	/**
	 * Store the values in a format configuration.
	 *
	 * @param cfg format configuration
	 */
	void toFormatConfig(FormatConfig* cfg) const;

private:
	QComboBox* m_caseConvComboBox;
	QCheckBox* m_strRepCheckBox;
	ConfigTable* m_strReplTable;
	QCheckBox* m_formatEditingCheckBox;
};

#endif
