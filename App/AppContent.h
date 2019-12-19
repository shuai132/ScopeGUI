#pragma once

#include "asio.hpp"

class AppContent : noncopyable {
public:
    asio::io_context& getUIContext() {
        return uiContext_;
    };

protected:
    asio::io_context uiContext_;
};
