cmake_minimum_required(VERSION 3.0.0)
project(rtsp_client VERSION 0.1.0)
add_compile_options(-std=c++17 -fpermissive)

set(OpenCV_DIR /usr/local/opencv-3.4.5/share/OpenCV)
include_directories(/usr/local/include/liveMedia)
include_directories(/usr/local/include/BasicUsageEnvironment)
include_directories(/usr/local/include/groupsock)
include_directories(/usr/local/include/UsageEnvironment)
include_directories(/usr/local/ffmpeg/include)
link_directories(/usr/local/ffmpeg/lib)

find_package(OpenCV REQUIRED)

aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} ${SRCS} Queue.h)
target_link_libraries(${PROJECT_NAME} liveMedia BasicUsageEnvironment groupsock UsageEnvironment) #必须是这个链接顺序
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swresample swscale pthread)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
