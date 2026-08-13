#include <ScopeCanvas/editor/text/syntax/ColorProfile.h>

namespace ScopeCanvas::Editor::Text::Syntax {
ColorProfile ColorProfile::visualStudioDark() {
    ColorProfile profile;
    profile.set(TokenKind::Keyword, {86, 156, 214, 255});
    profile.set(TokenKind::TypeKeyword, {78, 201, 176, 255});
    profile.set(TokenKind::PreprocessorDirective, {197, 134, 192, 255});
    profile.set(TokenKind::Identifier, {220, 220, 220, 255});
    profile.set(TokenKind::NumericLiteral, {181, 206, 168, 255});
    profile.set(TokenKind::StringLiteral, {214, 157, 133, 255});
    profile.set(TokenKind::CharacterLiteral, {214, 157, 133, 255});
    profile.set(TokenKind::Comment, {106, 153, 85, 255});
    profile.set(TokenKind::Operator, {212, 212, 212, 255});
    profile.set(TokenKind::Punctuation, {212, 212, 212, 255});

    return profile;
}
} // namespace ScopeCanvas::Editor::Text::Syntax
