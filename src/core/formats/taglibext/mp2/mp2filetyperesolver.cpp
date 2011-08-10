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

#include "mp2filetyperesolver.h"
#include <mpegfile.h>

#include <string.h>

#if (((TAGLIB_MAJOR_VERSION) << 16) + ((TAGLIB_MINOR_VERSION) << 8) + (TAGLIB_PATCH_VERSION)) > 0x010400  && defined _WIN32

TagLib::File *MP2FileTypeResolver::createFile(TagLib::FileName fileName,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
    const wchar_t* wstr = static_cast<const wchar_t*>(fileName);
    const char* str = static_cast<const char*>(fileName);
    const wchar_t* wext;
    const char* ext;
    if ((wstr && (wext = wcsrchr(fileName, L'.')) != 0 && !wcsicmp(wext, L".mp2")) ||
        (str  && (ext  = strrchr(fileName,  '.')) != 0 && !strcasecmp(ext, ".mp2")))
    {
        return new TagLib::MPEG::File(fileName, readProperties, propertiesStyle);
    }

    return 0;
}

#else

TagLib::File *MP2FileTypeResolver::createFile(const char *fileName,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
    const char *ext = strrchr(fileName, '.');
    if(ext && !strcasecmp(ext, ".mp2"))
    {
        return new TagLib::MPEG::File(fileName, readProperties, propertiesStyle);
    }

    return 0;
}

#endif
