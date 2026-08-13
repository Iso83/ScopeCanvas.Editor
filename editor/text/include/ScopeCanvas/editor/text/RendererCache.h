#pragma once

#include <ScopeCanvas/engine/render/text/Renderer.h>
#include <cmath>
#include <map>
#include <memory>
#include <string>

namespace ScopeCanvas::Editor::Text {
class RendererCache {
private:
    unsigned int m_bucketStep{};
    unsigned int m_maxCachedRenderers{};
    std::map<std::pair<std::string, unsigned int>, std::shared_ptr<Engine::Render::Text::Renderer>> m_renderers;

public:
    struct Bucket {
        unsigned int pixelSize{};
        float worldScale{1.0F};
    };

    explicit RendererCache(unsigned int bucketStep = 2U, unsigned int maxCachedRenderers = 8U)
        : m_bucketStep(std::max(bucketStep, 1U)), m_maxCachedRenderers(std::max(maxCachedRenderers, 1U)) {}
    ~RendererCache() = default;
    RendererCache(const RendererCache&) = delete;

    RendererCache& operator=(const RendererCache&) = delete;

    [[nodiscard]] Bucket bucketFor(float effectivePixelSize) const {
        const float requested = std::max(effectivePixelSize, 1.0F);
        const auto rounded =
            static_cast<unsigned int>(std::max(1.0F, std::round(requested / m_bucketStep))) * m_bucketStep;
        return {std::max(rounded, 1U), requested / static_cast<float>(std::max(rounded, 1U))};
    }
    [[nodiscard]] std::shared_ptr<Engine::Render::Text::Renderer> rendererFor(const std::string& fontPath,
                                                                              float effectivePixelSize);
    void clear() {
        m_renderers.clear();
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return m_renderers.size();
    }
};
} // namespace ScopeCanvas::Editor::Text
