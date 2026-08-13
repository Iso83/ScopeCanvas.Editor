#include "TestAssert.h"

#include <ScopeCanvas/editor/text/render/window/DrawContext.h>

using namespace ScopeCanvas::Editor::Text::Render::Window;

int test_retains_renderer_ownership() {
    DrawContext context;
    auto renderer = std::make_shared<ScopeCanvas::Engine::Render::Text::Renderer>();
    std::weak_ptr<ScopeCanvas::Engine::Render::Text::Renderer> lifetime = renderer;
    context.setTextRenderer(renderer);
    renderer.reset();
    CPPTEST_ASSERT(!lifetime.expired());
    context.setTextRenderer({});
    CPPTEST_ASSERT(lifetime.expired());
    return 0;
}

int main() {
    CPPTEST_RUN(test_retains_renderer_ownership);
    return 0;
}
