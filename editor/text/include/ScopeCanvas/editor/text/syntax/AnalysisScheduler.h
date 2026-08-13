#pragma once

#include <ScopeCanvas/editor/text/syntax/Highlighter.h>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

namespace ScopeCanvas::Editor::Text::Syntax {
struct AnalysisResult {
    std::size_t revision{};
    Document syntax;
};

class AnalysisScheduler {
private:
    struct Request {
        std::size_t revision{};
        Text::Document document;
        std::shared_ptr<const Profile> profile;
    };

    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    std::optional<Request> m_pending;
    std::optional<AnalysisResult> m_result;
    std::exception_ptr m_failure;
    std::function<void()> m_notification;
    std::size_t m_latestRevision{};
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
    std::jthread m_worker;
#endif

public:
    explicit AnalysisScheduler(std::function<void()> notification = {}) {
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
        m_worker = std::jthread([this](std::stop_token stopToken) { run(stopToken); });
#endif
    }
    ~AnalysisScheduler() {
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
        m_worker.request_stop();
        m_condition.notify_all();
#endif
    }
    AnalysisScheduler(const AnalysisScheduler&) = delete;
    AnalysisScheduler& operator=(const AnalysisScheduler&) = delete;

    void schedule(std::size_t revision, Text::Document document, std::shared_ptr<const Profile> profile);
    void setNotification(std::function<void()> notification) {
        std::lock_guard lock(m_mutex);
        m_notification = std::move(notification);
    }

    [[nodiscard]] std::optional<AnalysisResult> takeLatest(std::size_t currentRevision);
    [[nodiscard]] std::exception_ptr takeFailure() {
        std::lock_guard lock(m_mutex);
        return std::exchange(m_failure, {});
    }

    void processPending();

private:
    void analyze(Request request);
#if !defined(__EMSCRIPTEN__) || defined(__EMSCRIPTEN_PTHREADS__)
    void run(std::stop_token stopToken);
#endif
};
} // namespace ScopeCanvas::Editor::Text::Syntax
