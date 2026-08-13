#pragma once

#include <ScopeCanvas/editor/text/annotation/Color.h>
#include <stdlib.h>

namespace ScopeCanvas::Editor::Text::Render::Window {
struct Settings {
    float lineHeight{20.0F};
    float glyphAdvance{9.0F};
    float lineNumberPadding{8.0F};
    float markerGutterWidth{22.0F};
    float codePadding{8.0F};
    std::size_t overscanRows{2};
    float gutterSpacing{0.0F};
    float foldControlWidth{14.0F};
    float fontWorldScale{1.0F};
    float fontAscent{15.0F};
    float fontDescent{-5.0F};
    bool showHiddenCharacters{};
    bool showLineNumbers{};
    bool showMarkerGutter{};
    bool showGutterBackground{};
};

struct ColorProfile {
    Annotation::Color unchanged{19, 21, 25, 255};
    Annotation::Color gap{27, 29, 33, 255};
    Annotation::Color added{31, 70, 46, 255};
    Annotation::Color removed{82, 39, 43, 255};
    Annotation::Color modified{101, 105, 0, 92};
    Annotation::Color changedTextCurrent{215, 204, 35, 81};
    Annotation::Color changedTextHistory{215, 204, 35, 140};
    Annotation::Color currentLine{32, 34, 41, 255};
    Annotation::Color selection{48, 87, 148, 255};
    Annotation::Color find{224, 0, 238, 100};
    Annotation::Color activeFind{224, 0, 238, 161};
    Annotation::Color hiddenBackground{45, 51, 62, 210};
    Annotation::Color hiddenBorder{105, 116, 137, 255};
    Annotation::Color lineNumber{140, 148, 163, 255};
    Annotation::Color text{220, 224, 235, 255};
    Annotation::Color foldGuide{97, 107, 128, 255};
    Annotation::Color foldControl{51, 56, 66, 255};
    Annotation::Color foldText{191, 199, 214, 255};
    Annotation::Color caret{235, 240, 250, 255};
    auto operator<=>(const ColorProfile&) const = default;
};
} // namespace ScopeCanvas::Editor::Text::Render::Window
