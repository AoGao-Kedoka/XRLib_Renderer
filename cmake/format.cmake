include(CheckCXXSourceCompiles)

set(CHECK_STD_FORMAT "
#include <format>
int main() {
auto text = std::format(\"{}\", 42);
return 0;
}
")

check_cxx_source_compiles("${CHECK_STD_FORMAT}" HAS_STD_FORMAT)

if (HAS_STD_FORMAT)
    message(STATUS "std::format is available.")
else()
    message(STATUS "std::format not available")
    find_package(fmt QUIET)
    if (NOT fmt_FOUND)
        message("${MESSAGE_BOX}\nFMT not found, try fetching...\n${MESSAGE_BOX}")
        include(FetchContent)
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG        10.1.1  # Use the latest stable version or specify the version you need
            )
        FetchContent_MakeAvailable(fmt)
    endif()
endif()
