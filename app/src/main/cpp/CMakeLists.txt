cmake_minimum_required(VERSION 3.22.1)

project("myyolov5rtspthreadpool")

set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)  # ffmpeg的路径
# set(RTMP ${CMAKE_SOURCE_DIR}/rtmp) # rtmp的路径
set(RKNN ${CMAKE_SOURCE_DIR}/rknn)
set(OPENCV ${${CMAKE_SOURCE_DIR}/opencv})
set(MPP ${CMAKE_SOURCE_DIR}/mpp)
set(RGA ${CMAKE_SOURCE_DIR}/rga)

include_directories(${CMAKE_SOURCE_DIR}/include)  # 导入include文件夹
include_directories(${CMAKE_SOURCE_DIR}/task)
include_directories(${RKNN}/include)  # 导入rknn的头文件
include_directories(${FFMPEG}/include)  # 导入ffmpeg的头文件
include_directories(${OPENCV}/include)
include_directories(${MPP}/include)
include_directories(${RGA}/include)
include_directories(${CMAKE_SOURCE_DIR}/types)
include_directories(${CMAKE_SOURCE_DIR}/process)
include_directories(${CMAKE_SOURCE_DIR}/engine)
include_directories(${CMAKE_SOURCE_DIR}/draw)
include_directories(${CMAKE_SOURCE_DIR}/rkmedia)
include_directories(${CMAKE_SOURCE_DIR}/rkmedia/utils)

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv/jni)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs_export/${ANDROID_ABI})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs_export/${ANDROID_ABI})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs_export/${ANDROID_ABI}/binary)
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI}")

find_package(OpenCV 4.8 REQUIRED java)

# 记得修改路径
set(ZLMediaKit_Root "C:\\ZLMediaKit")
set(JNI_Root ${CMAKE_CURRENT_SOURCE_DIR})

# 添加主工程cmake
add_subdirectory(${ZLMediaKit_Root} ${EXECUTABLE_OUTPUT_PATH})

include_directories(${ZLMediaKit_Root}/src)
include_directories(${ZLMediaKit_Root}/srt)
include_directories(${ZLMediaKit_Root}/webrtc)
include_directories(${ZLMediaKit_Root}/server)
include_directories(${ZLMediaKit_Root}/3rdpart)
include_directories(${ZLMediaKit_Root}/3rdpart/media-server)
include_directories(${ZLMediaKit_Root}/3rdpart/ZLToolKit/src)
include_directories(${ZLMediaKit_Root}/3rdpart/ZLToolKit/api/include)

# find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
else (OpenCV_FOUND)
    message(FATAL_ERROR "Could not locate OpenCV")
endif (OpenCV_FOUND)

file(GLOB cpp_src_file *.cpp src/*.cpp)
# 查找所有的cpp源文件， 相当于把所有的cpp都纳入编译
file(GLOB JNI_src_list ${JNI_Root}/*.cpp ${JNI_Root}/*.h)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        ${cpp_src_file}
        task/yolov5.cpp
        task/yolov5_thread_pool.cpp
        engine/rknn_engine.cpp
        rkmedia/utils/mpp_decoder.cpp
        rkmedia/utils/drawing.cpp
        process/preprocess.cpp
        process/yolov5_postprocess.cpp
        )

target_link_libraries(${CMAKE_PROJECT_NAME}
        -Wl,--start-group android ${log-lib} z ${MK_LINK_LIBRARIES} -Wl,--end-group
        ${OpenCV_LIBS}
        android
        log
        ${MPP}/libs/${ANDROID_ABI}/libmpp.so
        ${RGA}/libs/${ANDROID_ABI}/librga.so
        ${RKNN}/libs/${ANDROID_ABI}/librknnrt.so

        mk_api # 在mpp_libs下，ZLMediaKit API库

        )