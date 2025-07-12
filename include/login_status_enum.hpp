#pragma once

namespace ff {
    enum class LoginStatus {
        Success,
        Failure,
        Inactive,
        InvalidUsername,
        InvalidPassword,
        Banned,
    };
} // namespace ff
