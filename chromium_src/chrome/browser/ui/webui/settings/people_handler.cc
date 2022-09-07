/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/sync/driver/brave_sync_service_impl.h"

// IsSetupInProgress isn't accurate in brave sync flow especially for the first
// time setup, we rely on it to display setup dialog
#define BRAVE_GET_SYNC_STATUS_DICTIONARY                        \
  sync_status.SetBoolPath(                                      \
      "firstSetupInProgress",                                   \
      service && !disallowed_by_policy &&                       \
          !service->GetUserSettings()->IsFirstSetupComplete()); \
  {                                                             \
    syncer::BraveSyncServiceImpl* brave_sync_service =          \
        static_cast<syncer::BraveSyncServiceImpl*>(service);    \
    if (brave_sync_service) {                                   \
      bool failed_to_decrypt = false;                           \
      brave_sync_service->prefs().GetSeed(&failed_to_decrypt);  \
      sync_status.SetBoolPath("hasSyncWordsDecryptionError",    \
                              failed_to_decrypt);               \
    }                                                           \
  }

#include "src/chrome/browser/ui/webui/settings/people_handler.cc"
#undef BRAVE_GET_SYNC_STATUS_DICTIONARY
