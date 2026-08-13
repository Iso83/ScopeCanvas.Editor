#pragma once

#include <ScopeCanvas/editor/text/Range.h>
#include <string>

namespace ScopeCanvas::Editor::Text::Annotation {
enum class DiagnosticSeverity { Hint, Warning, Error };

struct Diagnostic {
    Range range;
    DiagnosticSeverity severity;
    std::string message;
    std::string id;
};
} // namespace ScopeCanvas::Editor::Text::Annotation
