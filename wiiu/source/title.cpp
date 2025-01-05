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

#include "title.hpp"
#include <coreinit/debug.h>
#include <nn/idb.h>
#include <nn/idb/idb_cpp.h>
#include <nn/result.h>
#include <set>
#include <span>
#include <utility>

static std::unordered_map<nn::act::PersistentId, std::vector<Title>> sTitles;

static std::shared_ptr<IconWrapper> loadIcon(uint64_t id, std::span<uint8_t> iconData)
{
    SDL_Texture* texture;
    SDLH_LoadImage(&texture, iconData.data(), iconData.size(), true);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    return std::make_shared<IconWrapper>(texture);
}

Title::Title(uint64_t id, nn::act::PersistentId persistentId, const std::string& name, const std::string& publisher, const std::string& sourcePath,
    std::shared_ptr<IconWrapper> icon, uint32_t playTimeMinutes, uint32_t lastPlayedTimestamp)
    : mIcon(icon ? std::move(icon) : std::make_shared<IconWrapper>(nullptr))
{
    mId              = id;
    mUserId          = persistentId;
    mUserName        = Account::username(persistentId);
    mPublisher       = publisher;
    mName            = name;
    mSafeName        = StringUtils::containsInvalidChar(name) ? StringUtils::format("0x%016llX", mId) : StringUtils::removeForbiddenCharacters(name);
    mPath            = "wiiu/Checkpoint/saves/" + StringUtils::format("%016llX", mId) + " " + mSafeName;
    mSourcePath      = sourcePath;
    mPlayTimeMinutes = playTimeMinutes;
    mLastPlayedTimestamp = lastPlayedTimestamp;

    std::string aname = StringUtils::removeAccents(mName);
    size_t pos        = aname.rfind(':');
    mDisplayName      = std::make_pair(name, "");
    if (pos != std::string::npos) {
        std::string name1 = aname.substr(0, pos);
        std::string name2 = aname.substr(pos + 1);
        StringUtils::trim(name1);
        StringUtils::trim(name2);
        mDisplayName.first  = name1;
        mDisplayName.second = name2;
    }
    else {
        // check for parenthesis
        size_t pos1 = aname.rfind('(');
        size_t pos2 = aname.rfind(')');
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            std::string name1 = aname.substr(0, pos1);
            std::string name2 = aname.substr(pos1 + 1, pos2 - 1 - pos1);
            StringUtils::trim(name1);
            StringUtils::trim(name2);
            mDisplayName.first  = name1;
            mDisplayName.second = name2;
        }
    }

    if (!io::directoryExists(mPath)) {
        io::createDirectory(mPath);
    }

    refreshDirectories();
}

uint64_t Title::id() const
{
    return mId;
}

nn::act::PersistentId Title::userId() const
{
    return mUserId;
}

std::string Title::userName() const
{
    return mUserName;
}

std::string Title::publisher() const
{
    return mPublisher;
}

std::string Title::name() const
{
    return mName;
}

std::pair<std::string, std::string> Title::displayName()
{
    return mDisplayName;
}

std::string Title::path() const
{
    return mPath;
}

std::string Title::sourcePath() const
{
    return mSourcePath;
}

void Title::sourcePath(std::string spath)
{
    mSourcePath = std::move(spath);
}

std::string Title::fullPath(size_t index)
{
    return mFullSavePaths.at(index);
}

std::vector<std::string> Title::saves()
{
    return mSaves;
}

std::string Title::location() const
{
    if (mSourcePath.starts_with("fs:/vol/storage_mlc")) {
        return "MLC";
    }
    else if (mSourcePath.starts_with("fs:/vol/storage_usb")) {
        return "USB";
    }
    return "UNKNOWN";
}

uint32_t Title::playTimeMinutes(void)
{
    return mPlayTimeMinutes;
}

std::string Title::playTime(void)
{
    return StringUtils::format("%d", mPlayTimeMinutes / 60) + ":" + StringUtils::format("%02d", mPlayTimeMinutes % 60) + " hours";
}

void Title::playTimeMinutes(uint32_t playTimeMinutes)
{
    mPlayTimeMinutes = playTimeMinutes;
}

uint32_t Title::lastPlayedTimestamp()
{
    return mLastPlayedTimestamp;
}

void Title::lastPlayedTimestamp(uint32_t lastPlayedTimestamp)
{
    mLastPlayedTimestamp = lastPlayedTimestamp;
}

