set(ASSIMP_DEPS assimp::assimp)
find_package(assimp CONFIG QUIET)
if (NOT assimp_FOUND)
	message("${MESSAGE_BOX}\nAssimp not found, try fetching\n${MESSAGE_BOX}")
	FetchContent_Declare(assimp
		GIT_REPOSITORY https://github.com/assimp/assimp.git
		GIT_TAG v5.4.3)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
	set(ASSIMP_INSTALL ON)
	FetchContent_MakeAvailable(assimp)
	set(ASSIMP_DEPS assimp)
endif()
