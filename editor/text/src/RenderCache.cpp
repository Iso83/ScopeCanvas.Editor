#include <ScopeCanvas/editor/text/RendererCache.h>

namespace ScopeCanvas::Editor::Text {
std::shared_ptr<Engine::Render::Text::Renderer> RendererCache::rendererFor(const std::string& fontPath,
                                                                           float effectivePixelSize) {
    const Bucket bucket = bucketFor(effectivePixelSize);
    const auto key = std::make_pair(fontPath, bucket.pixelSize);
    if (const auto found = m_renderers.find(key); found != m_renderers.end())
        return found->second;

    auto renderer = std::make_shared<Engine::Render::Text::Renderer>();
    if (!renderer->init() || !renderer->loadFont(fontPath, bucket.pixelSize))
        return nullptr;

    m_renderers.emplace(key, renderer);
    while (m_renderers.size() > m_maxCachedRenderers) {
        auto evicted = m_renderers.begin();
        if (evicted->first == key)
            ++evicted;

        if (evicted == m_renderers.end())
            break;

        m_renderers.erase(evicted);
    }

    return renderer;
}
} // namespace ScopeCanvas::Editor::Text
