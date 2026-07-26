#ifndef SRL_DSP_HPP
#define SRL_DSP_HPP

#include "vm.hpp"
#include "value.hpp"
#include <vector>
#include <complex>

namespace srl {

class DSP {
public:
    static void registerNativeFunctions(VM& vm);

    // Cooley-Tukey Radix-2 FFT
    static void fft(std::vector<double>& real, std::vector<double>& imag, bool inverse = false);

    // Helper utilities
    static std::vector<double> createSineWave(double freq, double sampleRate, int numSamples);
    static std::vector<double> createSquareWave(double freq, double sampleRate, int numSamples);
    static std::vector<double> createNoise(int numSamples);
    static std::vector<double> createHannWindow(int numSamples);
    static std::vector<double> createHammingWindow(int numSamples);
    static std::vector<double> applyLowPassFilter(const std::vector<double>& input, double cutoff, double sampleRate);
    static std::vector<double> computeMagnitude(const std::vector<double>& real, const std::vector<double>& imag);
};

} // namespace srl

#endif // SRL_DSP_HPP
