#pragma once
#include <gtest/gtest.h>
#include <include/verifycxx.h>

#include <array>

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

TEST(VerifycxxTest, String)
{
    verifycxx<std::string> string{ "uint_test" };
    EXPECT_STREQ(string, "uint_test");
}

TEST(VerifycxxTest, C_String)
{
    verifycxx<const char*> cstring("uint_test");
    EXPECT_STREQ(cstring, "uint_test");
}

TEST(VerifycxxTest, CustomStruct)
{
    struct Player { int health; int mana; };
    verifycxx<Player> player{ 100, 50 };

    EXPECT_EQ(player.get()->health, 100);
    EXPECT_TRUE(player.verify());
}

TEST(VerifycxxTest, ModifyGuardArrow)
{
    struct Point { int x; int y; };
    verifycxx<Point> point{ 10, 20 };

    auto guard = point.modify();
    guard->x = 100;
    guard->y = 200;

    EXPECT_EQ(point.get()->x, 100);
}

TEST(VerifycxxTest, ModifyGuardDereference)
{
    verifycxx<int> value{ 42 };
    auto guard = value.modify();
    *guard = 100;
    EXPECT_EQ(*value.get(), 100);
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
