#include "watcher.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace srl {

FileWatcher::FileWatcher(std::string filePath, Callback callback, int pollIntervalMs)
    : filePath_(std::move(filePath)), callback_(std::move(callback)), pollIntervalMs_(pollIntervalMs) {
    if (std::filesystem::exists(filePath_)) {
        lastWriteTime_ = std::filesystem::last_write_time(filePath_);
    }
}

FileWatcher::~FileWatcher() {
    stop();
}

void FileWatcher::start() {
    if (running_) return;
    running_ = true;
    watchThread_ = std::thread(&FileWatcher::runLoop, this);
}

void FileWatcher::stop() {
    if (!running_) return;
    running_ = false;
    if (watchThread_.joinable()) {
        watchThread_.join();
    }
}

std::string FileWatcher::readFileContent() const {
    std::ifstream file(filePath_);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void FileWatcher::runLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs_));

        try {
            if (std::filesystem::exists(filePath_)) {
                auto currentWriteTime = std::filesystem::last_write_time(filePath_);
                if (currentWriteTime != lastWriteTime_) {
                    lastWriteTime_ = currentWriteTime;
                    std::string newContent = readFileContent();
                    if (!newContent.empty()) {
                        std::cout << "\n⚡ [Hot-Reload] File change detected in '" << filePath_ << "'. Patching live code..." << std::endl;
                        callback_(newContent);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[FileWatcher Error] " << e.what() << std::endl;
        }
    }
}

} // namespace srl
