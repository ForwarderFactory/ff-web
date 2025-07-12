#pragma once

namespace ff {
    enum class ProfileUpdateStatus {
        Success,
        Failure,
        InvalidCreds,
        InvalidJson,
        InvalidIcon,
    };
} // namespace ff
