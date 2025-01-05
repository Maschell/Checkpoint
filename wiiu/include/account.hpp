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

#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include "SDLHelper.hpp"
#include <cstring>
#include <map>
#include <nn/acp.h>
#include <nn/act.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#define COMMONSAVE_ID 0xFFFFFFFF
#define USER_ICON_SIZE 64
#define MAX_IMAGE_SIZE 1024 * 70 // 70 Kib should be enough

struct User {
    nn::act::PersistentId id;
    std::string name;
    std::string shortName;
    SDL_Texture* icon = nullptr;

    User(nn::act::PersistentId id, std::string name, std::string shortName, SDL_Texture* icon = nullptr)
        : id(id), name(std::move(name)), shortName(std::move(shortName)), icon(icon)
    {
    }

    ~User()
    {
        if (icon) {
            SDL_DestroyTexture(icon);
            icon = nullptr;
        }
    }

    // Delete the copy constructor and copy assignment operator
    User(const User&) = delete;

    User& operator=(const User&) = delete;

    User(User&& other) noexcept : id(other.id), name(std::move(other.name)), shortName(std::move(other.shortName)), icon(other.icon)
    {
        other.id        = {};
        other.name      = {};
        other.shortName = {};
        other.icon      = {};
    }

    User& operator=(User&& other) noexcept
    {
        if (this != &other) {
            id              = other.id;
            name            = other.name;
            shortName       = other.shortName;
            icon            = other.icon;
            other.id        = {};
            other.name      = {};
            other.shortName = {};
            other.icon      = {};
        }
        return *this;
    }
};

namespace Account {
    bool init();

    void exit();

    std::vector<nn::act::PersistentId> ids();

    std::optional<User> getUserFromSlot(nn::act::SlotNo slot);

    SDL_Texture* icon(nn::act::PersistentId id);

    std::string username(nn::act::PersistentId id);

    std::string shortName(nn::act::PersistentId id);

    User& getUserCached(nn::act::PersistentId id);
}

#endif