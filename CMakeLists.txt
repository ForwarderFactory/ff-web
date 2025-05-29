cmake_minimum_required(VERSION 3.27)

project(ff-web VERSION 0.1.0)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(FF_ENABLE_SQLITE ON)
set(FF_ENABLE_POSTGRESQL ON)

set(PROJECT_SOURCE_FILES
    src/main.cpp
    src/ff.cpp
    src/config.cpp
    src/database.cpp
    src/upload_manager.cpp
    src/account_manager.cpp
    src/path_handlers.cpp
    src/scrypto.cpp
    src/av_io.cpp
    src/wadinfo.cpp
)

include_directories(include)
include_directories(libs)

add_subdirectory(libs/libbcrypt)

include_directories(libs/libbcrypt/include)
include_directories(libs/limhamn/include)
include_directories(libs/maddy/include)

find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED CONFIG COMPONENTS system url)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS Magick++)
find_package(PkgConfig REQUIRED)
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED)

add_compile_definitions(LIMHAMN_DATABASE_ICONV)

if (FF_ENABLE_SQLITE)
    add_compile_definitions(FF_ENABLE_SQLITE)
    add_compile_definitions(LIMHAMN_DATABASE_SQLITE3)
    find_package(SQLite3 REQUIRED)
endif()
if (FF_ENABLE_POSTGRESQL)
    add_compile_definitions(FF_ENABLE_POSTGRESQL)
    add_compile_definitions(LIMHAMN_DATABASE_POSTGRESQL)
    find_package(PostgreSQL REQUIRED)
endif()
if (!FF_ENABLE_SQLITE AND !FF_ENABLE_POSTGRESQL)
    message(FATAL_ERROR "No database backend enabled")
endif()

add_compile_definitions(FF_VERSION="${PROJECT_VERSION}")

if(e_debug)
    add_compile_definitions(FF_DEBUG)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::system
    yaml-cpp::yaml-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    bcrypt
    nlohmann_json::nlohmann_json
    ImageMagick::Magick++
    ${FFMPEG_LIBRARIES}
)

# some systems require explicit linking to iconv, others don't
# this is a bit hacky but seems to work well on both ubuntu and macos
find_library(ICONV_LIB iconv)
if (ICONV_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ICONV_LIB})
endif()
if (FF_ENABLE_SQLITE)
    target_link_libraries(${PROJECT_NAME} PRIVATE SQLite::SQLite3)
endif()
if (FF_ENABLE_POSTGRESQL)
    target_link_libraries(${PROJECT_NAME} PRIVATE PostgreSQL::PostgreSQL)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(CODE "
  message(STATUS \"Running fetch-sharpii.sh...\")
  execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/sh/fetch-sharpii.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE res
  )
  if(NOT res EQUAL 0)
    message(FATAL_ERROR \"fetch-sharpii.sh failed with exit code \${res}\")
  endif()
")