#include <iostream>
#include <array>
#include <cstdint>
#include <optional>
#include <chrono>
#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>

constexpr int64_t WORLD_SEED = -4172144997902289642LL;
constexpr int64_t chunkZMultInv248 = 211541297333629LL; // https://www.wolframalpha.com/input?i=132897987541%5E-1+mod+2%5E48

constexpr auto WOODLAND_BOUNDS = 23440 / 2;

constexpr auto RAND_MASK = (1LL << 48) - 1;

int64_t reverseWoodlandZGivenX(int64_t seed48Bits, int chunkX) {
    constexpr auto mask = (1LL << 48) - 1;
    return (((seed48Bits - WORLD_SEED - 10387319 - (int64_t) chunkX * 341873128712LL) & mask) * chunkZMultInv248) << 16 >> 16;
}

int64_t woodlandMansionSeed(int chunkX, int chunkZ) { // from World and WoodlandMansion
    return ((int64_t) chunkX * 341873128712LL + (int64_t) chunkZ * 132897987541LL + WORLD_SEED + (int64_t) 10387319) & RAND_MASK; // rand drops highest 16 bits anyway
}

int64_t nextSeed(int64_t seed) {
    return (seed * 0x5DEECE66D + 0xB) & RAND_MASK;
}

std::optional<std::pair<int, int>> woodlandValid(int64_t candidate) {
    for (int x = -WOODLAND_BOUNDS; x <= WOODLAND_BOUNDS; x++) {
        long z = reverseWoodlandZGivenX(candidate, x);
        if (z >= -WOODLAND_BOUNDS && z <= WOODLAND_BOUNDS) {
            return {{x, z}};
        }
    }
    return {};
}

struct SmallSeed {
    int64_t bits:48;

    explicit SmallSeed(int64_t seed): bits(seed) {}

    [[nodiscard]] int64_t toInt() const {
        return bits;
    }

    template<typename H>
    friend H AbslHashValue(H h, SmallSeed seed) {
        return H::combine(std::move(h), seed.toInt());
    }
} __attribute__((packed));

bool operator==(SmallSeed lhs, SmallSeed rhs) {
    return lhs.bits == rhs.bits;
}

struct SmallInt {
    int bits:24;

    explicit SmallInt(int x): bits(x) {}
    explicit SmallInt() = default;

    [[nodiscard]] int toInt() const {
        return bits;
    }
} __attribute__((packed));


struct Pos {
    SmallInt x{};
    SmallInt z{};
};


int main() {
    auto bigMap = absl::flat_hash_map<SmallSeed, Pos>{};
    constexpr auto neededCapacity = (WOODLAND_BOUNDS * 2) * (WOODLAND_BOUNDS * 2);
    bigMap.reserve(neededCapacity + 1000);
    std::cout << woodlandMansionSeed(-69, 420) << std::endl;

    for (int x = -WOODLAND_BOUNDS; x <= WOODLAND_BOUNDS; x++) {
        for (int z = -WOODLAND_BOUNDS; z <= WOODLAND_BOUNDS; z++) {
            bigMap[SmallSeed{woodlandMansionSeed(x, z)}] = Pos{SmallInt{x}, SmallInt{z}};
        }
    }
    std::cout << "bigMap is " << bigMap.size() << " elements large" << std::endl;

    int64_t currentStepIdx = 0;
    int64_t stepIdxOfPreviousWoodlandSeed = -1;
    int previousWoodlandX = -10000000;
    int previousWoodlandZ = -10000000;
    int64_t stopAfter = -1;
    int64_t seed = 0;
    auto prev_time = std::chrono::steady_clock::now();
    for (;;) {
        auto it = bigMap.find(SmallSeed{seed});
        if (it != bigMap.end()) {
            const bool lastRun = seed == stopAfter;
            const auto stepsSincePrev = currentStepIdx - stepIdxOfPreviousWoodlandSeed;
            stepIdxOfPreviousWoodlandSeed = currentStepIdx;
            const auto myX = it->second.x.toInt();
            const auto myZ = it->second.z.toInt();
            if (stopAfter == -1) {
                stopAfter = seed;
            } else {
                //std::cout << "Woodland seed " << seed << " decodes to " << myX << ", " << myZ << " and the previous woodland was " << stepsSincePrev << " steps ago and it was " << previousWoodlandX << ", " << previousWoodlandZ << std::endl;
                //constexpr auto end = (1LL << 48);
                //const auto percent = ((double)currentStepIdx / (double)end) * 100;
                //fmt::print("At step {} out of {} ({:.2f}%)\n", currentStepIdx, end, percent);

                // write this to the file i guess
            }

            previousWoodlandX = myX;
            previousWoodlandZ = myZ;
            if (lastRun) {
                break;
            }
        }
        if (currentStepIdx % 100'000'000 == 0) {
            auto time = std::chrono::steady_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time - prev_time).count();
            prev_time = time;
            constexpr auto end = (1LL << 48);
            const auto percent = ((double)currentStepIdx / (double)end) * 100;
            fmt::print("At step {} out of {} ({:.4f}%)\n", currentStepIdx, end, percent);
            fmt::print("100,000,000 steps took {} ms ({} s)\n", millis, millis / 1000);
        }
        seed = nextSeed(seed);
        currentStepIdx++;
    }

}
