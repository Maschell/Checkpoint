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
#include <span>
#include <coreinit/debug.h>

static std::unordered_map<AccountUid, std::vector<Title>> sTitles;
static std::unordered_map<uint64_t, SDL_Texture *> icons;

void freeIcons(void) {
   for (auto &i: icons) {
      SDL_DestroyTexture(i.second);
   }
}

static void loadIcon(uint64_t id, uint8_t *icon, size_t iconsize) {
   auto it = icons.find(id);
   if (it == icons.end()) {
      SDL_Texture *texture;
      SDLH_LoadImage(&texture, icon, iconsize, true);
      SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
      icons.insert({id, texture});
   }
}

void Title::init(uint64_t id, AccountUid userID, const std::string &name, const std::string &author) {
   mId = id;
   mUserId = userID;
   mUserName = Account::username(userID);
   mAuthor = author;
   mName = name;
   mSafeName = StringUtils::containsInvalidChar(name) ? StringUtils::format("0x%016llX", mId)
                                                      : StringUtils::removeForbiddenCharacters(name);
   mPath = "wiiu/Checkpoint/saves/" + StringUtils::format("0x%016llX", mId) + " " + mSafeName;

   std::string aname = StringUtils::removeAccents(mName);
   size_t pos = aname.rfind(":");
   mDisplayName = std::make_pair(name, "");
   if (pos != std::string::npos) {
      std::string name1 = aname.substr(0, pos);
      std::string name2 = aname.substr(pos + 1);
      StringUtils::trim(name1);
      StringUtils::trim(name2);
      mDisplayName.first = name1;
      mDisplayName.second = name2;
   } else {
      // check for parenthesis
      size_t pos1 = aname.rfind("(");
      size_t pos2 = aname.rfind(")");
      if (pos1 != std::string::npos && pos2 != std::string::npos) {
         std::string name1 = aname.substr(0, pos1);
         std::string name2 = aname.substr(pos1 + 1, pos2 - 1 - pos1);
         StringUtils::trim(name1);
         StringUtils::trim(name2);
         mDisplayName.first = name1;
         mDisplayName.second = name2;
      }
   }

   if (!io::directoryExists(mPath)) {
      io::createDirectory(mPath);
   }

   refreshDirectories();
}

void initvWii(const std::string &name, const std::string &author) {

}

bool Title::isvWii(void) {
   return misvWii;
}

uint64_t Title::id(void) {
   return mId;
}

AccountUid Title::userId(void) {
   return mUserId;
}

std::string Title::userName(void) {
   return mUserName;
}

std::string Title::author(void) {
   return mAuthor;
}

std::string Title::name(void) {
   return mName;
}

std::pair<std::string, std::string> Title::displayName(void) {
   return mDisplayName;
}

std::string Title::path(void) {
   return mPath;
}

std::string Title::sourcePath(void) {
   return mSourcePath;
}

void Title::sourcePath(std::string spath) {
   mSourcePath = spath;
}

std::string Title::fullPath(size_t index) {
   return mFullSavePaths.at(index);
}

std::vector<std::string> Title::saves() {
   return mSaves;
}

SDL_Texture *Title::icon(void) {
   auto it = icons.find(mId);
   return it != icons.end() ? it->second : NULL;
}

uint32_t Title::playTimeMinutes(void) {
   return mPlayTimeMinutes;
}

std::string Title::playTime(void) {
   return StringUtils::format("%d", mPlayTimeMinutes / 60) + ":" + StringUtils::format("%02d", mPlayTimeMinutes % 60) +
          " hours";
}

void Title::playTimeMinutes(uint32_t playTimeMinutes) {
   mPlayTimeMinutes = playTimeMinutes;
}

uint32_t Title::lastPlayedTimestamp(void) {
   return mLastPlayedTimestamp;
}

void Title::lastPlayedTimestamp(uint32_t lastPlayedTimestamp) {
   mLastPlayedTimestamp = lastPlayedTimestamp;
}

void Title::refreshDirectories(void) {
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
   } else {
      Logger::getInstance().log(Logger::ERROR, "Couldn't retrieve the extdata directory list for the title " + name());
   }

   // save backups from configuration
   std::vector<std::string> additionalFolders = Configuration::getInstance().additionalSaveFolders(mId);
   for (std::vector<std::string>::const_iterator it = additionalFolders.begin(); it != additionalFolders.end(); ++it) {
      // we have other folders to parse
      Directory list(*it);
      if (list.good()) {
         for (size_t i = 0, sz = list.size(); i < sz; i++) {
            if (list.folder(i)) {
               mSaves.push_back(list.entry(i));
               mFullSavePaths.push_back(*it + "/" + list.entry(i));
            }
         }
      }
   }
}

extern "C" ACPResult
ACPGetSaveDataTitleIdList(int storage_type, uint64_t *titlesOut, uint32_t maxCount, uint32_t *titlesNumOut);

void loadTitles() {
   sTitles.clear();

   MCPError mcp = MCP_Open();
   if (mcp < 0) {
      return;
   }

   MCPError titleCount = MCP_TitleCount(mcp);

   if (titleCount < 0) {
      MCP_Close(mcp);
      return;
   }

   auto *titleIds = (uint64_t *) memalign(0x40, sizeof(uint64_t) * 1000);

   uint32_t outCount = 0;
   if (ACPGetSaveDataTitleIdList(3, titleIds, 1000, &outCount) != ACP_RESULT_SUCCESS) {
      MCP_Close(mcp);
      return;
   }

   for (const auto cur : std::span<uint64_t>(titleIds, titleIds + outCount)) {
      OSReport("%016llX\n", cur);
   }


   sortTitles();
}

void sortTitles(void) {
   for (auto &vect: sTitles) {
      std::sort(vect.second.begin(), vect.second.end(), [](Title &l, Title &r) {
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

void rotateSortMode(void) {
   g_sortMode = static_cast<sort_t>((g_sortMode + 1) % SORT_MODES_COUNT);
   sortTitles();
}

void getTitle(Title &dst, AccountUid uid, size_t i) {
   std::unordered_map<AccountUid, std::vector<Title>>::iterator it = sTitles.find(uid);
   if (it != sTitles.end() && i < getTitleCount(uid)) {
      dst = it->second.at(i);
   }
}

size_t getTitleCount(AccountUid uid) {
   std::unordered_map<AccountUid, std::vector<Title>>::iterator it = sTitles.find(uid);
   return it != sTitles.end() ? it->second.size() : 0;
}

bool favorite(AccountUid uid, int i) {
   std::unordered_map<AccountUid, std::vector<Title>>::iterator it = sTitles.find(uid);
   return it != sTitles.end() ? Configuration::getInstance().favorite(it->second.at(i).id()) : false;
}

void refreshDirectories(uint64_t id) {
   for (auto &pair: sTitles) {
      for (size_t i = 0; i < pair.second.size(); i++) {
         if (pair.second.at(i).id() == id) {
            pair.second.at(i).refreshDirectories();
         }
      }
   }
}

SDL_Texture *smallIcon(AccountUid uid, size_t i) {
   std::unordered_map<AccountUid, std::vector<Title>>::iterator it = sTitles.find(uid);
   return it != sTitles.end() ? it->second.at(i).icon() : NULL;
}

std::unordered_map<std::string, std::string> getCompleteTitleList(void) {
   std::unordered_map<std::string, std::string> map;
   for (const auto &pair: sTitles) {
      for (auto value: pair.second) {
         map.insert({StringUtils::format("0x%016llX", value.id()), value.name()});
      }
   }
   return map;
}
