#pragma once

namespace ff {
    enum class AccountCreationStatus {
        Success,
        Failure,
        UsernameExists,
        UsernameTooShort,
        UsernameTooLong,
        PasswordTooShort,
        PasswordTooLong,
        InvalidUsername,
        InvalidPassword,
        InvalidEmail,
        EmailExists,
    };
} // namespace ff
