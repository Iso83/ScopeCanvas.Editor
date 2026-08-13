#pragma once

#include <ScopeCanvas/editor/text/syntax/Document.h>
#include <ScopeCanvas/editor/text/syntax/Profile.h>

namespace ScopeCanvas::Editor::Text::Syntax {
class Highlighter {
public:
    [[nodiscard]] static Document highlight(const Text::Document& document, const Profile& profile) {
        Document result;
        update(document, profile, result);
        return result;
    }

    static void update(const Text::Document& document, const Profile& profile, Document& syntax);
};
} // namespace ScopeCanvas::Editor::Text::Syntax
