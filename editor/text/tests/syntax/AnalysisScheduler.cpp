#include "TestAssert.h"

#include <ScopeCanvas/editor/text/syntax/AnalysisScheduler.h>
#include <ScopeCanvas/editor/text/syntax/profiles/Cpp.h>
#include <chrono>
#include <thread>

using namespace ScopeCanvas::Editor::Text::Syntax;
using namespace ScopeCanvas::Editor::Text::Syntax::Profiles;
using namespace std::chrono_literals;
using txtDoc = ScopeCanvas::Editor::Text::Document;

class ThrowingProfile final : public Profile {
public:
    std::vector<Token> tokenizeLine(std::string_view, std::size_t, LexerState&) const override {
        throw std::runtime_error("test");
    }
};

int test_coalesces_and_only_publishes_current_revision() {
    AnalysisScheduler scheduler;
    auto profile = std::make_shared<CppProfile>();
    scheduler.schedule(1, txtDoc("int old;"), profile);
    scheduler.schedule(2, txtDoc("int newest;"), profile);
    for (int attempt = 0; attempt < 100 && !scheduler.takeLatest(2); ++attempt)
        std::this_thread::sleep_for(2ms);
    scheduler.schedule(3, txtDoc("int final;"), profile);
    std::optional<AnalysisResult> result;
    for (int attempt = 0; attempt < 100 && !result; ++attempt) {
        result = scheduler.takeLatest(3);
        std::this_thread::sleep_for(2ms);
    }
    CPPTEST_ASSERT(result.has_value());
    CPPTEST_ASSERT(result->revision == 3U);
    return 0;
}

int test_contains_worker_exception_and_destroys_with_work() {
    {
        AnalysisScheduler scheduler;
        scheduler.schedule(1, txtDoc("boom"), std::make_shared<ThrowingProfile>());
        for (int attempt = 0; attempt < 100 && !scheduler.takeFailure(); ++attempt)
            std::this_thread::sleep_for(2ms);
        scheduler.schedule(2, txtDoc(std::string(10000, 'x')), std::make_shared<CppProfile>());
    }
    return 0;
}

int main() {
    CPPTEST_RUN(test_coalesces_and_only_publishes_current_revision);
    CPPTEST_RUN(test_contains_worker_exception_and_destroys_with_work);
    return 0;
}
