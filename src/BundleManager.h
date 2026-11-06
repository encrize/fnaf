#pragma once
#include <string>

namespace FNAF {

// Log message to file and console
void LogDebug(const std::string& msg);

// Ensures all necessary game assets and configs are accessible.
// If running next to an existing assets folder, uses it directly.
// If running as a standalone single-file executable, automatically
// unpacks the embedded assets bundle into the user's local cache on first run.
bool EnsureAssetsReady();

} // namespace FNAF
