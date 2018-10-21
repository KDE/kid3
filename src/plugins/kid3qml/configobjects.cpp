/**
 * \file configobjects.cpp
 * Access to configurations as QObjects.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Sep 2014
 *
 * Copyright (C) 2014-2018  Urs Fleisch
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

#include "configobjects.h"
#include "batchimportconfig.h"
#include "filterconfig.h"
#include "formatconfig.h"
#include "importconfig.h"
#include "exportconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "rendirconfig.h"
#include "numbertracksconfig.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "networkconfig.h"
#include "playlistconfig.h"
#include "serverimporterconfig.h"
#include "findreplaceconfig.h"
#include "mainwindowconfig.h"

ConfigObjects::ConfigObjects(QObject* parent) : QObject(parent)
{
}

QObject* ConfigObjects::batchImportConfig()
{
  return &BatchImportConfig::instance();
}

QObject* ConfigObjects::filterConfig()
{
  return &FilterConfig::instance();
}

QObject* ConfigObjects::filenameFormatConfig()
{
  return &FilenameFormatConfig::instance();
}

QObject* ConfigObjects::tagFormatConfig()
{
  return &TagFormatConfig::instance();
}

QObject* ConfigObjects::importConfig()
{
  return &ImportConfig::instance();
}

QObject* ConfigObjects::exportConfig()
{
  return &ExportConfig::instance();
}

QObject* ConfigObjects::tagConfig()
{
  return &TagConfig::instance();
}

QObject* ConfigObjects::fileConfig()
{
  return &FileConfig::instance();
}

QObject* ConfigObjects::renDirConfig()
{
  return &RenDirConfig::instance();
}

QObject* ConfigObjects::numberTracksConfig()
{
  return &NumberTracksConfig::instance();
}

QObject* ConfigObjects::userActionsConfig()
{
  return &UserActionsConfig::instance();
}

QObject* ConfigObjects::guiConfig()
{
  return &GuiConfig::instance();
}

QObject* ConfigObjects::networkConfig()
{
  return &NetworkConfig::instance();
}

QObject* ConfigObjects::playlistConfig()
{
  return &PlaylistConfig::instance();
}

QObject* ConfigObjects::findReplaceConfig()
{
  return &FindReplaceConfig::instance();
}

QObject* ConfigObjects::mainWindowConfig()
{
  return &MainWindowConfig::instance();
}
