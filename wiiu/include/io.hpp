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

#ifndef IO_HPP
#define IO_HPP

#include "KeyboardManager.hpp"
#include "account.hpp"
#include "directory.hpp"
#include "multiselection.hpp"
#include "title.hpp"
#include "util.hpp"
#include <dirent.h>
#include <nn/act.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>
#include <utility>

#define BUFFER_SIZE 0x80000

namespace io {
    struct DirEntry {
        uint8_t type;
        std::string name;
    };

    std::tuple<bool, int32_t, std::string> backup(size_t index, nn::act::PersistentId uid, size_t cellIndex);
    std::tuple<bool, int32_t, std::string> restore(size_t index, nn::act::PersistentId uid, size_t cellIndex, const std::string& nameFromCell);

    int32_t copyDirectory(const std::string& srcPath, const std::string& dstPath, int mode = 0);
    void copyFile(const std::string& srcPath, const std::string& dstPath, int mode = 0);
    int32_t createDirectory(const std::string& path, int mode = 0);
    int32_t deleteFolderRecursively(const std::string& path);
    bool directoryExists(const std::string& path);
    bool fileExists(const std::string& path);
}

#endif