#include <ScopeCanvas/editor/text/syntax/AnalysisScheduler.h>
#include <utility>

namespace ScopeCanvas::Editor::Text::Syntax {
void AnalysisScheduler::schedule(std::size_t revision, Text::Document document,
                                 std::shared_ptr<const Profile> profile) {
    {
        std::lock_guard lock(m_mutex);
        m_latestRevision = std::max(m_latestRevision, revision);
        m_pending = Request{revision, std::move(document), std::move(profile)};
    }
    m_condition.notify_one();
}

std::optional<AnalysisResult> AnalysisScheduler::takeLatest(std::size_t currentRevision) {
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
    processPending();
#endif
    std::lock_guard lock(m_mutex);
    if (!m_result || m_result->revision != currentRevision)
        return {};
    auto result = std::move(m_result);
    m_result.reset();
    return result;
}

void AnalysisScheduler::processPending() {
    std::optional<Request> request;
    {
        std::lock_guard lock(m_mutex);
        request = std::move(m_pending);
        m_pending.reset();
    }
    if (request)
        analyze(std::move(*request));
}

void AnalysisScheduler::analyze(Request request) {
    try {
        AnalysisResult result{request.revision, Highlighter::highlight(request.document, *request.profile)};
        std::function<void()> notification;
        {
            std::lock_guard lock(m_mutex);
            if (request.revision == m_latestRevision)
                m_result = std::move(result);
            notification = m_notification;
        }
        if (notification)
            notification(); // Runs on the worker for native builds; it may only wake the owning event loop.
    } catch (...) {
        std::lock_guard lock(m_mutex);
        m_failure = std::current_exception();
    }
}

#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
void AnalysisScheduler::run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        std::optional<Request> request;
        {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [&] { return stopToken.stop_requested() || m_pending.has_value(); });
            if (stopToken.stop_requested())
                return;
            request = std::move(m_pending);
            m_pending.reset();
        }
        analyze(std::move(*request));
    }
}
#endif
} // namespace ScopeCanvas::Editor::Text::Syntax
