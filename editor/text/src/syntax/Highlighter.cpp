#include <ScopeCanvas/editor/text/syntax/Highlighter.h>

namespace ScopeCanvas::Editor::Text::Syntax {
void Highlighter::update(const Text::Document& document, const Profile& profile, Document& syntax) {
    const std::size_t lineCount = document.lineCount();
    if (syntax.lines.size() != lineCount || syntax.sourceLines.size() != lineCount ||
        syntax.endStates.size() != lineCount) {
        syntax.lines.clear();
        syntax.sourceLines.clear();
        syntax.endStates.clear();
        syntax.lines.reserve(lineCount);
        syntax.sourceLines.reserve(lineCount);
        syntax.endStates.reserve(lineCount);

        LexerState state;
        for (std::size_t line = 0; line < lineCount; ++line) {
            syntax.sourceLines.emplace_back(document.line(line));
            syntax.lines.push_back({line, profile.tokenizeLine(document.line(line), line, state)});
            syntax.endStates.push_back(state);
        }

        return;
    }

    std::size_t firstChanged = 0;
    while (firstChanged < lineCount && syntax.sourceLines[firstChanged] == document.line(firstChanged))
        ++firstChanged;

    if (firstChanged == lineCount)
        return;

    std::size_t lastChanged = lineCount - 1U;
    while (lastChanged > firstChanged && syntax.sourceLines[lastChanged] == document.line(lastChanged))
        --lastChanged;

    LexerState state = firstChanged == 0 ? LexerState{} : syntax.endStates[firstChanged - 1U];
    for (std::size_t line = firstChanged; line < lineCount; ++line) {
        const LexerState previousEndState = syntax.endStates[line];
        syntax.sourceLines[line] = document.line(line);
        syntax.lines[line] = {line, profile.tokenizeLine(document.line(line), line, state)};
        syntax.endStates[line] = state;

        if (line >= lastChanged && state == previousEndState)
            break;
    }
}
} // namespace ScopeCanvas::Editor::Text::Syntax
