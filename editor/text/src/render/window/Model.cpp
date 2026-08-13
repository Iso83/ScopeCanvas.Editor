#include <ScopeCanvas/editor/text/render/window/Frame.h>
#include <ScopeCanvas/editor/text/render/window/Model.h>
#include <algorithm>
#include <cmath>

using namespace ScopeCanvas::Editor::Text::Annotation;
using namespace ScopeCanvas::Editor::Text::Diff;
using namespace ScopeCanvas::Editor::Text::Session;
using namespace ScopeCanvas::Editor::Text::Projection;

namespace ScopeCanvas::Editor::Text::Render::Window {

constexpr std::size_t TabWidth = 4U;

std::size_t codePointBytes(std::string_view text, std::size_t offset) noexcept {
    const unsigned char lead = static_cast<unsigned char>(text[offset]);
    std::size_t count = lead < 0x80U ? 1U : lead < 0xE0U ? 2U : lead < 0xF0U ? 3U : 4U;
    if (offset + count > text.size())
        return 1U;

    for (std::size_t index = 1; index < count; ++index) {
        if ((static_cast<unsigned char>(text[offset + index]) & 0xC0U) != 0x80U)
            return 1U;
    }

    return count;
}

struct RenderedLine {
    std::string text;
    std::vector<std::size_t> sourceToVisual;
    std::vector<std::size_t> visualToSource;
    std::vector<std::pair<std::size_t, std::size_t>> hiddenMarkers;

