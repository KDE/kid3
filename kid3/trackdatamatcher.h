/**
 * \file trackdatamatcher.h
 * Shuffle imported tracks to optimize match with length, track or title.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
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

#ifndef TRACKDATAMATCHER_H
#define TRACKDATAMATCHER_H

class TrackDataModel;

/**
 * Shuffle imported tracks to optimize match with length, track or title.
 */
namespace TrackDataMatcher {

/**
 * Match import data with length.
 *
 * @param trackDataModel tracks to match
 * @param diffCheckEnable true if time difference check is enabled
 * @param maxDiff maximum allowed time difference
 */
bool matchWithLength(TrackDataModel* trackDataModel,
										 bool diffCheckEnable, int maxDiff);

/**
 * Match import data with track number.
 *
 * @param trackDataModel tracks to match
 */
bool matchWithTrack(TrackDataModel* trackDataModel);

/**
 * Match import data with title.
 *
 * @param trackDataModel tracks to match
 */
bool matchWithTitle(TrackDataModel* trackDataModel);

}

#endif // TRACKDATAMATCHER_H
