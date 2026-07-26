#ifndef SRL_WATCHER_HPP
#define SRL_WATCHER_HPP

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <filesystem>

namespace srl {

class FileWatcher {
public:
    using Callback = std::function<void(const std::string& newSource)>;

    FileWatcher(std::string filePath, Callback callback, int pollIntervalMs = 300);
    ~FileWatcher();

    void start();
    void stop();
    bool isRunning() const { return running_; }

private:
    std::string filePath_;
    Callback callback_;
    int pollIntervalMs_;
    std::atomic<bool> running_{false};
    std::thread watchThread_;
    std::filesystem::file_time_type lastWriteTime_;

    void runLoop();
    std::string readFileContent() const;
};

} // namespace srl

#endif // SRL_WATCHER_HPP
