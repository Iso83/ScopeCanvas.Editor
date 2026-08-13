#pragma once

#include <ScopeCanvas/editor/text/Range.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Annotation {
struct FoldRegion {
    std::string id;
    Range range;
    bool collapsed{};
    std::string placeholder{"..."};
};
} // namespace ScopeCanvas::Editor::Text::Annotation
