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

#ifndef CONFIGHANDLER_HPP
#define CONFIGHANDLER_HPP

#include "io.hpp"
#include "json.hpp"
#include "util.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CONFIG_VERSION 4

class Configuration {
public:
    static Configuration& getInstance(void)
    {
        static Configuration mConfiguration;
        return mConfiguration;
    }

    bool filter(uint64_t id);
    bool favorite(uint64_t id);
    std::vector<std::string> additionalSaveFolders(uint64_t id);
    void save(void);
    void load(void);
    void parse(void);
    const char* c_str(void);
    nlohmann::json getJson(void);

    const std::string BASEPATH = "wiiu/Checkpoint/config.json";

private:
    Configuration(void);
    ~Configuration(void);

    void store(void);

    Configuration(Configuration const&)  = delete;
    void operator=(Configuration const&) = delete;

    nlohmann::json mJson;
    std::unordered_set<uint64_t> mFilterIds, mFavoriteIds;
    std::unordered_map<uint64_t, std::vector<std::string>> mAdditionalSaveFolders;
};

#endif