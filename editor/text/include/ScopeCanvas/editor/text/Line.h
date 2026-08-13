#pragma once

#include <ScopeCanvas/editor/text/LineEnding.h>
#include <string>

namespace ScopeCanvas::Editor::Text {
struct Line {
    std::string content;
    LineEnding ending{LineEnding::None};
};
} // namespace ScopeCanvas::Editor::Text
