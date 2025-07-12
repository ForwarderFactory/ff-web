#pragma once

namespace ff {
    enum class UploadStatus {
        Success,
        Failure,
        InvalidCreds,
        NoFile,
        TooLarge,
    };
} // namespace ff
