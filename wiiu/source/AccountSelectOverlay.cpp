#include "AccountSelectOverlay.hpp"

AccountSelectOverlay::AccountSelectOverlay(
    Screen& screen, const std::function<void(nn::act::PersistentId)>& callbackSelectedUser, const std::function<void()>& callbackCancel)
    : Overlay(screen), hid(12, 12)
{
    select = callbackSelectedUser;
    cancel = callbackCancel;

    for (uint8_t i = 1; i <= 12; i++) {
        auto userOpt = Account::getUserFromSlot(i);
        if (userOpt) {
            if (userOpt->id == g_currentUId) {
                hid.index(i - 1);
            }

            users.push_back(std::move(*userOpt));
        }
    }
}

void AccountSelectOverlay::draw() const
{
    SDLH_DrawRect(0, 0, 1280, 720, COLOR_OVERLAY);

    int w = 128, h = 128;
    for (uint32_t i = 0; i < users.size(); i++) {
        int center     = 1280 / 2;
        int totalWidth = users.size() * (w + 10);
        int x          = center - (totalWidth / 2) + (i * (w + 10));
        int y          = 360;

        u32 username_w, username_h;
        std::string username = trimToFit(users[i].name, w - g_username_dotsize, 18);
        SDLH_GetTextDimensions(18, username.c_str(), &username_w, &username_h);

        SDLH_DrawRect(x, y, w, h + username_h + 5, theme().c1);
        SDLH_DrawRect(x, y, w, h, theme().c6);
        SDLH_DrawRect(x + 2, y + 2, w - 4, h - 4, theme().c1);
        SDLH_DrawImageScale(users[i].icon, x, y - 2, w, h);

        SDLH_DrawText(18, x + (w - username_w) / 2, y + h + 5, theme().c6, username.c_str());

        if (i == hid.index()) {
            drawPulsingOutline(x, y, w, h + username_h + 5, 4, COLOR_BLUE);
        }
    }
}

void AccountSelectOverlay::update(touchPosition* touch)
{
    hid.update(users.size());

    if (Input::getDown() & Input::BUTTON_A) {
        select(users[hid.index()].id);
    }
    else if (Input::getDown() & Input::BUTTON_B) {
        cancel();
    }
}