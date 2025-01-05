#ifndef ACCOUNTSELECTOVERLAY_HPP
#define ACCOUNTSELECTOVERLAY_HPP

#include "Overlay.hpp"
#include "SDLHelper.hpp"
#include "account.hpp"
#include "clickable.hpp"
#include "colors.hpp"
#include "hid.hpp"
#include "input.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Clickable;

class AccountSelectOverlay : public Overlay {
public:
    AccountSelectOverlay(
        Screen& screen, const std::function<void(nn::act::PersistentId)>& callbackSelectedUser, const std::function<void()>& callbackCancel);
    ~AccountSelectOverlay() override = default;
    void draw() const override;
    void update(touchPosition* touch) override;

private:
    Hid<HidDirection::HORIZONTAL, HidDirection::HORIZONTAL> hid;
    std::vector<User> users;
    std::function<void(nn::act::PersistentId)> select;
    std::function<void()> cancel;
};

#endif
