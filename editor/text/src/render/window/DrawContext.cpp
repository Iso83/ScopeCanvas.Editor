#include <ScopeCanvas/editor/text/render/window/DrawContext.h>
#include <ScopeCanvas/engine/render/gl/OpenGLApi.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Engine::Render::Window;
using namespace ScopeCanvas::Engine::Render::Text;

namespace ScopeCanvas::Editor::Text::Render::Window {
glm::vec4 colorFromAnnotation(Color color) {
    return {static_cast<float>(color.red) / 255.0F, static_cast<float>(color.green) / 255.0F,
            static_cast<float>(color.blue) / 255.0F, static_cast<float>(color.alpha) / 255.0F};
}

glm::vec4 colorForDiff(const ColorProfile& colors, Kind kind, bool gap) {
    if (gap)
        return colorFromAnnotation(colors.gap);

    switch (kind) {
        case Kind::Added:
            return colorFromAnnotation(colors.added);
        case Kind::Removed:
            return colorFromAnnotation(colors.removed);
        case Kind::Modified:
            return colorFromAnnotation(colors.modified);
        case Kind::Unchanged:
            return colorFromAnnotation(colors.unchanged);
    }

    return colorFromAnnotation(colors.unchanged);
}

void clearRect(float x, float y, float width, float height, glm::vec4 color, int framebufferWidth,
               int framebufferHeight) {
    if (width <= 0.0F || height <= 0.0F)
        return;

    const float left = std::clamp(x, 0.0F, static_cast<float>(framebufferWidth));
    const float right = std::clamp(x + width, 0.0F, static_cast<float>(framebufferWidth));
    const float top = std::clamp(y, 0.0F, static_cast<float>(framebufferHeight));
    const float bottom = std::clamp(y + height, 0.0F, static_cast<float>(framebufferHeight));
    if (right <= left || bottom <= top)
        return;

    const GLint scissorX = static_cast<GLint>(std::floor(left));
    const GLint scissorY = static_cast<GLint>(std::floor(static_cast<float>(framebufferHeight) - bottom));
    const GLsizei scissorW = static_cast<GLsizei>(std::ceil(right) - static_cast<float>(scissorX));
    const GLsizei scissorH =
        static_cast<GLsizei>(std::ceil(static_cast<float>(framebufferHeight) - top) - static_cast<float>(scissorY));
    glScissor(scissorX, scissorY, scissorW, scissorH);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DrawContext::draw(Viewport* view) {
    // Canvas owns framebuffer binding and texture lifetime. This pass is only
    // editor-specific painting into that existing ScopeCanvas render target.

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.075F, 0.082F, 0.098F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    if (view == nullptr) {
        m_dirty = false;
        return;
    }

    const auto& camera = view->camera();
    const int width = std::max(camera.getViewportWidth(), 1);
    const int height = std::max(camera.getViewportHeight(), 1);
    const glm::mat4 projection = glm::ortho(0.0F, static_cast<float>(width), 0.0F, static_cast<float>(height));

    const float lineHeight = std::max(m_frame.lineHeight, 1.0F);
    const float glyphAdvance = std::max(m_frame.glyphAdvance, 1.0F);
    glEnable(GL_SCISSOR_TEST);

    for (const auto& row : m_frame.rows) {
        const float rowTop = (static_cast<float>(row.visualRow) - m_frame.verticalRows) * lineHeight;
        clearRect(0.0F, rowTop, static_cast<float>(width), lineHeight, colorForDiff(m_colors, row.diffKind, row.gap),
                  width, height);

        if (row.currentLine)
            clearRect(0.0F, rowTop, static_cast<float>(width), lineHeight, colorFromAnnotation(m_colors.currentLine),
                      width, height);

        for (const auto& marker : row.hiddenCharacterMarkers) {
            const float x =
                m_frame.codeOriginX - m_frame.horizontalOffset + static_cast<float>(marker.startColumn) * glyphAdvance;
            const float markerWidth = static_cast<float>(marker.endColumn - marker.startColumn) * glyphAdvance;
            const float markerTop = rowTop + lineHeight * 0.12F;
            const float markerHeight = lineHeight * 0.76F;
            clearRect(x, markerTop, markerWidth, markerHeight, colorFromAnnotation(m_colors.hiddenBackground), width,
                      height);
            const glm::vec4 border = colorFromAnnotation(m_colors.hiddenBorder);
            clearRect(x, markerTop, markerWidth, 1.0F, border, width, height);
            clearRect(x, markerTop + markerHeight - 1.0F, markerWidth, 1.0F, border, width, height);
            clearRect(x, markerTop, 1.0F, markerHeight, border, width, height);
            clearRect(x + markerWidth - 1.0F, markerTop, 1.0F, markerHeight, border, width, height);
        }

        for (const auto& span : row.backgroundSpans) {
            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(span.startColumn) * glyphAdvance,
                      rowTop, static_cast<float>(span.endColumn - span.startColumn) * glyphAdvance, lineHeight,
                      colorFromAnnotation(span.color), width, height);
        }

        if (row.selection) {
            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(row.selection->startColumn) * glyphAdvance,
                      rowTop, static_cast<float>(row.selection->endColumn - row.selection->startColumn) * glyphAdvance,
                      lineHeight, colorFromAnnotation(m_colors.selection), width, height);
        }

        for (const auto& span : row.backgroundSpans) {
            if (span.id != "active-find")
                continue;

            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(span.startColumn) * glyphAdvance,
                      rowTop + lineHeight - 3.0F, static_cast<float>(span.endColumn - span.startColumn) * glyphAdvance,
                      3.0F, colorFromAnnotation(m_colors.activeFind), width, height);
        }

