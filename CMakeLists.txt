cmake_minimum_required(VERSION 3.0.0)
project(rtsp_client VERSION 0.1.0)
add_compile_options(-std=c++17)

include_directories(/usr/local/include/liveMedia)
include_directories(/usr/local/include/BasicUsageEnvironment)
include_directories(/usr/local/include/groupsock)
include_directories(/usr/local/include/UsageEnvironment)

aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} liveMedia BasicUsageEnvironment groupsock UsageEnvironment) #必须是这个链接顺序
