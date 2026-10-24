#pragma once
#include <filesystem>
#include <string_view>

// Writes the source, AST (or parse diagnostic), and symbol-resolution output.
// Returns true only when parsing and name resolution succeed.
bool write_debug_output(std::string_view title, std::string_view file_name,
                        std::string_view source,
                        const std::filesystem::path &out_dir);
