#pragma once

class AppContext {
public:
    virtual ~AppContext() = default;
    virtual void runOnUiThread(const std::function<void()>& task) = 0;
    virtual void post(const std::function<void()>& task) = 0;
};
