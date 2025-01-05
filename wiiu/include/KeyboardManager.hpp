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

#ifndef KEYBOARDMANAGER_HPP
#define KEYBOARDMANAGER_HPP

#include "SDLHelper.hpp"
#include "input.hpp"
#include <codecvt>
#include <locale>
#include <nn/swkbd.h>
#include <string>
#include <utility>

class KeyboardManager {
public:
    static KeyboardManager& get(void)
    {
        static KeyboardManager mSingleton;
        return mSingleton;
    }

    KeyboardManager(KeyboardManager const&) = delete;
    void operator=(KeyboardManager const&)  = delete;

    bool init();
    void shutdown();
    std::pair<bool, std::string> keyboard(const std::string& suggestion);
    bool isInitialized() { return initialized; }

    static const size_t CUSTOM_PATH_LEN = 49;

private:
    KeyboardManager(void);
    virtual ~KeyboardManager(void){};

    void update();
    void hide();

    bool hidden() { return nn::swkbd::GetStateInputForm() == nn::swkbd::State::Hidden; }

    nn::swkbd::CreateArg createArg;
    nn::swkbd::AppearArg appearArg;
    bool initialized = false;
};

#endif