        if (row.foldGutterGuide) {
            const float guideX = m_frame.lineNumberGutterWidth + m_frame.markerGutterWidth * 0.5F;
            clearRect(guideX, rowTop, std::max(1.0F, lineHeight * 0.06F), lineHeight,
                      colorFromAnnotation(m_colors.foldGuide), width, height);
        }

        if (row.foldControl) {
            clearRect(m_frame.lineNumberGutterWidth, rowTop + lineHeight * 0.2F,
                      std::max(m_frame.markerGutterWidth * 0.65F, 4.0F), std::max(lineHeight * 0.6F, 4.0F),
                      colorFromAnnotation(m_colors.foldControl), width, height);
        }
    }

    glDisable(GL_SCISSOR_TEST);

    if (m_textRenderer != nullptr && m_textRenderer->ready()) {
        const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const auto& row : m_frame.rows) {
            const float rowTop = (static_cast<float>(row.visualRow) - m_frame.verticalRows) * lineHeight;
            const float baseline = static_cast<float>(height) - rowTop - m_frame.baselineFromRowTop;
            const ClipRect lineClip{{0.0F, static_cast<float>(height) - rowTop - lineHeight},
                                    {m_frame.lineNumberGutterWidth, lineHeight}};

            if (!row.lineNumber.empty())
                m_textRenderer->render(
                    row.lineNumber,
                    {m_frame.lineNumberGutterWidth - m_frame.lineNumberPadding - row.lineNumber.size() * glyphAdvance,
                     baseline},
                    colorFromAnnotation(m_colors.lineNumber), projection, lineClip, m_frame.fontWorldScale);

            if (row.foldControl) {
                const ClipRect markerClip{
                    {m_frame.lineNumberGutterWidth, static_cast<float>(height) - rowTop - lineHeight},
                    {m_frame.markerGutterWidth, lineHeight}};
                m_textRenderer->render(row.foldControl->collapsed ? "+" : "-",
                                       {m_frame.lineNumberGutterWidth + m_frame.markerGutterWidth * 0.25F, baseline},
                                       colorFromAnnotation(m_colors.foldText), projection, markerClip,
                                       m_frame.fontWorldScale);
            }

            const ClipRect codeClip{{m_frame.codeOriginX, 0.0F},
                                    {static_cast<float>(width) - m_frame.codeOriginX, static_cast<float>(height)}};
            if (!row.text.empty())
                m_textRenderer->render(row.text, {m_frame.codeOriginX - m_frame.horizontalOffset, baseline},
                                       colorFromAnnotation(m_colors.text), projection, codeClip,
                                       m_frame.fontWorldScale);

            for (const auto& span : row.foregroundSpans) {
                const std::size_t start = std::min(span.startColumn, row.text.size());
                const std::size_t end = std::min(span.endColumn, row.text.size());
                if (start >= end)
                    continue;

                m_textRenderer->render(
                    std::string_view(row.text).substr(start, end - start),
                    {m_frame.codeOriginX - m_frame.horizontalOffset + static_cast<float>(start) * glyphAdvance,
                     baseline},
                    colorFromAnnotation(span.color), projection, codeClip, m_frame.fontWorldScale);
            }
        }

        if (blendWasEnabled == GL_FALSE)
            glDisable(GL_BLEND);
    }

    glEnable(GL_SCISSOR_TEST);
    for (const auto& row : m_frame.rows) {
        const float rowTop = (static_cast<float>(row.visualRow) - m_frame.verticalRows) * lineHeight;
        for (const auto& decoration : row.decorations) {
            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(decoration.startColumn) * glyphAdvance,
                      rowTop + lineHeight - 2.0F,
                      static_cast<float>(decoration.endColumn - decoration.startColumn) * glyphAdvance, 1.0F,
                      colorFromAnnotation(decoration.color), width, height);
        }
        for (const auto& diagnostic : row.diagnostics) {
            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(diagnostic.startColumn) * glyphAdvance,
                      rowTop + lineHeight - 3.0F,
                      static_cast<float>(diagnostic.endColumn - diagnostic.startColumn) * glyphAdvance, 2.0F,
                      colorFromAnnotation(diagnostic.color), width, height);
        }

        if (row.caret) {
            clearRect(m_frame.codeOriginX - m_frame.horizontalOffset +
                          static_cast<float>(row.caret->column) * glyphAdvance,
                      rowTop + 2.0F, 1.5F, std::max(lineHeight - 4.0F, 1.0F), colorFromAnnotation(m_colors.caret),
                      width, height);
        }
    }

    glDisable(GL_SCISSOR_TEST);
    m_dirty = false;
}
} // namespace ScopeCanvas::Editor::Text::Render::Window
