#pragma once

#include <ScopeCanvas/editor/text/Document.h>
#include <ScopeCanvas/editor/text/annotation/Decorations.h>
#include <ScopeCanvas/editor/text/annotation/Diagnostics.h>
#include <ScopeCanvas/editor/text/annotation/Folding.h>
#include <ScopeCanvas/editor/text/annotation/Segments.h>
#include <ScopeCanvas/editor/text/annotation/Spans.h>
#include <algorithm>
#include <vector>

namespace ScopeCanvas::Editor::Text::Annotation {
class Model {
private:
    std::vector<ForegroundSpan> m_foreground;
    std::vector<BackgroundSpan> m_background;
    std::vector<Diagnostic> m_diagnostics;
    std::vector<TextDecoration> m_decorations;
    std::vector<FoldRegion> m_folds;

public:
    [[nodiscard]] const std::vector<ForegroundSpan>& foregroundSpans() const noexcept {
        return m_foreground;
    }
    [[nodiscard]] const std::vector<BackgroundSpan>& backgroundSpans() const noexcept {
        return m_background;
    }
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept {
        return m_diagnostics;
    }
    [[nodiscard]] const std::vector<TextDecoration>& decorations() const noexcept {
        return m_decorations;
    }
    [[nodiscard]] const std::vector<FoldRegion>& foldRegions() const noexcept {
        return m_folds;
    }

    void setForegroundSpans(const Document& document, std::vector<ForegroundSpan> spans);
    void setBackgroundSpans(const Document& document, std::vector<BackgroundSpan> spans);
    void setDiagnostics(const Document& document, std::vector<Diagnostic> diagnostics);
    void setDecorations(const Document& document, std::vector<TextDecoration> decorations);
    void setFoldRegions(const Document& document, std::vector<FoldRegion> regions);

    bool setFoldCollapsed(const std::string& id, bool collapsed) noexcept {
        const auto found = std::ranges::find(m_folds, id, &FoldRegion::id);
        if (found == m_folds.end())
            return false;
        found->collapsed = collapsed;
        return true;
    }

    void synchronize(const Document& document);

    [[nodiscard]] std::vector<StyledLineSegment> resolvedForeground(const Document& document) const;
    [[nodiscard]] std::vector<LineSegment> clippedBackground(const Document& document, std::size_t index) const {
        return index < m_background.size() ? document.clipToLines(m_background[index].range)
                                           : std::vector<LineSegment>{};
    }

    [[nodiscard]] std::vector<LineSegment> clippedDiagnostic(const Document& document, std::size_t index) const {
        return index < m_diagnostics.size() ? document.clipToLines(m_diagnostics[index].range)
                                            : std::vector<LineSegment>{};
    }

    [[nodiscard]] const FoldRegion* foldRegion(const std::string& id) const noexcept {
        const auto found = std::ranges::find(m_folds, id, &FoldRegion::id);
        return found == m_folds.end() ? nullptr : &*found;
    }
};
} // namespace ScopeCanvas::Editor::Text::Annotation
