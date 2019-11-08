/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/content_settings/core/common/content_settings.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "url/gurl.h"

using brave_shields::ControlType;

class CookiePrefServiceTest : public InProcessBrowserTest {
 public:
  CookiePrefServiceTest() = default;
  ~CookiePrefServiceTest() override = default;

  Profile* profile() { return browser()->profile(); }

  ContentSetting GetCookiePref() {
    return IntToContentSetting(profile()->GetPrefs()->GetInteger(
        "profile.default_content_setting_values.cookies"));
  }

  void SetThirdPartyCookiePref(bool setting) {
    profile()->GetPrefs()->SetBoolean(
        prefs::kBlockThirdPartyCookies, setting);
  }

  void SetCookiePref(ContentSetting setting) {
    profile()->GetPrefs()->SetInteger(
        "profile.default_content_setting_values.cookies", setting);
  }
};

IN_PROC_BROWSER_TEST_F(CookiePrefServiceTest, CookieControlType_Preference) {
  // Initial state
  auto setting = brave_shields::GetCookieControlType(profile(), GURL());
  EXPECT_EQ(ControlType::BLOCK_THIRD_PARTY, setting);
  EXPECT_EQ(CONTENT_SETTING_ALLOW, GetCookiePref());

  // Control -> preference
  /* BLOCK */
  brave_shields::SetCookieControlType(profile(), ControlType::BLOCK, GURL());
  EXPECT_EQ(CONTENT_SETTING_BLOCK, GetCookiePref());

  /* ALLOW */
  brave_shields::SetCookieControlType(profile(), ControlType::ALLOW, GURL());
  EXPECT_EQ(CONTENT_SETTING_ALLOW, GetCookiePref());

  /* BLOCK_THIRD_PARTY */
  brave_shields::SetCookieControlType(profile(), ControlType::BLOCK, GURL());
  EXPECT_EQ(CONTENT_SETTING_BLOCK, GetCookiePref());
  brave_shields::SetCookieControlType(profile(), ControlType::BLOCK_THIRD_PARTY,
                                      GURL());
  EXPECT_EQ(CONTENT_SETTING_ALLOW, GetCookiePref());

  // Preference -> control
  /* BLOCK */
  SetCookiePref(CONTENT_SETTING_BLOCK);
  EXPECT_EQ(ControlType::BLOCK,
            brave_shields::GetCookieControlType(profile(), GURL()));

  /* ALLOW */
  SetCookiePref(CONTENT_SETTING_ALLOW);
  SetThirdPartyCookiePref(false);
  EXPECT_EQ(ControlType::ALLOW,
            brave_shields::GetCookieControlType(profile(), GURL()));

  // Preserve CONTENT_SETTING_SESSION_ONLY
  SetCookiePref(CONTENT_SETTING_BLOCK);
  EXPECT_EQ(ControlType::BLOCK,
            brave_shields::GetCookieControlType(profile(), GURL()));
  SetCookiePref(CONTENT_SETTING_SESSION_ONLY);
  SetThirdPartyCookiePref(false);
  EXPECT_EQ(ControlType::ALLOW,
            brave_shields::GetCookieControlType(profile(), GURL()));
  SetCookiePref(CONTENT_SETTING_ALLOW);
  SetThirdPartyCookiePref(false);
  EXPECT_EQ(ControlType::ALLOW,
            brave_shields::GetCookieControlType(profile(), GURL()));
}
