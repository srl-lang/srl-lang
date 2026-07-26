#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "dsp.hpp"
#include "tui.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace srl {

static const double PI = 3.14159265358979323846;

static size_t nextPowerOfTwo(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

void DSP::fft(std::vector<double>& real, std::vector<double>& imag, bool inverse) {
    size_t n = real.size();
    if (n == 0) return;

    size_t targetN = nextPowerOfTwo(n);
    if (targetN != n) {
        real.resize(targetN, 0.0);
        imag.resize(targetN, 0.0);
        n = targetN;
    }

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;

        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // Cooley-Tukey butterfly algorithm
    for (size_t len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * PI / len * (inverse ? 1.0 : -1.0);
        double wlen_r = std::cos(angle);
        double wlen_i = std::sin(angle);

        for (size_t i = 0; i < n; i += len) {
            double w_r = 1.0;
            double w_i = 0.0;

            for (size_t j = 0; j < len / 2; ++j) {
                size_t u = i + j;
                size_t v = i + j + len / 2;

                double vr_w_r = real[v] * w_r - imag[v] * w_i;
                double vr_w_i = real[v] * w_i + imag[v] * w_r;

                real[v] = real[u] - vr_w_r;
                imag[v] = imag[u] - vr_w_i;
                real[u] += vr_w_r;
                imag[u] += vr_w_i;

                double next_w_r = w_r * wlen_r - w_i * wlen_i;
                double next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }

    if (inverse) {
        for (size_t i = 0; i < n; ++i) {
            real[i] /= n;
            imag[i] /= n;
        }
    }
}

std::vector<double> DSP::createSineWave(double freq, double sampleRate, int numSamples) {
    std::vector<double> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        samples[i] = std::sin(2.0 * PI * freq * t);
    }
    return samples;
}

std::vector<double> DSP::createSquareWave(double freq, double sampleRate, int numSamples) {
    std::vector<double> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double s = std::sin(2.0 * PI * freq * t);
        samples[i] = (s >= 0.0) ? 1.0 : -1.0;
    }
    return samples;
}

std::vector<double> DSP::createNoise(int numSamples) {
    static std::mt19937 gen(1337);
    static std::uniform_real_distribution<double> dis(-1.0, 1.0);
    std::vector<double> samples(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        samples[i] = dis(gen);
    }
    return samples;
}

std::vector<double> DSP::createHannWindow(int numSamples) {
    std::vector<double> window(numSamples);
    if (numSamples <= 1) {
        if (numSamples == 1) window[0] = 1.0;
        return window;
    }
    for (int i = 0; i < numSamples; ++i) {
        window[i] = 0.5 * (1.0 - std::cos(2.0 * PI * i / (numSamples - 1)));
    }
    return window;
}

std::vector<double> DSP::createHammingWindow(int numSamples) {
    std::vector<double> window(numSamples);
    if (numSamples <= 1) {
        if (numSamples == 1) window[0] = 1.0;
        return window;
    }
    for (int i = 0; i < numSamples; ++i) {
        window[i] = 0.54 - 0.46 * std::cos(2.0 * PI * i / (numSamples - 1));
    }
    return window;
}

std::vector<double> DSP::applyLowPassFilter(const std::vector<double>& input, double cutoff, double sampleRate) {
    std::vector<double> output(input.size());
    if (input.empty()) return output;

    double dt = 1.0 / sampleRate;
    double RC = 1.0 / (2.0 * PI * cutoff);
    double alpha = dt / (RC + dt);

    output[0] = input[0];
    for (size_t i = 1; i < input.size(); ++i) {
        output[i] = output[i - 1] + alpha * (input[i] - output[i - 1]);
    }
    return output;
}

std::vector<double> DSP::computeMagnitude(const std::vector<double>& real, const std::vector<double>& imag) {
    size_t n = (std::min)(real.size(), imag.size());
    std::vector<double> mag(n);
    for (size_t i = 0; i < n; ++i) {
        mag[i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
    }
    return mag;
}

void DSP::registerNativeFunctions(VM& vm) {
    // dsp_sine(freq, sampleRate, numSamples)
    vm.defineNative("dsp_sine", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber()) {
            auto vec = createSineWave(args[0].asNumber(), args[1].asNumber(), static_cast<int>(args[2].asNumber()));
            auto arr = std::make_shared<std::vector<Value>>();
            arr->reserve(vec.size());
            for (double val : vec) arr->push_back(Value(val));
            return Value(arr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_square(freq, sampleRate, numSamples)
    vm.defineNative("dsp_square", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber()) {
            auto vec = createSquareWave(args[0].asNumber(), args[1].asNumber(), static_cast<int>(args[2].asNumber()));
            auto arr = std::make_shared<std::vector<Value>>();
            arr->reserve(vec.size());
            for (double val : vec) arr->push_back(Value(val));
            return Value(arr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_noise(numSamples)
    vm.defineNative("dsp_noise", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isNumber()) {
            auto vec = createNoise(static_cast<int>(args[0].asNumber()));
            auto arr = std::make_shared<std::vector<Value>>();
            arr->reserve(vec.size());
            for (double val : vec) arr->push_back(Value(val));
            return Value(arr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_hann(numSamples)
    vm.defineNative("dsp_hann", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isNumber()) {
            auto vec = createHannWindow(static_cast<int>(args[0].asNumber()));
            auto arr = std::make_shared<std::vector<Value>>();
            arr->reserve(vec.size());
            for (double val : vec) arr->push_back(Value(val));
            return Value(arr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_hamming(numSamples)
    vm.defineNative("dsp_hamming", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isNumber()) {
            auto vec = createHammingWindow(static_cast<int>(args[0].asNumber()));
            auto arr = std::make_shared<std::vector<Value>>();
            arr->reserve(vec.size());
            for (double val : vec) arr->push_back(Value(val));
            return Value(arr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_lowpass(signalArray, cutoff, sampleRate)
    vm.defineNative("dsp_lowpass", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isArray() && args[1].isNumber() && args[2].isNumber()) {
            auto inArr = args[0].asArray();
            std::vector<double> input;
            input.reserve(inArr->size());
            for (const auto& v : *inArr) {
                input.push_back(v.isNumber() ? v.asNumber() : 0.0);
            }
            auto filtered = applyLowPassFilter(input, args[1].asNumber(), args[2].asNumber());
            auto outArr = std::make_shared<std::vector<Value>>();
            outArr->reserve(filtered.size());
            for (double val : filtered) outArr->push_back(Value(val));
            return Value(outArr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_fft(realArray, [imagArray]) -> returns Map { "real": arr, "imag": arr }
    vm.defineNative("dsp_fft", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isArray()) {
            auto rArr = args[0].asArray();
            std::vector<double> realVec, imagVec;
            realVec.reserve(rArr->size());
            for (const auto& v : *rArr) {
                realVec.push_back(v.isNumber() ? v.asNumber() : 0.0);
            }
            if (argCount >= 2 && args[1].isArray()) {
                auto iArr = args[1].asArray();
                imagVec.reserve(iArr->size());
                for (const auto& v : *iArr) {
                    imagVec.push_back(v.isNumber() ? v.asNumber() : 0.0);
                }
            } else {
                imagVec.resize(realVec.size(), 0.0);
            }

            fft(realVec, imagVec, false);

            auto resReal = std::make_shared<std::vector<Value>>();
            auto resImag = std::make_shared<std::vector<Value>>();
            resReal->reserve(realVec.size());
            resImag->reserve(imagVec.size());

            for (double r : realVec) resReal->push_back(Value(r));
            for (double i : imagVec) resImag->push_back(Value(i));

            auto resultMap = std::make_shared<std::unordered_map<std::string, Value>>();
            (*resultMap)["real"] = Value(resReal);
            (*resultMap)["imag"] = Value(resImag);
            return Value(resultMap);
        }
        return Value(std::make_shared<std::unordered_map<std::string, Value>>());
    });

    // dsp_ifft(realArray, imagArray) -> returns ArrayPtr
    vm.defineNative("dsp_ifft", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray() && args[1].isArray()) {
            auto rArr = args[0].asArray();
            auto iArr = args[1].asArray();
            std::vector<double> realVec, imagVec;
            realVec.reserve(rArr->size());
            imagVec.reserve(iArr->size());

            for (const auto& v : *rArr) realVec.push_back(v.isNumber() ? v.asNumber() : 0.0);
            for (const auto& v : *iArr) imagVec.push_back(v.isNumber() ? v.asNumber() : 0.0);

            fft(realVec, imagVec, true);

            auto outArr = std::make_shared<std::vector<Value>>();
            outArr->reserve(realVec.size());
            for (double r : realVec) outArr->push_back(Value(r));
            return Value(outArr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_magnitude(realArray, imagArray) -> returns ArrayPtr
    vm.defineNative("dsp_magnitude", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray() && args[1].isArray()) {
            auto rArr = args[0].asArray();
            auto iArr = args[1].asArray();
            std::vector<double> realVec, imagVec;
            realVec.reserve(rArr->size());
            imagVec.reserve(iArr->size());

            for (const auto& v : *rArr) realVec.push_back(v.isNumber() ? v.asNumber() : 0.0);
            for (const auto& v : *iArr) imagVec.push_back(v.isNumber() ? v.asNumber() : 0.0);

            auto mag = computeMagnitude(realVec, imagVec);

            auto outArr = std::make_shared<std::vector<Value>>();
            outArr->reserve(mag.size());
            for (double m : mag) outArr->push_back(Value(m));
            return Value(outArr);
        }
        return Value(std::make_shared<std::vector<Value>>());
    });

    // dsp_plot(signalArray, height, title) -> ASCII waveform / spectrum renderer
    vm.defineNative("dsp_plot", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isArray()) {
            auto arr = args[0].asArray();
            int height = (argCount >= 2 && args[1].isNumber()) ? static_cast<int>(args[1].asNumber()) : 8;
            std::string title = (argCount >= 3 && args[2].isString()) ? args[2].asString() : "Signal Plot";

            if (arr->empty()) return Value();

            double minVal = arr->at(0).isNumber() ? arr->at(0).asNumber() : 0.0;
            double maxVal = minVal;
            for (const auto& v : *arr) {
                if (v.isNumber()) {
                    double num = v.asNumber();
                    if (num < minVal) minVal = num;
                    if (num > maxVal) maxVal = num;
                }
            }

            if (std::abs(maxVal - minVal) < 1e-9) {
                maxVal = minVal + 1.0;
            }

            std::cout << "\n=== [DSP Plot: " << title << "] (Min: " << std::fixed << std::setprecision(2) << minVal 
                      << ", Max: " << maxVal << ") ===" << std::endl;

            int width = (std::min)(static_cast<int>(arr->size()), 64);
            int step = (std::max)(1, static_cast<int>(arr->size()) / width);

            for (int r = height - 1; r >= 0; --r) {
                double rowVal = minVal + (maxVal - minVal) * (static_cast<double>(r) / (height - 1));
                std::cout << std::setw(6) << std::fixed << std::setprecision(1) << rowVal << " | ";
                for (int c = 0; c < width; ++c) {
                    int idx = c * step;
                    double val = arr->at(idx).isNumber() ? arr->at(idx).asNumber() : 0.0;
                    int valRow = static_cast<int>((val - minVal) / (maxVal - minVal) * (height - 1));
                    if (valRow == r) {
                        std::cout << "*";
                    } else if (valRow > r && minVal < 0 && r == static_cast<int>(-minVal / (maxVal - minVal) * (height - 1))) {
                        std::cout << "-";
                    } else {
                        std::cout << " ";
                    }
                }
                std::cout << "\n";
            }
            std::cout << "       +" << std::string(width, '-') << "\n" << std::endl;
        }
        return Value();
    });
}

} // namespace srl
