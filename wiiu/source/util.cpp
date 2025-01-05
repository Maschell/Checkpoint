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

#include "util.hpp"
#include <coreinit/debug.h>
#include <nn/acp/drcled_c.h>

void servicesExit()
{
    Input::finalize();

    KeyboardManager::get().shutdown();

    nn::pdm::Finalize();

    Account::exit();

    SDLH_Exit();

    Logger::getInstance().flush();
}

int32_t servicesInit()
{
    io::createDirectory("wiiu");
    io::createDirectory("wiiu/Checkpoint");
    io::createDirectory("wiiu/Checkpoint/saves");

    Logger::getInstance().log(Logger::INFO, "Starting Checkpoint loading...");

    Configuration::getInstance();

    if (!SDLH_Init()) {
        Logger::getInstance().log(Logger::ERROR, "SDLH_Init failed");
        return -1;
    }

    if (!Account::init()) {
        Logger::getInstance().log(Logger::ERROR, "Account::init failed");
        return -1;
    }

    if (nn::pdm::Initialize() != 0) {
        Logger::getInstance().log(Logger::ERROR, "pdm::Initialize() failed");
        return -1;
    }

    if (!KeyboardManager::get().init()) {
        Logger::getInstance().log(Logger::ERROR, "Error initializing keyboard");
    }

    Logger::getInstance().log(Logger::INFO, "Checkpoint loading completed!");

    Input::initialize();

    return 0;
}

std::u16string StringUtils::UTF8toUTF16(const char* src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.from_bytes(src);
}

// https://stackoverflow.com/questions/14094621/change-all-accented-letters-to-normal-letters-in-c
std::string StringUtils::removeAccents(const std::string& str)
{
    std::u16string src = UTF8toUTF16(str.c_str());
    const std::u16string illegal = UTF8toUTF16("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüūýþÿ");
    const std::u16string fixed = UTF8toUTF16("AAAAAAECEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaeceeeeiiiiOnooooo/0uuuuuypy");

    for (size_t i = 0, sz = src.length(); i < sz; i++) {
        size_t index = illegal.find(src[i]);
        if (index != std::string::npos) {
            src[i] = fixed[index];
        }
    }

    return UTF16toUTF8(src);
}

std::string StringUtils::removeNotAscii(std::string str)
{
    for (size_t i = 0, sz = str.length(); i < sz; i++) {
        if (!isascii(str[i])) {
            str[i] = ' ';
        }
    }
    return str;
}

#define ACP_DRC_LED_PATTERN_2 2

void blinkLed(uint8_t times)
{
    ACPTurnOnDrcLed(0, ACP_DRC_LED_PATTERN_2);
}

nn::act::SlotNo accountIdToSlotNo(uint32_t accountId)
{
    OSReport("accountIdToSlotNo %d\n", accountId);
    for (uint8_t i = 1; i <= 12; i++) {
        if (nn::act::GetPersistentIdEx(i) == accountId) {
            OSReport("Found in slot %d\n", i);
            return i;
        }
    }
    OSReport("Didn't find slot for :( %d\n", accountId);
    return 0;
}