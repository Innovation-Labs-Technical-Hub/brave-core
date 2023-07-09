/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/ads/serving/permission_rules/promoted_content_ads/promoted_content_ads_per_day_permission_rule.h"

#include "brave/components/brave_ads/core/internal/ads/ad_events/ad_event_unittest_util.h"
#include "brave/components/brave_ads/core/internal/ads/promoted_content_ad_feature.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsPromotedContentAdsPerDayPermissionRuleTest : public UnitTestBase {
 protected:
  const PromotedContentAdsPerDayPermissionRule permission_rule_;
};

TEST_F(BraveAdsPromotedContentAdsPerDayPermissionRuleTest,
       AllowAdIfThereAreNoAdEvents) {
  // Arrange

  // Act

  // Assert
  EXPECT_TRUE(permission_rule_.ShouldAllow().has_value());
}

TEST_F(BraveAdsPromotedContentAdsPerDayPermissionRuleTest,
       AllowAdIfDoesNotExceedCap) {
  // Arrange

  // Act
  RecordAdEvents(AdType::kPromotedContentAd, ConfirmationType::kServed,
                 /*count*/ kMaximumPromotedContentAdsPerDay.Get() - 1);

  // Assert
  EXPECT_TRUE(permission_rule_.ShouldAllow().has_value());
}

TEST_F(BraveAdsPromotedContentAdsPerDayPermissionRuleTest,
       AllowAdIfDoesNotExceedCapAfter1Day) {
  // Arrange
  RecordAdEvents(AdType::kPromotedContentAd, ConfirmationType::kServed,
                 /*count*/ kMaximumPromotedContentAdsPerDay.Get());

  // Act
  AdvanceClockBy(base::Days(1));

  // Assert
  EXPECT_TRUE(permission_rule_.ShouldAllow().has_value());
}

TEST_F(BraveAdsPromotedContentAdsPerDayPermissionRuleTest,
       DoNotAllowAdIfExceedsCapWithin1Day) {
  // Arrange
  RecordAdEvents(AdType::kPromotedContentAd, ConfirmationType::kServed,
                 /*count*/ kMaximumPromotedContentAdsPerDay.Get());

  // Act
  AdvanceClockBy(base::Days(1) - base::Milliseconds(1));

  // Assert
  EXPECT_FALSE(permission_rule_.ShouldAllow().has_value());
}

}  // namespace brave_ads