void Title::refreshDirectories()
{
    mSaves.clear();
    mFullSavePaths.clear();

    Directory savelist(mPath);
    if (savelist.good()) {
        for (size_t i = 0, sz = savelist.size(); i < sz; i++) {
            if (savelist.folder(i)) {
                mSaves.push_back(savelist.entry(i));
                mFullSavePaths.push_back(mPath + "/" + savelist.entry(i));
            }
        }

        std::sort(mSaves.rbegin(), mSaves.rend());
        std::sort(mFullSavePaths.rbegin(), mFullSavePaths.rend());
        mSaves.insert(mSaves.begin(), "New...");
        mFullSavePaths.insert(mFullSavePaths.begin(), "New...");
    }
    else {
        Logger::getInstance().log(Logger::ERROR, "Couldn't retrieve the extdata directory list for the title " + name());
    }

    // save backups from configuration
    std::vector<std::string> additionalFolders = Configuration::getInstance().additionalSaveFolders(mId);
    for (const auto& additionalFolder : additionalFolders) {
        // we have other folders to parse
        Directory list(additionalFolder);
        if (list.good()) {
            for (size_t i = 0, sz = list.size(); i < sz; i++) {
                if (list.folder(i)) {
                    mSaves.push_back(list.entry(i));
                    mFullSavePaths.push_back(additionalFolder + "/" + list.entry(i));
                }
            }
        }
    }
}

const IconWrapper& Title::icon() const
{
    return *mIcon;
}

void loadTitles()
{
    sTitles.clear();


    alignas(0x40) MCPTitleListType titles[0x50];

    int handle = MCP_Open();
    if (handle < 0) {
        OSReport("MCP_Open failed");
    }
    MCPDeviceType mcpTypes[]  = {(MCPDeviceType)1, MCP_DEVICE_TYPE_ODD, MCP_DEVICE_TYPE_MLC, MCP_DEVICE_TYPE_USB ,(MCPDeviceType)5,(MCPDeviceType)6,(MCPDeviceType)7,(MCPDeviceType)8};
    for (const auto& device : mcpTypes) {
        uint32_t curOutCount = 0;
        if (MCP_TitleListByDeviceType(handle, (MCPDeviceType)device, &curOutCount, titles, sizeof(titles)) == 0) {
            for (int i = 0; i < curOutCount; i++) {
                OSReport("%d returned %016llX %s\n", device, titles[i].titleId, titles[i].indexedDevice);
            }
            OSReport("===\n");
        } else {
            OSReport("%d\n", device);
        }
    }

    MCP_Close(handle);



    nn::idb::IDBReader read;
    if (!read.Initialize()) {
        OSReport("FAILED TO INIT DATABASE\n");
    }

    constexpr int MAX_SAVE_COUNT = 1000;
    auto* titleIdsBuffer         = (uint64_t*)memalign(0x40, sizeof(uint64_t) * MAX_SAVE_COUNT);
    ACPDeviceType deviceTypes[]  = {ACP_DEVICE_TYPE_AUTO, ACP_DEVICE_TYPE_ODD, ACP_DEVICE_TYPE_MLC, ACP_DEVICE_TYPE_USB};
    std::set<uint64_t> titleIdList;
    for (const auto& saveDevice : deviceTypes) {
        uint32_t curOutCount = 0;
        if (ACPGetSaveDataTitleIdList(saveDevice, titleIdsBuffer, MAX_SAVE_COUNT, &curOutCount) == ACP_RESULT_SUCCESS) {
            for (int i = 0; i < curOutCount; i++) {
                OSReport("%d returned %016llX\n", saveDevice, titleIdsBuffer[i]);
                if ((titleIdsBuffer[i] & 0xFFFFFFFF00000000) != 0x0005000000000000) {
                    continue;
                }
                titleIdList.insert(titleIdsBuffer[i]);
            }
            OSReport("%d returned %d titles\n", saveDevice, curOutCount);
        }
    }
    OSReport("Found %d saves in total: %d\n", titleIdList.size());
    auto* xml = (ACPMetaXml*)memalign(0x40, sizeof(ACPMetaXml));

    constexpr int SAVE_DIR_BUFFER_COUNT = 0x40;
    auto* saveDirBuffer                 = (ACPSaveDirInfo*)memalign(0x40, sizeof(ACPSaveDirInfo) * SAVE_DIR_BUFFER_COUNT);

    auto iconDataFromCache = std::make_unique<nn::idb::IconDataRaw>();
    for (const auto cur : titleIdList) {
        std::shared_ptr<IconWrapper> iconWrapper = nullptr;
        *iconDataFromCache                       = {};

        if (read.LoadIconDataFromIdb(cur, *iconDataFromCache).IsSuccess()) {
            iconWrapper = loadIcon(cur, std::span<uint8_t>(iconDataFromCache->icon, iconDataFromCache->icon + sizeof(iconDataFromCache->icon)));
        }

        std::string titleName = StringUtils::format("0x%016llX", cur);
        std::string publisher;
        if (ACPGetTitleSaveMetaXml(cur, xml, ACP_DEVICE_TYPE_AUTO) ==
            ACP_RESULT_SUCCESS) { // ACP_DEVICE_TYPE_AUTO seems to work for both MLC and USB titles
            titleName = xml->shortname_en;
            publisher = xml->publisher_en;
        }

        for (const auto& saveDevice : deviceTypes) {
            uint32_t outCount = 0;
            if (ACPGetTitleSaveDirEx(cur, saveDevice, 0, saveDirBuffer, SAVE_DIR_BUFFER_COUNT, &outCount) == ACP_RESULT_SUCCESS) {
                for (uint32_t i = 0; i < outCount; i++) {
                    auto& titleList = sTitles[saveDirBuffer[i].persistentId];
                    auto sourcePath = std::string(saveDirBuffer[i].path);

                    // TODO: put in read icon from cache helper function
                    if (!iconWrapper) {
                        std::string iconPath = sourcePath + "iconTex.tga";
                        FILE* iconFile       = fopen(iconPath.c_str(), "rb");
                        if (iconFile) {
                            fseek(iconFile, 0, SEEK_END);
                            size_t icon_size = ftell(iconFile);
                            rewind(iconFile);

                            auto* icon = (uint8_t*)malloc(icon_size);
                            if (icon) {
                                fread(icon, 1, icon_size, iconFile);
                                fclose(iconFile);

                                iconWrapper = loadIcon(cur, std::span<uint8_t>(icon, icon + icon_size));
                                free(icon);
                            }
                        }
                    }

                    if (sourcePath.ends_with("meta/")) {
                        sourcePath.erase(sourcePath.size() - 5, 5);
                    }

                    nn::act::PersistentId pId = saveDirBuffer[i].persistentId;
                    sourcePath =
                        StringUtils::format("fs:%suser/%s", sourcePath.c_str(), pId == 0 ? "common" : StringUtils::format("%08X", pId).c_str());

                    uint32_t playTimeMinutes     = 0;
                    uint16_t lastPlayedTimestamp = 0;
                    // load play statistics
                    if (pId != 0) {
                        nn::pdm::PlayStats stats{};
                        uint32_t res = nn::pdm::GetPlayStatsOfTitleId(&stats, accountIdToSlotNo(pId), cur);
                        if (res == 0) {
                            playTimeMinutes     = stats.playtime;
                            lastPlayedTimestamp = stats.last_time_played;
                        }
                    }

                    titleList.emplace_back(cur, pId, titleName, publisher, sourcePath, iconWrapper, playTimeMinutes, lastPlayedTimestamp);
                }
            }
        }
    }

    sortTitles();
    free(titleIdsBuffer);
    free(xml);
    read.Finalize();
}

