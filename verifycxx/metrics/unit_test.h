#pragma once
#include <gtest/gtest.h>
#include <include/verifycxx.h>

#include <array>
#include <vector>
#include <string>

TEST(VerifycxxTest, ChecksumGenerate)
{
    verifycxx<int> value{ 100 };
    EXPECT_NE(value.get_checksum(), 0);
}

TEST(VerifycxxTest, Verify)
{
    verifycxx<int> value{ 100 };
    EXPECT_TRUE(value.verify());
    value.modify() = 10;
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, BoolConversion)
{
    verifycxx<int> value{ 100 };
    EXPECT_TRUE(value);
}

TEST(VerifycxxTest, DetectTampering)
{
    verifycxx<int> value{ 100 };
    EXPECT_TRUE(value.verify());
    *const_cast<int*>(value.get()) = 999;
    EXPECT_FALSE(value.verify());
}

TEST(VerifycxxTest, ScalarConversion)
{
    verifycxx<int> value{ 42 };
    int x = value;
    EXPECT_EQ(x, 42);
}

TEST(VerifycxxTest, ScalarDereference)
{
    verifycxx<double> value{ 3.14 };
    EXPECT_DOUBLE_EQ(*value, 3.14);
}

TEST(VerifycxxTest, AssignmentOperator)
{
    verifycxx<int> value{ 10 };
    value.modify() = 50;
    EXPECT_EQ(*value.get(), 50);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, AddAssignOperator)
{
    verifycxx<int> value{ 10 };
    value.modify() += 5;
    EXPECT_EQ(*value.get(), 15);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, SubtractAssignOperator)
{
    verifycxx<int> value{ 20 };
    value.modify() -= 7;
    EXPECT_EQ(*value.get(), 13);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, MultiplyAssignOperator)
{
    verifycxx<int> value{ 5 };
    value.modify() *= 3;
    EXPECT_EQ(*value.get(), 15);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, DivideAssignOperator)
{
    verifycxx<int> value{ 20 };
    value.modify() /= 4;
    EXPECT_EQ(*value.get(), 5);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, ModuloAssignOperator)
{
    verifycxx<int> value{ 17 };
    value.modify() %= 5;
    EXPECT_EQ(*value.get(), 2);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, BitwiseAndAssignOperator)
{
    verifycxx<int> value{ 0b1111 };
    value.modify() &= 0b1010;
    EXPECT_EQ(*value.get(), 0b1010);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, BitwiseOrAssignOperator)
{
    verifycxx<int> value{ 0b1010 };
    value.modify() |= 0b0101;
    EXPECT_EQ(*value.get(), 0b1111);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, BitwiseXorAssignOperator)
{
    verifycxx<int> value{ 0b1111 };
    value.modify() ^= 0b1010;
    EXPECT_EQ(*value.get(), 0b0101);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, LeftShiftAssignOperator)
{
    verifycxx<int> value{ 5 };
    value.modify() <<= 2;
    EXPECT_EQ(*value.get(), 20);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, RightShiftAssignOperator)
{
    verifycxx<int> value{ 20 };
    value.modify() >>= 2;
    EXPECT_EQ(*value.get(), 5);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, PrefixIncrementOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    ++guard;
    EXPECT_EQ(*guard, 11);
}

TEST(VerifycxxTest, PostfixIncrementOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    int old = guard++;
    EXPECT_EQ(old, 10);
    EXPECT_EQ(*guard, 11);
}

TEST(VerifycxxTest, PrefixDecrementOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    --guard;
    EXPECT_EQ(*guard, 9);
}

TEST(VerifycxxTest, PostfixDecrementOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    int old = guard--;
    EXPECT_EQ(old, 10);
    EXPECT_EQ(*guard, 9);
}

TEST(VerifycxxTest, AdditionOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    int result = guard + 5;
    EXPECT_EQ(result, 15);
    EXPECT_EQ(*guard, 10);
}

TEST(VerifycxxTest, SubtractionOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    int result = guard - 3;
    EXPECT_EQ(result, 7);
}

TEST(VerifycxxTest, MultiplicationOperator)
{
    verifycxx<int> value{ 10 };
    auto guard = value.modify();
    int result = guard * 3;
    EXPECT_EQ(result, 30);
}

