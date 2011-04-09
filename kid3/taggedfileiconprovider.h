/**
 * \file taggedfileiconprovider.h
 * Provides icons for tagged files.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29-Mar-2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef TAGGEDFILEICONPROVIDER_H
#define TAGGEDFILEICONPROVIDER_H

#include <QIcon>
#include <QColor>

class TaggedFile;

/**
 * Provides icons for tagged files.
 */
class TaggedFileIconProvider {
public:
	/**
	 * Constructor.
	 */
	TaggedFileIconProvider();

	/**
	 * Get an icon for a tagged file.
	 *
	 * @param taggedFile tagged file
	 *
	 * @return icon for tagged file
	 */
	QIcon iconForTaggedFile(const TaggedFile* taggedFile);

	/**
	 * Get background color for a tagged file.
	 *
	 * @param taggedFile tagged file
	 *
	 * @return background color for tagged file
	 */
	QColor backgroundForTaggedFile(const TaggedFile* taggedFile);

private:
	/** Empty icon */
	QIcon m_nullIcon;
	/** Icon for modified file */
	QIcon m_modifiedIcon;
	/** Icon for V1V2 */
	QIcon m_v1v2Icon;
	/** Icon for V1 */
	QIcon m_v1Icon;
	/** Icon for V2 */
	QIcon m_v2Icon;
	/** Icon for "no tag" */
	QIcon m_notagIcon;
};

#endif // TAGGEDFILEICONPROVIDER_H
