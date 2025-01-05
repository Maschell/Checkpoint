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

#include "account.hpp"
#include <coreinit/debug.h>

static std::map<nn::act::PersistentId, User> sUsers;

bool Account::init()
{
    auto res = nn::act::Initialize().IsSuccess();

    // add common user
    sUsers.insert({0, User(0, "Common", "common", nullptr)});

    return res;
}

void Account::exit()
{
    nn::act::Finalize();
}

std::vector<nn::act::PersistentId> Account::ids()
{
    std::vector<nn::act::PersistentId> v;
    for (auto& value : sUsers) {
        v.push_back(value.second.id);
    }
    return v;
}

std::optional<User> Account::getUserFromSlot(nn::act::SlotNo slot)
{
    if (!nn::act::IsSlotOccupied(slot)) {
        return {};
    }
    auto id = nn::act::GetPersistentIdEx(slot);

    // Get the mii name because not every account has a NNID linked
    int16_t miiName[nn::act::MiiNameSize];
    nn::act::GetMiiNameEx(miiName, slot);

    // convert to string
    std::u16string miiNameString((char16_t*)miiName, nn::act::MiiNameSize);
    auto name = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(miiNameString);

    auto shortName = trimToFit(name, 96 - g_username_dotsize * 2, 13);

    // load icon
    auto* buffer      = (uint8_t*)malloc(MAX_IMAGE_SIZE);
    size_t image_size = 0;
    nn::act::GetMiiImageEx(&image_size, buffer, MAX_IMAGE_SIZE, 0, slot);
    SDL_Texture* icon = nullptr;
    if (image_size > 0) {
        SDLH_LoadImage(&icon, buffer, image_size, true);
    }
    free(buffer);

    return User(id, name, shortName, icon);
}

static std::optional<User> getUser(nn::act::PersistentId id)
{
    nn::act::SlotNo slotNo = accountIdToSlotNo(id);
    OSReport("slot res %d\n", slotNo);
    if (slotNo != 0) {
        return Account::getUserFromSlot(slotNo);
    }

    return {};
}

User& Account::getUserCached(nn::act::PersistentId id)
{
    if (!sUsers.contains(id)) {
        auto user = getUser(id);
        if (!user) {
            OSReport("Tried to access invalid user %08X\n", id);
            throw std::runtime_error("Tried to access invalid user");
        }
        sUsers.insert({id, std::move(*user)});
    }
    return sUsers.at(id);
}

std::string Account::username(nn::act::PersistentId id)
{
    return getUserCached(id).name;
}

std::string Account::shortName(nn::act::PersistentId id)
{
    return getUserCached(id).shortName;
}

SDL_Texture* Account::icon(nn::act::PersistentId id)
{
    return getUserCached(id).icon;
}