TEST(VerifycxxTest, DivisionOperator)
{
    verifycxx<int> value{ 20 };
    auto guard = value.modify();
    int result = guard / 4;
    EXPECT_EQ(result, 5);
}

TEST(VerifycxxTest, ModuloOperator)
{
    verifycxx<int> value{ 17 };
    auto guard = value.modify();
    int result = guard % 5;
    EXPECT_EQ(result, 2);
}

TEST(VerifycxxTest, Array)
{
    verifycxx<std::array<int, 5>> array{ 1, 2, 3, 4, 5 };
    for (int j{}; auto i : array) {
        EXPECT_EQ(i, ++j);
    }
}

TEST(VerifycxxTest, ArrayIndexing)
{
    verifycxx<std::array<int, 5>> array{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(array[0], 1);
    EXPECT_EQ(array[4], 5);
}

TEST(VerifycxxTest, ArrayModifyIndexing)
{
    verifycxx<std::array<int, 5>> array{ 1, 2, 3, 4, 5 };
    auto guard = array.modify();
    guard[0] = 100;
    guard[4] = 500;
    EXPECT_EQ(guard[0], 100);
    EXPECT_EQ(guard[4], 500);
}

TEST(VerifycxxTest, String)
{
    verifycxx<std::string> string{ "uint_test" };
    EXPECT_STREQ(string, "uint_test");
}

TEST(VerifycxxTest, StringModify)
{
    verifycxx<std::string> str{ "hello" };
    auto guard = str.modify();
    *guard = "world";
    EXPECT_STREQ(str, "world");
}

TEST(VerifycxxTest, StringAppend)
{
    verifycxx<std::string> str{ "hello" };
    auto guard = str.modify();
    guard->append(" world");
    EXPECT_STREQ(str, "hello world");
}

TEST(VerifycxxTest, C_String)
{
    verifycxx<const char*> cstring("uint_test");
    EXPECT_STREQ(cstring, "uint_test");
}

TEST(VerifycxxTest, CustomStruct)
{
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

    verifycxx<Player> player{ 1, "best_player", 999 };

    EXPECT_EQ(player.get()->health, 100);
    EXPECT_TRUE(player.verify());
}

TEST(VerifycxxTest, ModifyGuardArrow)
{
    struct Point { int x; int y; };
    verifycxx<Point> point{ 10, 20 };

    {
        auto guard = point.modify();
        guard->x = 100;
        guard->y = 200;
    }

    EXPECT_EQ(point.get()->x, 100);
}

TEST(VerifycxxTest, ModifyGuardDereference)
{
    verifycxx<int> value{ 42 };
    auto guard = value.modify();
    *guard = 100;
    EXPECT_EQ(*guard, 100);
}

TEST(VerifycxxTest, ModifyUpdatesChecksum)
{
    verifycxx<int> value{ 100 };
    uint64_t old_checksum = value.get_checksum();

    value.modify() = 200;

    EXPECT_NE(value.get_checksum(), old_checksum);
    EXPECT_TRUE(value.verify());
}

TEST(VerifycxxTest, Vector)
{
    verifycxx<std::vector<int>> vec{ 1, 2, 3 };
    EXPECT_EQ(vec[1], 2);
    int sum = 0;
    for (auto v : vec) sum += v;
    EXPECT_EQ(sum, 6);
}

TEST(VerifycxxTest, VectorModify)
{
    verifycxx<std::vector<int>> vec{ 1, 2, 3 };
    vec.modify()->push_back(4);
    EXPECT_TRUE(vec.verify());
}

TEST(VerifycxxTest, VectorIndexModify)
{
    verifycxx<std::vector<int>> vec{ 10, 20, 30 };
    vec.modify()[1] = 999;
    EXPECT_TRUE(vec.verify());
}

TEST(VerifycxxTest, MultiThreadVerify)
{
    verifycxx<int> val{ 1 };

    std::thread t1([&] {
        for (int i = 0; i < 1000000; i++) {
            ++val.modify();
        }
    });

    std::thread t2([&] {
        static bool falied = false;
        for (int i = 0; i < 1000000; i++) {
            if (!val.verify()) {
                falied = true;
                break;
            }
        }
        EXPECT_FALSE(falied);
    });

    std::thread t3([&] {
        for (int i = 0; i < 1000000; i++) {
            --val.modify();
        }
    });

    t3.join();
    t2.join();
    t1.join();
}