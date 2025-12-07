/**
 * \file taglibmodsupport.cpp
 * Support for Tracker modules.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 30 Nov 2025
 *
 * Copyright (C) 2025  Urs Fleisch
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

#include "taglibmodsupport.h"

#include <itfile.h>
#include <modfile.h>
#include <s3mfile.h>
#include <xmfile.h>

#include "taglibutils.h"
#include "taglibfile.h"

using namespace TagLibUtils;

namespace {

/**
 * Get tracker name of a module file.
 *
 * @return tracker name, null if not found.
 */
QString getTrackerName(TagLib::Tag *tag)
{
  QString trackerName;
  if (auto modTag = dynamic_cast<TagLib::Mod::Tag*>(tag)) {
    trackerName = toQString(modTag->trackerName()).trimmed();
  }
  return trackerName;
}

}


TagLib::File* TagLibModSupport::createFromExtension(
  TagLib::IOStream* stream, const TagLib::String& ext) const
{
  if (ext == "MOD" || ext == "MODULE" || ext == "NST" || ext == "WOW")
    return new TagLib::Mod::File(stream);
  if (ext == "S3M")
    return new TagLib::S3M::File(stream);
  if (ext == "IT")
    return new TagLib::IT::File(stream);
  if (ext == "XM")
    return new TagLib::XM::File(stream);
  return nullptr;
}

bool TagLibModSupport::readFile(TagLibFile& f, TagLib::File* file) const
{
  if (dynamic_cast<TagLib::Mod::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".mod");
    putFileRefTagInTag2(f);
    return true;
  }
  if (dynamic_cast<TagLib::S3M::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".s3m");
    putFileRefTagInTag2(f);
    return true;
  }
  if (dynamic_cast<TagLib::IT::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".it");
    putFileRefTagInTag2(f);
    return true;
  }
  if (dynamic_cast<TagLib::XM::File*>(file) != nullptr) {
    f.m_fileExtension = QLatin1String(".xm");
    putFileRefTagInTag2(f);
    return true;
  }
  return false;
}

bool TagLibModSupport::readAudioProperties(
  TagLibFile& f, TagLib::AudioProperties* audioProperties) const
{
  if (auto modProperties =
      dynamic_cast<TagLib::Mod::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("Mod %1 %2 Instruments"))
        .arg(getTrackerName(f.m_tag[Frame::Tag_2]))
        .arg(modProperties->instrumentCount());
    return true;
  }
  if (auto s3mProperties =
      dynamic_cast<TagLib::S3M::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("S3M %1 V%2 T%3"))
                            .arg(getTrackerName(f.m_tag[Frame::Tag_2]))
                            .arg(s3mProperties->fileFormatVersion())
                            .arg(s3mProperties->trackerVersion(), 0, 16);
    f.m_detailInfo.channelMode = s3mProperties->stereo()
      ? TagLibFile::DetailInfo::CM_Stereo : TagLibFile::DetailInfo::CM_None;
    return true;
  }
  if (auto itProperties =
      dynamic_cast<TagLib::IT::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("IT %1 V%2 %3 Instruments"))
                            .arg(getTrackerName(f.m_tag[Frame::Tag_2]))
                            .arg(itProperties->version(), 0, 16)
                            .arg(itProperties->instrumentCount());
    f.m_detailInfo.channelMode = itProperties->stereo()
      ? TagLibFile::DetailInfo::CM_Stereo : TagLibFile::DetailInfo::CM_None;
    return true;
  }
  if (auto xmProperties =
      dynamic_cast<TagLib::XM::Properties*>(audioProperties)) {
    f.m_detailInfo.format = QString(QLatin1String("XM %1 V%2 %3 Instruments"))
                            .arg(getTrackerName(f.m_tag[Frame::Tag_2]))
                            .arg(xmProperties->version(), 0, 16)
                            .arg(xmProperties->instrumentCount());
    return true;
  }
  return false;
}
