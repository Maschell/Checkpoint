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

#include "KeyboardManager.hpp"

KeyboardManager::KeyboardManager(void) {}

bool KeyboardManager::init()
{
    createArg.workMemory = malloc(nn::swkbd::GetWorkMemorySize(0));
    createArg.fsClient   = (FSClient*)malloc(sizeof(FSClient));

    if (!createArg.workMemory || !createArg.fsClient) {
        free(createArg.workMemory);
        free(createArg.fsClient);
        return false;
    }

    FSAddClient(createArg.fsClient, FS_ERROR_FLAG_NONE);

    if (nn::swkbd::Create(createArg)) {
        initialized = true;
        return true;
    }

    return false;
}

void KeyboardManager::shutdown()
{
    nn::swkbd::Destroy();
    free(createArg.workMemory);
    createArg.workMemory = nullptr;

    if (createArg.fsClient) {
        FSDelClient(createArg.fsClient, FS_ERROR_FLAG_NONE);
        free(createArg.fsClient);
        createArg.fsClient = nullptr;
    }

    initialized = false;
}

std::pair<bool, std::string> KeyboardManager::keyboard(const std::string& suggestion)
{
    if (nn::swkbd::AppearInputForm(appearArg)) {
        // convert the suggestion to a char16_t
        char16_t wsuggestion[suggestion.size() + 1] = {0};
        for (unsigned int i = 0; i < suggestion.size() + 1; i++) {
            wsuggestion[i] = suggestion.c_str()[i];
        }

        nn::swkbd::SetInputFormString(wsuggestion);

        for (;;) {
            update();

            if (nn::swkbd::IsDecideOkButton(nullptr)) {
                std::u16string output(nn::swkbd::GetInputFormString());
                hide();
                return std::make_pair(true, std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(output));
            }

            if (nn::swkbd::IsDecideCancelButton(nullptr)) {
                hide();
                break;
            }
        }
    }

    return std::make_pair(false, suggestion);
}

void KeyboardManager::update()
{
    // Update controller
    Input::update();

    nn::swkbd::ControllerInfo controllerInfo;
    VPADStatus vpad = Input::getVpad();
    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &vpad.tpFiltered1, &vpad.tpNormal);
    vpad.tpFiltered2 = vpad.tpNormal = vpad.tpFiltered1;

    controllerInfo.vpad = &vpad;
    for (int i = 0; i < 4; i++) {
        KPADStatus kpad        = Input::getKpad(i);
        controllerInfo.kpad[i] = &kpad;
    }
    nn::swkbd::Calc(controllerInfo);

    if (nn::swkbd::IsNeedCalcSubThreadFont()) {
        nn::swkbd::CalcSubThreadFont();
    }

    // draw keyboard
    SDLH_ClearScreen(theme().c1);
    nn::swkbd::DrawDRC();
    nn::swkbd::DrawTV();
    SDLH_Render();
}

void KeyboardManager::hide()
{
    if (hidden()) {
        return;
    }

    nn::swkbd::DisappearInputForm();

    while (!hidden()) {
        // update until fully hidden
        update();
    }
}