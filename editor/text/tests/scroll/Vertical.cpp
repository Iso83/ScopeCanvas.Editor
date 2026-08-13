#include "TestAssert.h"

#include <ScopeCanvas/editor/text/scroll/Vertical.h>

using namespace ScopeCanvas::Editor::Text::Scroll;

int test_synchronizes_clamps_and_handles_short_layouts() {
    Vertical shared{100.0F, 20.0F, 0.0F};
    shared.setNormalizedVerticalPosition(0.5F);
    CPPTEST_ASSERT(std::abs(shared.verticalRows - 40.0F) < 0.001F);
    const float observedByLeft = shared.verticalRows;
    const float observedByRight = shared.verticalRows;
    CPPTEST_ASSERT(observedByLeft == observedByRight);

    shared.setNormalizedVerticalPosition(0.75F);
    CPPTEST_ASSERT(std::abs(shared.verticalRows - 60.0F) < 0.001F);
    shared.scrollRows(-3.0F);
    CPPTEST_ASSERT(std::abs(shared.verticalRows - 57.0F) < 0.001F);
    shared.scrollRows(20.0F);
    CPPTEST_ASSERT(std::abs(shared.verticalRows - 77.0F) < 0.001F);

    shared.visibleRows = 30.0F;
    shared.clamp();
    CPPTEST_ASSERT(std::abs(shared.maximumVerticalRows() - 70.0F) < 0.001F);
    CPPTEST_ASSERT(std::abs(shared.verticalRows - 70.0F) < 0.001F);
    shared.totalRows = 10.0F;
    shared.clamp();
    CPPTEST_ASSERT(shared.maximumVerticalRows() == 0.0F);
    CPPTEST_ASSERT(shared.verticalRows == 0.0F);
    CPPTEST_ASSERT(shared.normalizedVerticalPosition() == 0.0F);
    return 0;
}

int main() {
    CPPTEST_RUN(test_synchronizes_clamps_and_handles_short_layouts);
    return 0;
}
