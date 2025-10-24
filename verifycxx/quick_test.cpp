#include <utility>

#include "windows.h"
#include "metrics/unit_test.h"
#include "metrics/benchmark.h"

struct Vector3 {
    float pos[3]{};
};

struct Entity {
    float health{ 100.f }, armour{ 100.f };
    Vector3 position{};
};

struct Player : Entity {
    Player(const int uid, std::string n, const int l) :
        uniqueId(uid), name(std::move(n)), level(l) {};
    ~Player() = default;

    int uniqueId{};
    std::string name{};
    int level{};
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    auto const result = RUN_ALL_TESTS();

    verifycxx<int> value{ 1337 };
    std::cout << &value << std::endl;

    verifycxx<Player> player{ 1, "Nigger", 15 };
    std::cout << &player << "crc: " << (uint64_t)player.get_checksum() << " " << offsetof(verifycxx<Player>, checksum) << std::endl;
    std::cout << player->name.c_str() << std::endl;

    Sleep(INFINITE);

    std::cout << '\n';

    if (result != 0) {
        std::cout << "UNIT TEST FAILED: RESULT: " << result << '\n';
        return result;
    }

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}