void sortTitles()
{
    for (auto& vect : sTitles) {
        std::sort(vect.second.begin(), vect.second.end(), [](Title& l, Title& r) {
            if (Configuration::getInstance().favorite(l.id()) != Configuration::getInstance().favorite(r.id())) {
                return Configuration::getInstance().favorite(l.id());
            }
            switch (g_sortMode) {
                case SORT_LAST_PLAYED:
                    return l.lastPlayedTimestamp() > r.lastPlayedTimestamp();
                case SORT_PLAY_TIME:
                    return l.playTimeMinutes() > r.playTimeMinutes();
                case SORT_ALPHA:
                default:
                    return l.name() < r.name();
            }
        });
    }
}

void rotateSortMode()
{
    g_sortMode = static_cast<sort_t>((g_sortMode + 1) % SORT_MODES_COUNT);
    sortTitles();
}

Title* getTitle(nn::act::PersistentId uid, size_t i)
{
    auto it = sTitles.find(uid);
    if (it != sTitles.end() && i < getTitleCount(uid)) {
        return &it->second.at(i);
    }
    return nullptr;
}

size_t getTitleCount(nn::act::PersistentId uid)
{
    auto it = sTitles.find(uid);
    return it != sTitles.end() ? it->second.size() : 0;
}

bool favorite(nn::act::PersistentId uid, int i)
{
    auto it = sTitles.find(uid);
    return it != sTitles.end() && Configuration::getInstance().favorite(it->second.at(i).id());
}

void refreshDirectories(uint64_t id)
{
    for (auto& pair : sTitles) {
        for (auto& i : pair.second) {
            if (i.id() == id) {
                i.refreshDirectories();
                break;
            }
        }
    }
}

std::unordered_map<std::string, std::string> getCompleteTitleList()
{
    std::unordered_map<std::string, std::string> map;
    for (const auto& pair : sTitles) {
        for (const auto& value : pair.second) {
            map.insert({StringUtils::format("0x%016llX", value.id()), value.name()});
        }
    }
    return map;
}
