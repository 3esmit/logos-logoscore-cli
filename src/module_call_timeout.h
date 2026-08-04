#ifndef MODULE_CALL_TIMEOUT_H
#define MODULE_CALL_TIMEOUT_H

namespace logoscore {

// Module calls can perform bounded cold initialization or submit a transaction.
// Keep both proxy legs above the SDK's short metadata-call default, but finite
// so a hung module remains observable to callers.
inline constexpr int kModuleCallTimeoutMs = 60'000;

} // namespace logoscore

#endif // MODULE_CALL_TIMEOUT_H
