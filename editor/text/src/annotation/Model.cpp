#include <ScopeCanvas/editor/text/annotation/Model.h>
#include <set>

namespace ScopeCanvas::Editor::Text::Annotation {

template <typename T> void normalizeAndFilter(const Document& document, std::vector<T>& values) {
    std::erase_if(values, [&document](T& value) {
        value.range = value.range.normalized();
        return value.range.empty() || !document.valid(value.range);
    });
}

void Model::setForegroundSpans(const Document& document, std::vector<ForegroundSpan> spans) {
    normalizeAndFilter(document, spans);
    m_foreground = std::move(spans);
}

void Model::setBackgroundSpans(const Document& document, std::vector<BackgroundSpan> spans) {
    normalizeAndFilter(document, spans);
    m_background = std::move(spans);
}

void Model::setDiagnostics(const Document& document, std::vector<Diagnostic> diagnostics) {
    normalizeAndFilter(document, diagnostics);
    m_diagnostics = std::move(diagnostics);
}

void Model::setDecorations(const Document& document, std::vector<TextDecoration> decorations) {
    normalizeAndFilter(document, decorations);
    std::ranges::stable_sort(decorations, {}, &TextDecoration::precedence);
    m_decorations = std::move(decorations);
}

void Model::setFoldRegions(const Document& document, std::vector<FoldRegion> regions) {
    normalizeAndFilter(document, regions);
    std::set<std::string> ids;
    std::erase_if(regions,
                  [&ids](const FoldRegion& region) { return region.id.empty() || !ids.insert(region.id).second; });
    m_folds = std::move(regions);
}

void Model::synchronize(const Document& document) {
    normalizeAndFilter(document, m_foreground);
    normalizeAndFilter(document, m_background);
    normalizeAndFilter(document, m_diagnostics);
    normalizeAndFilter(document, m_decorations);
    normalizeAndFilter(document, m_folds);
}

std::vector<StyledLineSegment> Model::resolvedForeground(const Document& document) const {
    std::vector<StyledLineSegment> result;

    struct Candidate {
        std::size_t spanIndex{};
        std::size_t startColumn{};
        std::size_t endColumn{};
    };

    std::vector<std::vector<Candidate>> candidates(document.lineCount());

    for (std::size_t spanIndex = 0; spanIndex < m_foreground.size(); ++spanIndex) {
        for (const auto& segment : document.clipToLines(m_foreground[spanIndex].range))
            candidates[segment.line].push_back({spanIndex, segment.startColumn, segment.endColumn});
    }

    for (std::size_t line = 0; line < document.lineCount(); ++line) {
        std::vector<std::size_t> boundaries;
        boundaries.reserve(candidates[line].size() * 2U);

        for (const auto& candidate : candidates[line]) {
            boundaries.push_back(candidate.startColumn);
            boundaries.push_back(candidate.endColumn);
        }

        std::ranges::sort(boundaries);
        const auto uniqueEnd = std::ranges::unique(boundaries).begin();
        boundaries.erase(uniqueEnd, boundaries.end());

        for (std::size_t boundary = 1; boundary < boundaries.size(); ++boundary) {
            const auto start = boundaries[boundary - 1];
            const auto end = boundaries[boundary];

            std::optional<std::size_t> winner;
            for (const auto& candidate : candidates[line]) {
                const auto& span = m_foreground[candidate.spanIndex];
                const bool covers = candidate.startColumn <= start && candidate.endColumn >= end;

                if (covers && (!winner || span.precedence >= m_foreground[*winner].precedence))
                    winner = candidate.spanIndex;
            }

            if (winner) {
                const auto& span = m_foreground[*winner];
                result.push_back(
                    {{line, start, end}, span.color, span.style, span.classification, span.id, span.precedence});
            }
        }
    }

    return result;
}
} // namespace ScopeCanvas::Editor::Text::Annotation
