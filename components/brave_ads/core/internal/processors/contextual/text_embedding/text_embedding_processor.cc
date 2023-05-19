/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/processors/contextual/text_embedding/text_embedding_processor.h"

#include "base/ranges/algorithm.h"
#include "brave/components/brave_ads/core/internal/ads/serving/targeting/contextual/text_embedding/text_embedding_feature.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/common/search_engine/search_engine_results_page_util.h"
#include "brave/components/brave_ads/core/internal/common/search_engine/search_engine_util.h"
#include "brave/components/brave_ads/core/internal/ml/pipeline/text_processing/embedding_info.h"
#include "brave/components/brave_ads/core/internal/ml/pipeline/text_processing/embedding_processing.h"
#include "brave/components/brave_ads/core/internal/processors/contextual/text_embedding/text_embedding_html_events.h"
#include "brave/components/brave_ads/core/internal/processors/contextual/text_embedding/text_embedding_processor_util.h"
#include "brave/components/brave_ads/core/internal/resources/contextual/text_embedding/text_embedding_resource.h"
#include "brave/components/brave_ads/core/internal/tabs/tab_manager.h"
#include "url/gurl.h"

namespace brave_ads {

TextEmbeddingProcessor::TextEmbeddingProcessor(TextEmbeddingResource& resource)
    : resource_(resource) {
  TabManager::GetInstance().AddObserver(this);
}

TextEmbeddingProcessor::~TextEmbeddingProcessor() {
  TabManager::GetInstance().RemoveObserver(this);
}

void TextEmbeddingProcessor::Process(const std::string& html) {
  if (!resource_->IsInitialized()) {
    return BLOG(
        1, "Failed to process text embeddings as resource not initialized");
  }

  const std::string text = SanitizeHtml(html);
  if (text.empty()) {
    return BLOG(1, "No text available for embedding");
  }

  const ml::pipeline::TextEmbeddingInfo text_embedding =
      resource_->get().EmbedText(text);
  if (text_embedding.embedding.empty()) {
    return BLOG(1, "Embedding is empty");
  }

  if (*base::ranges::min_element(text_embedding.embedding) == 0.0 &&
      *base::ranges::max_element(text_embedding.embedding) == 0.0) {
    return BLOG(1, "Not enough words to embed text");
  }

  LogTextEmbeddingHtmlEvent(
      BuildTextEmbeddingHtmlEvent(text_embedding),
      base::BindOnce([](const bool success) {
        if (!success) {
          return BLOG(1, "Failed to log text embedding HTML event");
        }

        BLOG(3, "Successfully logged text embedding HTML event");

        PurgeStaleTextEmbeddingHtmlEvents(
            base::BindOnce([](const bool success) {
              if (!success) {
                return BLOG(1,
                            "Failed to purge stale text embedding HTML events");
              }

              BLOG(3, "Successfully purged stale text embedding HTML events");
            }));
      }));
}

///////////////////////////////////////////////////////////////////////////////

void TextEmbeddingProcessor::OnHtmlContentDidChange(
    const int32_t /*tab_id*/,
    const std::vector<GURL>& redirect_chain,
    const std::string& content) {
  if (redirect_chain.empty()) {
    return;
  }

  const GURL& url = redirect_chain.back();

  if (!url.SchemeIsHTTPOrHTTPS()) {
    return BLOG(1,
                url.scheme()
                    << " scheme is not supported for processing HTML content");
  }

  if (IsSearchEngine(url) && !IsSearchEngineResultsPage(url)) {
    return BLOG(1,
                "Search engine landing pages are not supported for processing "
                "HTML content");
  }

  if (!IsTextEmbeddingFeatureEnabled()) {
    return;
  }

  Process(content);
}

}  // namespace brave_ads
