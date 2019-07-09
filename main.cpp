#include <iostream>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "MultiRTSPClient.h"
unsigned rtspClientCount = 0;
volatile char eventLoopWatchVariable = 0;

int main(int, char**) {
    std::string url = "rtsp://192.168.2.253:8554/test.h264";
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    MultiRTSPClient client(*env,url.data(),1,"rtsp_client");
    rtspClientCount++;
    client.sendDescribeCommand(continueAfterDESCRIBE);
    env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
    env->reclaim(); env = NULL;
    delete scheduler; scheduler = NULL;
}
