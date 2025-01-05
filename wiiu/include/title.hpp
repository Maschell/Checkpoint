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

#ifndef TITLE_HPP
#define TITLE_HPP

#include "SDLHelper.hpp"
#include "account.hpp"
#include "configuration.hpp"
#include "io.hpp"
#include <algorithm>
#include <malloc.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <coreinit/mcp.h>
#include <nn/act.h>
#include <nn/pdm.h>

struct IconWrapper {
    SDL_Texture* icon = nullptr;

    explicit IconWrapper(SDL_Texture* icon) : icon(icon) {}

    ~IconWrapper()
    {
        if (icon) {
            SDL_DestroyTexture(icon);
            icon = nullptr;
        }
    }

    // Delete the copy constructor and copy assignment operator
    IconWrapper(const IconWrapper&) = delete;

    IconWrapper& operator=(const IconWrapper&) = delete;

    IconWrapper(IconWrapper&& other) noexcept : icon(other.icon) { other.icon = {}; }

    IconWrapper& operator=(IconWrapper&& other) noexcept
    {
        if (this != &other) {
            icon       = other.icon;
            other.icon = {};
        }
        return *this;
    }
};

class Title {
public:
    Title(uint64_t titleid,
        nn::act::PersistentId persistentId,
        const std::string& name,
        const std::string& author,
        const std::string& sourcePath,
        std::shared_ptr<IconWrapper> icon,
        uint32_t playTimeMinutes,
        uint32_t lastPlayedTimestamp
        );

    [[nodiscard]] std::string publisher() const;

    std::pair<std::string, std::string> displayName();

    [[nodiscard]] const IconWrapper& icon() const;

    [[nodiscard]] uint64_t id() const;

    [[nodiscard]] std::string name() const;

    [[nodiscard]] std::string path() const;

    [[nodiscard]] std::string sourcePath() const;

    [[nodiscard]] std::string location() const;

    void sourcePath(std::string spath);

    std::string fullPath(size_t index);

    void refreshDirectories();

    std::vector<std::string> saves();

    [[nodiscard]] nn::act::PersistentId userId() const;

    [[nodiscard]] std::string userName() const;

    uint32_t playTimeMinutes();
    std::string playTime();
    void playTimeMinutes(uint32_t playTimeMinutes);
    uint32_t lastPlayedTimestamp();
    void lastPlayedTimestamp(uint32_t lastPlayedTimestamp);

private:
    uint64_t mId;
    nn::act::PersistentId mUserId;
    std::string mUserName;
    std::string mName;
    std::string mSafeName;
    std::string mPublisher;
    std::string mPath;
    std::string mSourcePath;
    std::vector<std::string> mSaves;
    std::vector<std::string> mFullSavePaths;
    std::pair<std::string, std::string> mDisplayName;
    std::shared_ptr<IconWrapper> mIcon;
    uint32_t mPlayTimeMinutes;
    uint32_t mLastPlayedTimestamp;
};

Title* getTitle(nn::act::PersistentId uid, size_t i);

size_t getTitleCount(nn::act::PersistentId uid);

void loadTitles();

void sortTitles();

void rotateSortMode();

void refreshDirectories(uint64_t id);

bool favorite(nn::act::PersistentId uid, int i);

IconWrapper& smallIcon(nn::act::PersistentId uid, size_t i);

std::unordered_map<std::string, std::string> getCompleteTitleList();

#endif