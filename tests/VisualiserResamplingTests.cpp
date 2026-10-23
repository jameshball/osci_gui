// Standalone regression against the actual bundled resampler (no GUI/runtime).
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>
#include <xsimd/xsimd.hpp>

namespace juce {
template <typename T> T jmin(T a, T b) { return std::min(a, b); }
template <typename T> struct MathConstants { static constexpr T pi = static_cast<T>(3.14159265358979323846); };
}
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Type)
#include "../third_party/chowdsp_utils/modules/dsp/chowdsp_dsp_utils/Resampling/chowdsp_BaseResampler.h"
#include "../third_party/chowdsp_utils/modules/dsp/chowdsp_dsp_utils/Resampling/chowdsp_LanczosResampler.h"

using Resampler = chowdsp::ResamplingTypes::LanczosResampler<2048, 8>;
using Signal = std::array<std::vector<float>, 5>;

Signal process(const Signal& input, Resampler::Interpolation interpolation, const std::vector<std::size_t>& blocks, bool defaultMode = false, bool guarded = false, double ratio = 4) {
    std::array<Resampler, 5> resamplers;
    for (auto& item : resamplers) { item.prepare(48000, ratio); }
    Signal result;
    std::size_t position = 0, block = 0;
    while (position < input[0].size()) {
        const auto count = std::min(blocks[block++ % blocks.size()], input[0].size() - position);
        Signal output;
        std::array<Resampler::Channel, 5> channels;
        for (std::size_t ch = 0; ch < 5; ++ch) {
            output[ch].resize(count * static_cast<std::size_t>(std::ceil(ratio)) + (count + 1023) / 1024);
            channels[ch] = {&resamplers[ch], input[ch].data() + position, output[ch].data()};
        }
        const Resampler::ControlSignalGuard guard {2, 3};
        const auto size = defaultMode ? Resampler::processChannels(channels.data(), channels.size(), count)
            : Resampler::processChannels(channels.data(), channels.size(), count, interpolation, guarded ? &guard : nullptr);
        for (std::size_t ch = 0; ch < 5; ++ch) {
            result[ch].insert(result[ch].end(), output[ch].begin(), output[ch].begin() + size);
        }
        position += count;
    }
    return result;
}