    RenderedLine(const Document& document, std::size_t line, bool showHiddenCharacters);
};

RenderedLine::RenderedLine(const Document& document, std::size_t line, bool showHiddenCharacters) {
    const std::string source(document.line(line));
    sourceToVisual.resize(source.size() + 1U);
    visualToSource.push_back(0U);
    std::size_t visualColumn = 0U;

    for (std::size_t sourceColumn = 0; sourceColumn < source.size();) {
        sourceToVisual[sourceColumn] = visualColumn;
        if (source[sourceColumn] == '\t') {
            const std::size_t markerStart = visualColumn;
            const std::size_t spaces = TabWidth - visualColumn % TabWidth;
            text += showHiddenCharacters ? '>' : ' ';
            text.append(spaces - 1U, ' ');

            for (std::size_t index = 0; index < spaces; ++index)
                visualToSource.push_back(sourceColumn + 1U);

            visualColumn += spaces;

            if (showHiddenCharacters)
                hiddenMarkers.emplace_back(markerStart, visualColumn);
            ++sourceColumn;
        } else {
            const std::size_t bytes = codePointBytes(source, sourceColumn);
            text += showHiddenCharacters && source[sourceColumn] == ' '
                        ? std::string_view{"."}
                        : std::string_view{source}.substr(sourceColumn, bytes);

            for (std::size_t index = 1; index < bytes; ++index)
                sourceToVisual[sourceColumn + index] = visualColumn;

            ++visualColumn;
            sourceColumn += bytes;
            visualToSource.push_back(sourceColumn);
        }
    }

    sourceToVisual[source.size()] = visualColumn;
    if (showHiddenCharacters) {
        std::string marker;
        if (document.lineEnding(line) == LineEnding::CRLF)
            marker = " CRLF";
        else if (document.lineEnding(line) == LineEnding::LF)
            marker = " LF";
        else if (line + 1U == document.lineCount())
            marker = " EOF";
        const std::size_t markerStart = visualColumn;
        text += marker;
        visualToSource.insert(visualToSource.end(), marker.size(), source.size());
        if (!marker.empty())
            hiddenMarkers.emplace_back(markerStart, markerStart + marker.size());
    }
}

std::size_t visualColumn(const Row& row, std::size_t sourceColumn) {
    if (row.sourceToVisualColumns.empty())
        return sourceColumn;

    return row.sourceToVisualColumns[std::min(sourceColumn, row.sourceToVisualColumns.size() - 1U)];
}

void mapSegmentsToVisualColumns(Row& row) {
    auto map = [&](auto& segments) {
        for (auto& segment : segments) {
            segment.startColumn = visualColumn(row, segment.startColumn);
            segment.endColumn = visualColumn(row, segment.endColumn);
        }
    };
    map(row.backgroundSpans);
    map(row.foregroundSpans);
    map(row.diagnostics);
    map(row.decorations);
}

const std::vector<Layer> LayerOrder{
    Layer::RowBackground,        Layer::CurrentLine, Layer::BackgroundAnnotation, Layer::Selection, Layer::Text,
    Layer::ForegroundAnnotation, Layer::Decoration,  Layer::Diagnostic,           Layer::Caret,     Layer::FoldControl,
};

Segment segmentFromLine(const LineSegment& segment, Color color, std::string id = {}, int precedence = 0) {
    return {segment.startColumn,      segment.endColumn, color,     FontStyle::Regular,
            DiagnosticSeverity::Hint, std::move(id),     precedence};
}

Segment segmentFromStyled(const StyledLineSegment& segment) {
    return {segment.startColumn,      segment.endColumn, segment.color,     segment.style,
            DiagnosticSeverity::Hint, segment.id,        segment.precedence};
}

Color severityColor(DiagnosticSeverity severity) {
    switch (severity) {
        case DiagnosticSeverity::Warning:
            return {230, 180, 70, 255};
        case DiagnosticSeverity::Error:
            return {220, 80, 80, 255};
        case DiagnosticSeverity::Hint:
            return {120, 160, 220, 255};
    }
    return {120, 160, 220, 255};
}

void appendRowAnnotations(Row& row, const Document& document, const Annotation::Model& annotations,
                          const std::vector<StyledLineSegment>& resolvedForeground) {
    if (!row.logicalLine)
        return;

    for (const auto& span : annotations.backgroundSpans()) {
        for (const auto& segment : document.clipToLines(span.range)) {
            if (segment.line == *row.logicalLine)
                row.backgroundSpans.push_back(segmentFromLine(segment, span.color, span.id, span.precedence));
        }
    }

    const auto first = std::ranges::lower_bound(resolvedForeground, *row.logicalLine, {}, &StyledLineSegment::line);
    const auto last =
        std::ranges::upper_bound(first, resolvedForeground.end(), *row.logicalLine, {}, &StyledLineSegment::line);
    row.foregroundSpans.reserve(static_cast<std::size_t>(std::distance(first, last)));

    for (auto segment = first; segment != last; ++segment)
        row.foregroundSpans.push_back(segmentFromStyled(*segment));

    for (const auto& diagnostic : annotations.diagnostics()) {
        for (const auto& segment : document.clipToLines(diagnostic.range)) {
            if (segment.line == *row.logicalLine) {
                Segment viewSegment = segmentFromLine(segment, severityColor(diagnostic.severity), diagnostic.id);
                viewSegment.severity = diagnostic.severity;
                row.diagnostics.push_back(std::move(viewSegment));
            }
        }
    }

    for (const auto& decoration : annotations.decorations()) {
        for (const auto& segment : document.clipToLines(decoration.range)) {
            if (segment.line == *row.logicalLine)
                row.decorations.push_back(
                    segmentFromLine(segment, decoration.color, decoration.id, decoration.precedence));
        }
    }

    for (const auto& fold : annotations.foldRegions()) {
        if (!fold.collapsed && fold.range.start.line <= *row.logicalLine && fold.range.end.line >= *row.logicalLine)
            row.foldGutterGuide = true;
        if (fold.range.start.line == *row.logicalLine) {
            row.foldControl = FoldControl{fold.id, fold.collapsed, fold.placeholder};
            if (fold.collapsed)
                row.text = fold.placeholder;
        }
    }
}

void appendSelectionAndCaret(Row& row, const EditorSession* session, bool activePane, bool showCaret) {
    if (session == nullptr || !row.logicalLine)
        return;

    const Range selection = session->selection().normalized();
    if (!selection.empty() && selection.start.line <= *row.logicalLine && selection.end.line >= *row.logicalLine) {
        const std::size_t start = selection.start.line == *row.logicalLine ? selection.start.column : 0U;
        const std::size_t lineEnd = session->document().line(*row.logicalLine).size();
        const std::size_t end = selection.end.line == *row.logicalLine ? selection.end.column : lineEnd;
        if (end > start)
            row.selection =
                Selection{visualColumn(row, std::min(start, lineEnd)), visualColumn(row, std::min(end, lineEnd))};
    }

    const Position caret = session->caret();
    row.currentLine = activePane && caret.line == *row.logicalLine;
    row.activePane = activePane;
    if (showCaret && activePane && !session->readOnly() && session->mode() == Mode::Insert &&
        caret.line == *row.logicalLine)
        row.caret = Caret{visualColumn(row, std::min(caret.column, session->document().line(*row.logicalLine).size()))};
}

float decimalWidth(std::size_t lineCount, const Settings& settings) {
    std::size_t digits = 1;
    while (lineCount >= 10) {
        lineCount /= 10;
        ++digits;
    }

    return static_cast<float>(digits) * settings.glyphAdvance + settings.lineNumberPadding * 2.0F;
}

std::size_t Model::visualColumnCount(std::string_view text) noexcept {
    std::size_t visualColumn = 0U;
    for (std::size_t offset = 0; offset < text.size();) {
        if (text[offset] == '\t') {
            visualColumn += TabWidth - visualColumn % TabWidth;
            ++offset;
        } else {
            ++visualColumn;
            offset += codePointBytes(text, offset);
        }
    }

    return visualColumn;
}

Frame Model::buildFrame(const Document& document, const Projected& projection, const Annotation::Model& annotations,
                        const EditorSession* session, const Settings& settings, const Scroll& scroll,
                        float viewportHeight, bool activePane, bool showCaret,
                        const std::vector<StyledLineSegment>* preparedForeground) const {
    Frame frame;
    frame.layerOrder = LayerOrder;
    frame.lineNumberGutterWidth = settings.showLineNumbers ? decimalWidth(document.lineCount(), settings) : 0.0F;
    frame.lineNumberPadding = settings.showLineNumbers ? std::max(settings.lineNumberPadding, 0.0F) : 0.0F;
    frame.markerGutterWidth = settings.showMarkerGutter ? std::max(settings.markerGutterWidth, 0.0F) : 0.0F;
    const bool hasGutter = settings.showLineNumbers || settings.showMarkerGutter;
    frame.codeOriginX = frame.lineNumberGutterWidth + frame.markerGutterWidth +
                        (hasGutter ? settings.gutterSpacing : 0.0F) + settings.codePadding;
    frame.horizontalOffset = std::max(scroll.horizontalPixels, 0.0F);
    frame.lineHeight = std::max(settings.lineHeight, 1.0F);
    frame.glyphAdvance = std::max(settings.glyphAdvance, 1.0F);
    frame.fontWorldScale = std::max(settings.fontWorldScale, 0.01F);

    const float lineHeight = frame.lineHeight;
    const std::size_t rowCount = projection.rows().size();
    const float verticalRows = std::max(scroll.verticalRows, 0.0F);
    frame.verticalRows = verticalRows;
    const float textHeight = std::max(settings.fontAscent - settings.fontDescent, 0.0F);
    frame.baselineFromRowTop = (lineHeight - textHeight) * 0.5F + settings.fontAscent;
    const std::size_t top = static_cast<std::size_t>(std::floor(verticalRows));
    frame.firstVisibleRow = top > settings.overscanRows ? top - settings.overscanRows : 0;
    const std::size_t visible = static_cast<std::size_t>(std::ceil(std::max(viewportHeight, 0.0F) / lineHeight));
    const std::size_t wantedEnd = top + visible + settings.overscanRows;
    frame.endVisibleRow = std::min(rowCount, wantedEnd < top ? std::numeric_limits<std::size_t>::max() : wantedEnd);
    frame.rows.reserve(frame.endVisibleRow - frame.firstVisibleRow);

    // Resolve overlapping foreground ranges once for this frame. Resolving
    // them per visible row made large highlighted files quadratic and could
    // retain invalid iterators while documents were replaced.
    const std::vector<StyledLineSegment> resolved =
        preparedForeground == nullptr ? annotations.resolvedForeground(document) : std::vector<StyledLineSegment>{};
    const auto& resolvedForeground = preparedForeground == nullptr ? resolved : *preparedForeground;

    for (std::size_t visualRow = frame.firstVisibleRow; visualRow < frame.endVisibleRow; ++visualRow) {
        const Projected::Row& projectedRow = projection.rows()[visualRow];
        std::optional<std::size_t> logicalLine = projectedRow.logicalLine;
        if (logicalLine && *logicalLine >= document.lineCount())
            logicalLine.reset();

        Row row;
        row.visualRow = visualRow;
        row.logicalLine = logicalLine;
        row.diffKind = projectedRow.kind;
        row.gap = !logicalLine.has_value();

        if (logicalLine) {
            if (settings.showLineNumbers)
                row.lineNumber = std::to_string(*logicalLine + 1);
            RenderedLine rendered(document, *logicalLine, settings.showHiddenCharacters);
            row.text = std::move(rendered.text);
            row.sourceToVisualColumns = std::move(rendered.sourceToVisual);
            row.visualToSourceColumns = std::move(rendered.visualToSource);
            for (const auto& [start, end] : rendered.hiddenMarkers)
                row.hiddenCharacterMarkers.push_back({start, end});
            appendRowAnnotations(row, document, annotations, resolvedForeground);
            mapSegmentsToVisualColumns(row);
            appendSelectionAndCaret(row, session, activePane, showCaret);
        }

        frame.rows.push_back(std::move(row));
    }

    return frame;
}
} // namespace ScopeCanvas::Editor::Text::Render::Window
