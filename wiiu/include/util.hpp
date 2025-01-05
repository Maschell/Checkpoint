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

#ifndef UTIL_HPP
#define UTIL_HPP

#include "account.hpp"
#include "common.hpp"
#include "input.hpp"
#include "io.hpp"
#include <sys/stat.h>
#include <tuple>

#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <nn/acp.h>

#include <nn/act.h>

void servicesExit();
int32_t servicesInit();
void blinkLed(uint8_t times);

nn::act::SlotNo accountIdToSlotNo(uint32_t accountId);

namespace StringUtils {
    std::string removeAccents(const std::string& str);
    std::string removeNotAscii(std::string str);
    std::u16string UTF8toUTF16(const char* src);
}

#endif
