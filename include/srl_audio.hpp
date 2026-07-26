#ifndef SRL_AUDIO_HPP
#define SRL_AUDIO_HPP

#include <string>

namespace srl {
class VM;

class Audio {
public:
    static void registerNativeFunctions(VM& vm);

    // Audio Playback API
    static bool play(const std::string& filepath);
    static bool pause();
    static bool resume();
    static bool stop();
    static bool setVolume(int volumePercent); // 0 to 100
    static int getVolume();
    static double getPosition(); // in seconds
    static double getLength();   // in seconds
    static bool seek(double seconds);
    static bool isPlaying();
    static void beep(int frequency, int durationMs);
};

} // namespace srl

#endif // SRL_AUDIO_HPP
