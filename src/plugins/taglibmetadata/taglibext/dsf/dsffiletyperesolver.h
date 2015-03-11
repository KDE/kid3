/***************************************************************************
    copyright            : (C) 2006 by Martin Aumueller
    email                : aumuell@reserv.at
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,            *
 *   MA  02110-1301  USA                                                   *
 ***************************************************************************/

#ifndef TAGLIB_DSFFILETYPERESOLVER_H
#define TAGLIB_DSFFILETYPERESOLVER_H

#include <tfile.h>
#include <fileref.h>

#if (((TAGLIB_MAJOR_VERSION) << 16) + ((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION)) > 0x010400  && defined _WIN32

class DSFFileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
public:
    virtual TagLib::File *createFile(TagLib::FileName fileName,
            bool readAudioProperties,
            TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
    virtual ~DSFFileTypeResolver() {}
};

#else

class DSFFileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
public:
    virtual TagLib::File *createFile(const char *fileName,
            bool readAudioProperties,
            TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
    virtual ~DSFFileTypeResolver() {}
};

#endif

#endif
