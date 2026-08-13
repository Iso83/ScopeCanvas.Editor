#pragma once

#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <ScopeCanvas/editor/text/annotation/Diagnostics.h>
#include <ScopeCanvas/editor/text/annotation/FontStyle.h>
#include <ScopeCanvas/editor/text/diff/Types.h>
#include <ScopeCanvas/editor/text/render/window/Folding.h>
#include <ScopeCanvas/editor/text/render/window/Selection.h>
#include <vector>

namespace ScopeCanvas::Editor::Text::Render::Window {
enum class Layer : std::uint8_t {
    RowBackground,
    CurrentLine,
    BackgroundAnnotation,
    Selection,
    Text,
    ForegroundAnnotation,
    Decoration,
    Diagnostic,
    Caret,
    FoldControl
};

struct Segment {
    std::size_t startColumn{};
    std::size_t endColumn{};
    Annotation::Color color{};
    Annotation::FontStyle style{Annotation::FontStyle::Regular};
    Annotation::DiagnosticSeverity severity{Annotation::DiagnosticSeverity::Hint};
    std::string id;
    int precedence{};
    bool operator==(const Segment&) const = default;
};

struct Row {
    std::size_t visualRow{};
    std::optional<std::size_t> logicalLine;
    Diff::Kind diffKind{Diff::Kind::Unchanged};
    std::string lineNumber;
    std::string text;
    std::vector<std::size_t> sourceToVisualColumns;
    std::vector<std::size_t> visualToSourceColumns;
    bool gap{};
    bool currentLine{};
    bool activePane{};
    std::optional<Selection> selection;
    std::optional<Caret> caret;
    std::optional<FoldControl> foldControl;
    bool foldGutterGuide{};
    std::vector<Segment> backgroundSpans;
    std::vector<Segment> foregroundSpans;
    std::vector<Segment> diagnostics;
    std::vector<Segment> decorations;
    std::vector<Segment> hiddenCharacterMarkers;
    bool operator==(const Row&) const = default;
};

struct Frame {
    std::size_t firstVisibleRow{};
    std::size_t endVisibleRow{};
    float lineNumberGutterWidth{};
    float lineNumberPadding{};
    float markerGutterWidth{};
    float codeOriginX{};
    float horizontalOffset{};
    float lineHeight{};
    float glyphAdvance{};
    float fontWorldScale{1.0F};
    float verticalRows{};
    float baselineFromRowTop{};
    std::vector<Row> rows;
    std::vector<Layer> layerOrder;
    bool operator==(const Frame&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
