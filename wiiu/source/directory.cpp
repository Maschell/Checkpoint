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

#include "directory.hpp"
#include <coreinit/debug.h>

Directory::Directory(const std::string& root)
{
    mGood  = false;
    mError = 0;
    mList.clear();

    DIR* dir = opendir(root.c_str());
    struct dirent* ent;

    if (dir == nullptr) {
        OSReport("Open dir failed for %s\n", root.c_str());
        mError = (int32_t)errno;
    }
    else {
        while ((ent = readdir(dir))) {
            std::string name         = std::string(ent->d_name);
            bool directory           = ent->d_type == DT_DIR;
            struct DirectoryEntry de = {name, directory};
            mList.push_back(de);
        }
    }

    closedir(dir);
    mGood = true;
}

int32_t Directory::error() const
{
    return mError;
}

bool Directory::good() const
{
    return mGood && mError == 0;
}

std::string Directory::entry(size_t index)
{
    return index < mList.size() ? mList.at(index).name : "";
}

bool Directory::folder(size_t index)
{
    return index < mList.size() && mList.at(index).directory;
}

size_t Directory::size()
{
    return mList.size();
}