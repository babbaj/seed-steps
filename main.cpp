#include <iostream>
#include <array>
#include <cstdint>
#include <optional>
#include <chrono>
#include <thread>
#include <vector>
#include <filesystem>
#include <span>
#include <fstream>

#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>

constexpr int64_t WORLD_SEED = -4172144997902289642LL;
constexpr int64_t chunkZMultInv248 = 211541297333629LL; // https://www.wolframalpha.com/input?i=132897987541%5E-1+mod+2%5E48

constexpr int64_t WOODLAND_BOUNDS = 23440;
constexpr int64_t TOTAL_MANSIONS = (WOODLAND_BOUNDS * 2 + 1) * (WOODLAND_BOUNDS * 2 + 1);
constexpr int64_t MANSIONS_IN_ROW = (WOODLAND_BOUNDS * 2 + 1);


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
    //int64_t bits;

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

/*namespace std {
    template <> struct hash<SmallSeed> {
        size_t operator()(SmallSeed x) const {
            return x.bits;
        }
    };
}*/

struct SmallInt {
    int bits:16;
    //int bits;

    SmallInt(int x): bits(x) {}
    explicit SmallInt() = default;

    [[nodiscard]] int toInt() const {
        return bits;
    }
} __attribute__((packed));


struct Pos {
    SmallInt x{};
    SmallInt z{};
};

std::pair<int, Pos> stepsForSeed(int64_t seed, const absl::flat_hash_map<SmallSeed, Pos>& map) {
    int i = 1;
    while (true) {
        seed = nextSeed(seed);
        auto it = map.find(SmallSeed{seed});
        if (it != map.end()) {
            return {i, it->second};
        }
        i++;
    }
}

struct Result {
    Pos start;
    Pos end;
    int32_t steps;
} __attribute__((packed));

constexpr uint64_t EXPECTED_FILE_SIZE = MANSIONS_IN_ROW * sizeof(Result);

void serializeSmallInt(SmallInt x, char* out) {
    auto i = x.bits;
    out[1] = (char) (i & 0xFF);
    out[0] = (char) (i >> 8 & 0xFF);
}
void serializeSmallSeed(SmallSeed x, char* out) {
    auto i = x.bits;
    out[0] = (char) (i & 0xFF);
    out[1] = (char) (i >> 8 & 0xFF);
    out[2] = (char) (i >> 16 & 0xFF);
    out[3] = (char) (i >> 24 & 0xFF);
    out[4] = (char) (i >> 32 & 0xFF);
    out[5] = (char) (i >> 40 & 0xFF);
}
void serializeInt(int32_t x, char* out) {
    out[3] = (char) (x & 0xFF);
    out[2] = (char) (x >> 8 & 0xFF);
    out[1] = (char) (x >> 16 & 0xFF);
    out[0] = (char) (x >> 24 & 0xFF);
}

std::array<char, sizeof(Result)> serializeResult(const Result& result) {
    std::array<char, sizeof(Result)> out{};
    char* ptr = out.data();
    serializeSmallInt(result.start.x, ptr);
    ptr += 2;
    serializeSmallInt(result.start.z, ptr);
    ptr += 2;
    serializeSmallInt(result.end.x, ptr);
    ptr += 2;
    serializeSmallInt(result.end.z, ptr);
    ptr += 2;
    serializeInt(result.steps, ptr);
    return out;
}

void writeResults(std::span<Result> results, const std::filesystem::path& path) {
    std::ofstream output(path, std::ios::out | std::ios::binary | std::ios::trunc);
    for (const auto& res : results) {
        auto serialized = serializeResult(res);
        output.write(serialized.data(), serialized.size());
    }
}


int main() {
    //std::cout << EXPECTED_FILE_SIZE << std::endl;
    //return 0;
    auto bigMap = absl::flat_hash_map<SmallSeed, Pos>{};
    constexpr auto neededCapacity = (WOODLAND_BOUNDS * 2) * (WOODLAND_BOUNDS * 2);
    bigMap.reserve(neededCapacity + 1000);
    std::cout << woodlandMansionSeed(-69, 420) << std::endl;
    std::cout << "meow! v3" << std::endl;
    for (int x = -WOODLAND_BOUNDS; x <= WOODLAND_BOUNDS; x++) {
        if (!(x&127))
        fmt::print("generating the big map currently at x={}\n", x);
        std::fflush(stdout);
        for (int z = -WOODLAND_BOUNDS; z <= WOODLAND_BOUNDS; z++) {
            bigMap[SmallSeed{woodlandMansionSeed(x, z)}] = Pos{SmallInt{x}, SmallInt{z}};
        }
    }
    /*for (int x = 0; x < 1000; x++) {
        for (int z = 0; z < 1000; z++) {
            bigMap[SmallSeed{woodlandMansionSeed(x, z)}] = Pos{SmallInt{x}, SmallInt{z}};
        }
    }*/
    std::cout << "bigMap is " << bigMap.size() << " elements large" << std::endl;

    std::filesystem::path outDir{"out"};
    std::filesystem::create_directory(outDir);
    const auto threadCount = std::thread::hardware_concurrency();
    fmt::print("thread count {}\n", threadCount);
    std::fflush(stdout);
    std::atomic_int result_count{};
    std::vector<std::jthread> threads;
    auto prev_time = std::chrono::steady_clock::now();
    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back([i, threadCount, &bigMap, &result_count, &prev_time, outDir] {
            std::vector<Result> results;
            results.reserve(MANSIONS_IN_ROW);
            for (int x = -WOODLAND_BOUNDS; x <= WOODLAND_BOUNDS; x++) {
                auto start_time = std::chrono::steady_clock::now();
                auto file = outDir / std::to_string(x);
                auto err = std::error_code{};
                auto size = std::filesystem::file_size(file, err);
                // if file exists and the size is what we expect, skip
                if (abs(x) % threadCount != i) continue;
                if (!err && size == EXPECTED_FILE_SIZE) {
                    fmt::print("skipping already completed file {}\n", x);
                    std::fflush(stdout);
                    continue;
                }
                for (int z = -WOODLAND_BOUNDS; z <= WOODLAND_BOUNDS; z++) {
                    if (!(z & 8191)){
                        fmt::print("thread {} is at x={} z={}\n", i, x, z);
                        std::fflush(stdout);
                    }
                    auto seed = woodlandMansionSeed(x, z);
                    auto [steps, pos] = stepsForSeed(seed, bigMap);
                    results.push_back(Result{
                        .start = {x, z},
                        .end = pos,
                        .steps = steps
                    });
                    /*result_count++;
                    if (result_count % 20'000 == 0) {
                        auto time = std::chrono::steady_clock::now();
                        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time - prev_time).count();
                        prev_time = time;
                        constexpr auto end = TOTAL_MANSIONS;
                        const auto percent = ((double)result_count / (double)end) * 100;
                        fmt::print("At step {} out of {} ({:.4f}%)\n", result_count, end, percent);
                        fmt::print("20000 results took {} ms ({} s)\n", millis, millis / 1000);
                        std::fflush(stdout);
                    }*/
                }
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
                fmt::print("x {} took {}s, writing file now\n", x, duration);
                std::fflush(stdout);
                writeResults(results, file);
            }
        });
    }
}
