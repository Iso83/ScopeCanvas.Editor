#include "TestAssert.h"

#include <ScopeCanvas/editor/text/RendererCache.h>

using namespace ScopeCanvas::Editor::Text;

int test_buckets_requests() {
    RendererCache cache(4U, 2U);
    CPPTEST_ASSERT(cache.bucketFor(17.0F).pixelSize == 16U);
    CPPTEST_ASSERT(cache.bucketFor(17.0F).worldScale > 1.0F);
    CPPTEST_ASSERT(cache.bucketFor(28.0F).pixelSize == 28U);
    CPPTEST_ASSERT(cache.size() == 0U);
    cache.clear();
    CPPTEST_ASSERT(cache.size() == 0U);
    return 0;
}

int main() {
    CPPTEST_RUN(test_buckets_requests);
    return 0;
}
