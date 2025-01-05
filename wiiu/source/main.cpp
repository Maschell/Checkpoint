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

#include "main.hpp"
#include "MainScreen.hpp"
#include "input.hpp"
#include <coreinit/debug.h>
#include <coreinit/filesystem_fsa.h>
#include <mocha/mocha.h>
#include <whb/proc.h>

typedef struct FSADeviceData {
    devoptab_t device;
    bool setup;
    bool mounted;
    bool isSDCard;
    char name[32];
    char mountPath[0x80];
    char cwd[FS_MAX_PATH + 1];
    FSAClientHandle clientHandle;
    uint64_t deviceSizeInSectors;
    uint32_t deviceSectorSize;
} __wut_fsa_device_t;

bool slibMochaMount = false;

void initMochaLib()
{
    slibMochaMount       = false;
    MochaUtilsStatus res = Mocha_InitLibrary();
    if (res != MOCHA_RESULT_SUCCESS) {
        OSReport("Mocha_InitLibrary failed: %s\n", Mocha_GetStatusStr(res));
        OSFatal("Failed to init libmocha. Please update MochaPayload.");
    }
    else {
        slibMochaMount = true;
    }
}

void deInitMochaLib()
{
    if (slibMochaMount) {
        Mocha_DeInitLibrary();
        slibMochaMount = false;
    }
}

extern "C" __wut_fsa_device_t __wut_fsa_device_data;
int main(void)
{
    WHBProcInit();

    try {
        if (servicesInit() != 0) {
            servicesExit();
            WHBProcShutdown();
            return 0;
        }

        initMochaLib();
        // TODO:
        if (Mocha_UnlockFSClientEx(__wut_fsa_device_data.clientHandle) != MOCHA_RESULT_SUCCESS) {
            OSFatal("Failed to unlock FSAClientHandle");
        }

        deInitMochaLib();
        g_screen = std::make_unique<MainScreen>();

        loadTitles();

        // set g_currentUId to the current user
        g_currentUId = nn::act::GetPersistentId();

        while (WHBProcIsRunning()) {
            g_screen->doDraw();

            Input::update();
            touchPosition touch = Input::getTouch();
            g_screen->doUpdate(&touch);

            SDLH_Render();
        }
        servicesExit();
    }
    catch (std::exception& e) {
        OSReport("%s\n", e.what());
        while (WHBProcIsRunning()) {}
        servicesExit();
    }

    WHBProcShutdown();

    return 0;
}