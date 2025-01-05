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

#include "MainScreen.hpp"
#include "input.hpp"

static constexpr size_t rowlen = 5, collen = 4, rows = 10, SIDEBAR_w = 96;

MainScreen::MainScreen() : hid(rowlen * collen, collen)
{
    selectionTimer = 0;
    sprintf(ver, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    backupList    = std::make_unique<Scrollable>(538, 276, 414, 380, rows);
    buttonBackup  = std::make_unique<Clickable>(956, 276, 220, 80, theme().c2, theme().c6, "Backup \ue004", true);
    buttonRestore = std::make_unique<Clickable>(956, 360, 220, 80, theme().c2, theme().c6, "Restore \ue005", true);
    buttonBackup->canChangeColorWhenSelected(true);
    buttonRestore->canChangeColorWhenSelected(true);
}

int MainScreen::selectorX(size_t i) const
{
    return 128 * ((i % (rowlen * collen)) % collen) + 4 * (((i % (rowlen * collen)) % collen) + 1);
}

int MainScreen::selectorY(size_t i) const
{
    return 128 * ((i % (rowlen * collen)) / collen) + 4 * (((i % (rowlen * collen)) / collen) + 1);
}

void MainScreen::draw() const
{
    auto selEnt          = MS::selectedEntries();
    const size_t entries = hid.maxVisibleEntries();
    const size_t max     = hid.maxEntries(getTitleCount(g_currentUId)) + 1;

    SDLH_ClearScreen(theme().c1);
    SDL_Color colorBar = FC_MakeColor(theme().c1.r - 15, theme().c1.g - 15, theme().c1.b - 15, 255);
    SDLH_DrawRect(0, 0, 532, 662, FC_MakeColor(theme().c1.r + 5, theme().c1.g + 5, theme().c1.b + 5, 255));
    SDLH_DrawRect(1280 - SIDEBAR_w, 0, SIDEBAR_w, 720, colorBar);

    drawPulsingOutline(
        1280 - SIDEBAR_w + (SIDEBAR_w - USER_ICON_SIZE) / 2, 720 - USER_ICON_SIZE - 30, USER_ICON_SIZE, USER_ICON_SIZE, 2, COLOR_GREEN);
    SDLH_DrawImageScale(
        Account::icon(g_currentUId), 1280 - SIDEBAR_w + (SIDEBAR_w - USER_ICON_SIZE) / 2, 720 - USER_ICON_SIZE - 30, USER_ICON_SIZE, USER_ICON_SIZE);

    u32 username_w, username_h;
    std::string username = Account::shortName(g_currentUId);
    SDLH_GetTextDimensions(13, username.c_str(), &username_w, &username_h);
    SDLH_DrawTextBox(13, 1280 - SIDEBAR_w + (SIDEBAR_w - username_w) / 2, 720 - 28 + (28 - username_h) / 2, theme().c6, SIDEBAR_w, username.c_str());

    // title icons
    for (size_t k = hid.page() * entries; k < hid.page() * entries + max; k++) {
        int selectorx = selectorX(k);
        int selectory = selectorY(k);
        auto* title   = getTitle(g_currentUId, k);

        if (title && title->icon().icon != NULL) {
            SDLH_DrawImageScale(title->icon().icon, selectorx, selectory, 128, 128);
        }
        else {
            SDLH_DrawRect(selectorx, selectory, 128, 128, theme().c0);
        }

        if (!selEnt.empty() && std::find(selEnt.begin(), selEnt.end(), k) != selEnt.end()) {
            SDLH_DrawIcon("checkbox", selectorx + 86, selectory + 86);
        }

        if (favorite(g_currentUId, k)) {
            SDLH_DrawRect(selectorx + 94, selectory + 8, 24, 24, COLOR_GOLD);
            SDLH_DrawIcon("star", selectorx + 86, selectory);
        }
    }

    // title selector
    if (getTitleCount(g_currentUId) > 0) {
        const int x = selectorX(hid.index()) + 4 / 2;
        const int y = selectorY(hid.index()) + 4 / 2;
        drawPulsingOutline(x, y, 124, 124, 4, COLOR_BLUE);
        SDLH_DrawRect(x, y, 124, 124, COLOR_WHITEMASK);
    }

    if (getTitleCount(g_currentUId) > 0 && getTitle(g_currentUId, hid.fullIndex()) != nullptr) {
        auto* title = getTitle(g_currentUId, hid.fullIndex());

        backupList->flush();
        std::vector<std::string> dirs = title->saves();

        for (size_t i = 0; i < dirs.size(); i++) {
            backupList->push_back(theme().c2, theme().c6, dirs.at(i), i == backupList->index());
        }

        if (title->icon().icon) {
            drawOutline(1018, 6, 256, 256, 4, theme().c3);
            SDLH_DrawImageScale(title->icon().icon, 1018, 6, 256, 256);
        }

        // draw infos
        u32 title_w, title_h, h, titleid_w, producer_w, user_w, location_w, subtitle_w, playtime_w;
        auto displayName = title->displayName();
        SDLH_GetTextDimensions(28, displayName.first.c_str(), &title_w, &title_h);
        SDLH_GetTextDimensions(23, "Title: ", &subtitle_w, NULL);
        SDLH_GetTextDimensions(23, "Title ID: ", &titleid_w, &h);
        SDLH_GetTextDimensions(23, "Author: ", &producer_w, NULL);
        SDLH_GetTextDimensions(23, "User: ", &user_w, NULL);
        SDLH_GetTextDimensions(23, "Location: ", &location_w, NULL);

        if (title_w >= 534) {
            displayName.first = displayName.first.substr(0, 24) + "...";
            SDLH_GetTextDimensions(28, displayName.first.c_str(), &title_w, &title_h);
        }

        u8 boxRows = (!displayName.second.empty() ? 5 : 4);

        h += 6;
        if (!title->playTime().empty()) {
            boxRows++;
            SDLH_GetTextDimensions(23, "Play Time: ", &playtime_w, NULL);
        }

        u32 offset = 10 + title_h + h / 2;
        int i      = 0;

        SDLH_DrawRect(534, 2, 482, 16 + title_h, theme().c3);
        SDLH_DrawRect(534, offset - h / 2 - 2, 480, h * boxRows + h / 2, theme().c2);

        SDLH_DrawText(28, 538 - 8 + 482 - title_w, 8, theme().c5, displayName.first.c_str());
        if (!displayName.second.empty()) {
            SDLH_DrawText(23, 538, offset + h * i, theme().c5, "Title:");
            SDLH_DrawTextBox(23, 538 + subtitle_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - subtitle_w, displayName.second.c_str());
        }

        SDLH_DrawText(23, 538, offset + h * i, theme().c5, "Title ID:");
        SDLH_DrawTextBox(
            23, 538 + titleid_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - titleid_w, StringUtils::format("%016llX", title->id()).c_str());

        SDLH_DrawText(23, 538, offset + h * i, theme().c5, "Author:");
        SDLH_DrawTextBox(23, 538 + producer_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - producer_w, title->publisher().c_str());

        SDLH_DrawText(23, 538, offset + h * i, theme().c5, "User:");
        SDLH_DrawTextBox(23, 538 + user_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - user_w, title->userName().c_str());


        SDLH_DrawText(23, 538, offset + h * i, theme().c5, "Location:");
        SDLH_DrawTextBox(23, 538 + location_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - location_w, title->location().c_str());


        if (!title->playTime().empty()) {
            SDLH_DrawText(23, 538, offset + h * i, theme().c5, "Play Time:");
            SDLH_DrawTextBox(23, 538 + playtime_w, offset + h * (i++), theme().c6, 478 - 4 * 2 - playtime_w, title->playTime().c_str());
        }

        drawOutline(538, 276, 414, 380, 4, theme().c3);
        drawOutline(956, 276, 220, 80, 4, theme().c3);
        drawOutline(956, 360, 220, 80, 4, theme().c3);
        backupList->draw(g_backupScrollEnabled);
        buttonBackup->draw(30, COLOR_NULL);
        buttonRestore->draw(30, COLOR_NULL);
    }

    SDL_Color lightBlack = FC_MakeColor(theme().c0.r + 20, theme().c0.g + 20, theme().c0.b + 20, 255);
    u32 ver_w, ver_h, checkpoint_h, checkpoint_w, inst_w, inst_h;
    SDLH_GetTextDimensions(20, ver, &ver_w, &ver_h);
    SDLH_GetTextDimensions(26, "checkpoint", &checkpoint_w, &checkpoint_h);
    SDLH_GetTextDimensions(24, "\ue046 Instructions", &inst_w, &inst_h);

    if (Input::getHeld() & Input::BUTTON_MINUS && currentOverlay == nullptr) {
        SDLH_DrawRect(0, 0, 1280, 720, COLOR_OVERLAY);
        SDLH_DrawText(27, 1205, 646, theme().c6, "\ue085\ue086");
        SDLH_DrawText(24, 58, 69, theme().c6, "\ue058 Tap to select title");
        SDLH_DrawText(24, 58, 109, theme().c6, ("\ue026 Sort: " + sortMode()).c_str());
        SDLH_DrawText(24, 100, 270, theme().c6, "\ue006 \ue080 to scroll between titles");
        SDLH_DrawText(24, 100, 300, theme().c6, "\ue004 \ue005 to scroll between pages");
        SDLH_DrawText(24, 100, 330, theme().c6, "\ue000 to enter the selected title");
        SDLH_DrawText(24, 100, 360, theme().c6, "\ue001 to exit the selected title");
        SDLH_DrawText(24, 100, 390, theme().c6, "\ue002 to change sort mode");
        SDLH_DrawText(24, 100, 420, theme().c6, "\ue003 to multiselect title");
        SDLH_DrawText(24, 100, 450, theme().c6, "Hold \ue003 to select all titles");
        SDLH_DrawText(24, 616, 480, theme().c6, "\ue002 to delete a backup");
    }

    SDLH_DrawRect(0, 672, checkpoint_w + ver_w + 2 * 16 + 8, 40, lightBlack);
    SDLH_DrawText(26, 16, 672 + 40 - checkpoint_h + 2, theme().c6, "checkpoint");
    SDLH_DrawText(20, 16 + checkpoint_w + 8, 672 + 40 - checkpoint_h + checkpoint_h - ver_h, theme().c6, ver);
    SDLH_DrawText(24, 16 * 3 + checkpoint_w + 8 + ver_w, 672 + 40 - checkpoint_h + checkpoint_h - inst_h, theme().c6, "\ue046 Instructions");

    if (g_isTransferringFile) {
        SDLH_DrawRect(0, 0, 1280, 720, COLOR_OVERLAY);

        u32 w, h;
        SDLH_GetTextDimensions(28, g_currentFile.c_str(), &w, &h);
        SDLH_DrawText(28, (1280 - w) / 2, (720 - h) / 2, COLOR_WHITE, g_currentFile.c_str());
    }
}

void MainScreen::update(touchPosition* touch)
{
    updateSelector(touch);
    handleEvents(touch);
}

void MainScreen::updateSelector(touchPosition* touch)
{
    if (!g_backupScrollEnabled) {
        size_t count = getTitleCount(g_currentUId);
        hid.update(count);

        // loop through every rendered title
        for (u8 row = 0; row < rowlen; row++) {
            for (u8 col = 0; col < collen; col++) {
                u8 index = row * collen + col;
                if (index > hid.maxEntries(count))
                    break;

                u32 x = selectorX(index);
                u32 y = selectorY(index);
                if (touch->touched && touch->x >= x && touch->x <= x + 128 && touch->y >= y && touch->y <= y + 128) {
                    hid.index(index);
                }
            }
        }

        backupList->resetIndex();
    }
    else {
        backupList->updateSelection();
    }
}

void MainScreen::handleEvents(touchPosition* touch)
{
    uint32_t kdown = Input::getDown();
    uint32_t kheld = Input::getHeld();

    if (kdown & Input::BUTTON_ZL || kdown & Input::BUTTON_ZR) {
        currentOverlay = std::make_shared<AccountSelectOverlay>(
            *this,
            [this](nn::act::PersistentId uid) {
                g_currentUId = uid;
                this->removeOverlay();

                this->index(TITLES, 0);
                this->index(CELLS, 0);
            },
            [this]() { this->removeOverlay(); });
    }

    // handle touchscreen
    if (!g_backupScrollEnabled && touch->touched && touch->x >= 1200 && touch->x <= 1200 + USER_ICON_SIZE && touch->y >= 626 &&
        touch->y <= 626 + USER_ICON_SIZE) {
        currentOverlay = std::make_shared<AccountSelectOverlay>(
            *this,
            [this](nn::act::PersistentId uid) {
                g_currentUId = uid;
                this->removeOverlay();

                this->index(TITLES, 0);
                this->index(CELLS, 0);
            },
            [this]() { this->removeOverlay(); });
    }

    // Handle touching the backup list
    if ((touch->touched && (int)touch->x > 538 && (int)touch->x < 952 && (int)touch->y > 276 && (int)touch->y < 656)) {
        // Activate backup list only if multiple selections are enabled
        if (!MS::multipleSelectionEnabled()) {
            g_backupScrollEnabled = true;
            updateButtons();
            entryType(CELLS);
        }
    }

    // Handle pressing A
    // Backup list active:   Backup/Restore
    // Backup list inactive: Activate backup list only if multiple
    //                       selections are enabled
    if (kdown & Input::BUTTON_A) {
        if (getTitleCount(g_currentUId) > 0) // TODO this is probably needed on the switch version too
                                             // or else you can select a game on a blank account with no games
        {
            // If backup list is active...
            if (g_backupScrollEnabled) {
                // If the "New..." entry is selected...
                if (0 == this->index(CELLS)) {
                    auto result = io::backup(this->index(TITLES), g_currentUId, this->index(CELLS));
                    if (std::get<0>(result)) {
                        currentOverlay = std::make_shared<InfoOverlay>(*this, std::get<2>(result));
                    }
                    else {
                        currentOverlay = std::make_shared<ErrorOverlay>(*this, std::get<1>(result), std::get<2>(result));
                    }
                }
                else {
                    currentOverlay = std::make_shared<YesNoOverlay>(
                        *this, "Restore selected save?",
                        [this]() {
                            auto result = io::restore(this->index(TITLES), g_currentUId, this->index(CELLS), nameFromCell(this->index(CELLS)));
                            if (std::get<0>(result)) {
                                currentOverlay = std::make_shared<InfoOverlay>(*this, std::get<2>(result));
                            }
                            else {
                                currentOverlay = std::make_shared<ErrorOverlay>(*this, std::get<1>(result), std::get<2>(result));
                            }
                        },
                        [this]() { this->removeOverlay(); });
                }
            }
            else {
                // Activate backup list only if multiple selections are not enabled
                if (!MS::multipleSelectionEnabled()) {
                    g_backupScrollEnabled = true;
                    updateButtons();
                    entryType(CELLS);
                }
            }
        }
    }

    // Handle pressing B
    if (kdown & Input::BUTTON_B || (touch->touched && (int)touch->x >= 0 && (int)touch->x <= 532 && (int)touch->y >= 0 && (int)touch->y <= 664)) {
        this->index(CELLS, 0);
        g_backupScrollEnabled = false;
        entryType(TITLES);
        MS::clearSelectedEntries();
        updateButtons(); // Do this last
    }

    // Handle pressing X
    if (kdown & Input::BUTTON_X) {
        if (g_backupScrollEnabled) {
            size_t index = this->index(CELLS);
            if (index > 0) {
                currentOverlay = std::make_shared<YesNoOverlay>(
                    *this, "Delete selected backup?",
                    [this, index]() {
                        auto* title = getTitle(g_currentUId, this->index(TITLES));
                        if (!title) {
                            return;
                        }
                        std::string path = title->fullPath(index);
                        io::deleteFolderRecursively((path + "/").c_str());
                        refreshDirectories(title->id());
                        this->index(CELLS, index - 1);
                        this->removeOverlay();
                    },
                    [this]() { this->removeOverlay(); });
            }
        }
        else {
            rotateSortMode();
        }
    }

    // Handle pressing Y
    // Backup list active:   Deactivate backup list, select title, and
    //                       enable backup button
    // Backup list inactive: Select title and enable backup button
    if (kdown & Input::BUTTON_Y) {
        if (getTitleCount(g_currentUId) > 0) // TODO this is probably needed on the switch version too
                                             // or else you can select a game on a blank account with no games
        {
            if (g_backupScrollEnabled) {
                this->index(CELLS, 0);
                g_backupScrollEnabled = false;
            }
            entryType(TITLES);
            MS::addSelectedEntry(this->index(TITLES));
            updateButtons(); // Do this last
        }
    }

    // Handle holding Y
    if (kheld & Input::BUTTON_Y && !(g_backupScrollEnabled)) {
        selectionTimer++;
    }
    else {
        selectionTimer = 0;
    }

    if (selectionTimer > 45) {
        MS::clearSelectedEntries();
        for (size_t i = 0, sz = getTitleCount(g_currentUId); i < sz; i++) {
            MS::addSelectedEntry(i);
        }
        selectionTimer = 0;
    }

    // Handle pressing/touching L
    if (buttonBackup->released() || (kdown & Input::BUTTON_L)) {
        if (MS::multipleSelectionEnabled()) {
            resetIndex(CELLS);
            std::vector<size_t> list = MS::selectedEntries();
            for (size_t i = 0, sz = list.size(); i < sz; i++) {
                // check if multiple selection is enabled and don't ask for confirmation if that's the case
                auto result = io::backup(list.at(i), g_currentUId, this->index(CELLS));
                if (std::get<0>(result)) {
                    currentOverlay = std::make_shared<InfoOverlay>(*this, std::get<2>(result));
                }
                else {
                    currentOverlay = std::make_shared<ErrorOverlay>(*this, std::get<1>(result), std::get<2>(result));
                }
            }
            MS::clearSelectedEntries();
            updateButtons();
            blinkLed(4);
            currentOverlay = std::make_shared<InfoOverlay>(*this, "Progress correctly saved to disk.");
        }
        else if (g_backupScrollEnabled) {
            currentOverlay = std::make_shared<YesNoOverlay>(
                *this, "Backup selected save?",
                [this]() {
                    auto result = io::backup(this->index(TITLES), g_currentUId, this->index(CELLS));
                    if (std::get<0>(result)) {
                        currentOverlay = std::make_shared<InfoOverlay>(*this, std::get<2>(result));
                    }
                    else {
                        currentOverlay = std::make_shared<ErrorOverlay>(*this, std::get<1>(result), std::get<2>(result));
                    }
                },
                [this]() { this->removeOverlay(); });
        }
    }

    // Handle pressing/touching R
    if (buttonRestore->released() || (kdown & Input::BUTTON_R)) {
        if (g_backupScrollEnabled) {
            if (this->index(CELLS) != 0) {
                currentOverlay = std::make_shared<YesNoOverlay>(
                    *this, "Restore selected save?",
                    [this]() {
                        auto result = io::restore(this->index(TITLES), g_currentUId, this->index(CELLS), nameFromCell(this->index(CELLS)));
                        if (std::get<0>(result)) {
                            currentOverlay = std::make_shared<InfoOverlay>(*this, std::get<2>(result));
                        }
                        else {
                            currentOverlay = std::make_shared<ErrorOverlay>(*this, std::get<1>(result), std::get<2>(result));
                        }
                    },
                    [this]() { this->removeOverlay(); });
            }
        }
    }
}

std::string MainScreen::nameFromCell(size_t index) const
{
    return backupList->cellName(index);
}

void MainScreen::entryType(entryType_t type_)
{
    type = type_;
}

void MainScreen::resetIndex(entryType_t type)
{
    if (type == TITLES) {
        hid.reset();
    }
    else {
        backupList->resetIndex();
    }
}

size_t MainScreen::index(entryType_t type) const
{
    return type == TITLES ? hid.fullIndex() : backupList->index();
}

void MainScreen::index(entryType_t type, size_t i)
{
    if (type == TITLES) {
        hid.page(i / hid.maxVisibleEntries());
        hid.index(i - hid.page() * hid.maxVisibleEntries());
    }
    else {
        backupList->setIndex(i);
    }
}

void MainScreen::updateButtons(void)
{
    if (MS::multipleSelectionEnabled()) {
        buttonRestore->canChangeColorWhenSelected(true);
        buttonRestore->canChangeColorWhenSelected(false);
        buttonBackup->setColors(theme().c2, theme().c6);
        buttonRestore->setColors(theme().c2, theme().c5);
    }
    else if (g_backupScrollEnabled) {
        buttonBackup->canChangeColorWhenSelected(true);
        buttonRestore->canChangeColorWhenSelected(true);
        buttonBackup->setColors(theme().c2, theme().c6);
        buttonRestore->setColors(theme().c2, theme().c6);
    }
    else {
        buttonBackup->setColors(theme().c2, theme().c6);
        buttonRestore->setColors(theme().c2, theme().c6);
    }

    buttonBackup->text("Backup \ue004");
    buttonRestore->text("Restore \ue005");
}

std::string MainScreen::sortMode() const
{
    switch (g_sortMode) {
        case SORT_LAST_PLAYED:
            return "Last played";
        case SORT_PLAY_TIME:
            return "Play time";
        case SORT_ALPHA:
            return "Alphabetical";
        default:
            break;
    }
    return "";
}