int main() {
    Signal input;
    for (auto& channel : input) { channel.resize(4096); }
    for (std::size_t i = 0; i < input[0].size(); ++i) {
        input[0][i] = i <= 1023 ? -.8f : .8f;
        input[1][i] = i <= 1023 ? -.4f : .6f;
        for (std::size_t ch = 2; ch < 5; ++ch) { input[ch][i] = i == 1023 || i == 1024 ? 0 : 1; }
    }
    const auto linear = process(input, Resampler::Interpolation::linear, {4096});
    const auto split = process(input, Resampler::Interpolation::linear, {1, 7, 1023, 2, 511, 1024, 3});
    assert(linear == split);
    bool sawTravel = false;
    for (std::size_t i = 0; i < linear[0].size(); ++i) {
        // read() centres the sample at phaseO-1; four outputs per input.
        const auto position = static_cast<double>(i) / 4 - 1;
        for (std::size_t ch = 2; ch < 5; ++ch) { assert(linear[ch][i] >= 0 && linear[ch][i] <= 1); }
        if (position >= 1023 && position <= 1024) {
            sawTravel = true;
            assert(linear[2][i] == 0 && linear[3][i] == 0 && linear[4][i] == 0);
            const auto fraction = static_cast<float>(position - 1023);
            assert(std::abs(linear[0][i] - ((1 - fraction) * -.8f + fraction * .8f)) < 1e-6f);
        }
    }
    assert(sawTravel);
    const auto lanczos = process(input, Resampler::Interpolation::lanczos, {4096});
    const auto existing = process(input, Resampler::Interpolation::lanczos, {4096}, true);
    assert(lanczos == existing && lanczos[0].size() == linear[0].size());
    bool reproduced = false;
    for (std::size_t i = 0; i < lanczos[2].size(); ++i) {
        const auto position = static_cast<double>(i) / 4 - 1;
        if (position > 1023 && position < 1024 && lanczos[2][i] < -.1f) { reproduced = true; }
    }
    assert(reproduced); // Proves the original negative-red/inherited-colour defect.
    for (const double ratio : {4.0, 6.0}) {
        const auto adaptive = process(input, Resampler::Interpolation::lanczos, {4096}, false, true, ratio);
        const auto partitioned = process(input, Resampler::Interpolation::lanczos, {1, 7, 1023, 2, 511}, false, true, ratio);
        assert(adaptive[0].size() == partitioned[0].size());
        for (std::size_t i = 0; i < adaptive[0].size(); ++i) {
            for (std::size_t ch = 0; ch < 5; ++ch) { assert(std::abs(adaptive[ch][i] - partitioned[ch][i]) < 2e-5f); }
            const auto position = static_cast<double>(i) / ratio - 1;
            if (position > 1023.01 && position < 1023.99) {
                assert(adaptive[2][i] == 0 && adaptive[3][i] == 0 && adaptive[4][i] == 0);
            }
            for (std::size_t ch = 2; ch < 5; ++ch) { assert(adaptive[ch][i] >= 0); }
        }
    }

    // Mixed inherited/blank/explicit colour across both a host-block boundary
    // and the internal 1024-sample chunk boundary. Dark endpoints remain exact.
    for (std::size_t i = 0; i < input[0].size(); ++i) {
        for (std::size_t ch = 2; ch < 5; ++ch) {
            input[ch][i] = i < 1023 ? -1 : i <= 1024 ? 0 : static_cast<float>(ch - 1) * .2f;
        }
    }
    const auto mixed = process(input, Resampler::Interpolation::linear, {1024});
    assert(mixed == process(input, Resampler::Interpolation::linear, {17, 1, 2047, 31}));
    for (std::size_t i = 0; i < mixed[0].size(); ++i) {
        const auto position = static_cast<double>(i) / 4 - 1;
        if (position >= 1 && position <= 1022) {
            assert(mixed[2][i] == -1 && mixed[3][i] == -1 && mixed[4][i] == -1);
        }
        if (position >= 1023 && position <= 1024) {
            assert(mixed[2][i] == 0 && mixed[3][i] == 0 && mixed[4][i] == 0);
        }
        if (position >= 1025) {
            assert(mixed[2][i] == .2f && mixed[3][i] == .4f && mixed[4][i] == .6f);
        }
    }
    const auto inherited = process(input, Resampler::Interpolation::lanczos, {19, 1005}, false, true);
    const auto inheritedReference = process(input, Resampler::Interpolation::lanczos, {19, 1005});
    for (std::size_t i = 4096; i <= 4100; ++i) {
        assert(inherited[2][i] == 0 && inherited[3][i] == 0 && inherited[4][i] == 0);
        assert(inherited[0][i] == mixed[0][i] && inherited[1][i] == mixed[1][i]);
    }
    for (std::size_t i = 100; i < 3900; ++i) {
        for (std::size_t ch = 0; ch < 5; ++ch) { assert(inherited[ch][i] == inheritedReference[ch][i]); }
        assert(inherited[2][i] < 0);
    }
    // Explicit red can ring below zero even with no black vector at all:
    // constant green keeps this a continuous stroke, so only colour is clamped.
    for (std::size_t i = 0; i < input[0].size(); ++i) {
        input[0][i] = std::sin(static_cast<float>(i) * .12f);
        input[1][i] = std::cos(static_cast<float>(i) * .12f);
        input[2][i] = i == 1023 || i == 1024 ? 0 : 1;
        input[3][i] = 1; input[4][i] = .25f;
    }
    const auto colour = process(input, Resampler::Interpolation::lanczos, {4096}, false, true);
    const auto rawColour = process(input, Resampler::Interpolation::lanczos, {4096});
    for (std::size_t i = 100; i < colour[0].size(); ++i) {
        assert(colour[0][i] == rawColour[0][i] && colour[1][i] == rawColour[1][i]);
        assert(colour[2][i] >= 0);
    }
    std::cout << "Visualiser actual-resampler blanking, sentinel, phase and block-boundary tests passed\n";
}
