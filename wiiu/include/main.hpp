/*
 *   This file is part of Checkpoint
 *   Copyright (C) 2017-2019 Bernardo Giordano, FlagBrew
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#ifndef MAIN_HPP
#define MAIN_HPP

#include "Screen.hpp"
#include "account.hpp"
#include "title.hpp"
#include "util.hpp"
#include <memory>

typedef enum { SORT_ALPHA, SORT_LAST_PLAYED, SORT_PLAY_TIME, SORT_MODES_COUNT } sort_t;

inline float g_currentTime = 0;
inline AccountUid g_currentUId;
inline bool g_backupScrollEnabled       = 0;
inline std::shared_ptr<Screen> g_screen = nullptr;
inline uint32_t g_username_dotsize;
inline sort_t g_sortMode = SORT_ALPHA;

inline std::string g_currentFile = "";
inline bool g_isTransferringFile = false;

#endif