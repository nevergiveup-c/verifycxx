#pragma once
#include <array>
#include <benchmark/benchmark.h>

#include <include/verifycxx.h>

static void BM_ValueModify(benchmark::State& state) {
    verifycxx<int> value{ 100 };
    int i = 0;
    for (auto _ : state) {
        value.modify() = i++;
        benchmark::DoNotOptimize(i);
    }
}

BENCHMARK(BM_ValueModify);

static void BM_ValueVerify(benchmark::State& state) {
    verifycxx<int> value{ 100 };
    for (auto _ : state) {
        benchmark::DoNotOptimize(value.verify());
    }
}

BENCHMARK(BM_ValueVerify);

static void BM_ChecksumRecalculation(benchmark::State& state) {
    verifycxx<int> value{ 100 };
    for (auto _ : state) {
        benchmark::DoNotOptimize(value.get_checksum());
    }
}

BENCHMARK(BM_ChecksumRecalculation);

static void BM_ArrayModify(benchmark::State& state) {
    verifycxx<std::array<int, 10>> array{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    for (auto _ : state) {
        int j = 0;
        for (auto& val : *array.modify()) {
            val = j++;
        }
        benchmark::DoNotOptimize(j);
    }
}

BENCHMARK(BM_ArrayModify);

static void BM_ArrayVerify(benchmark::State& state) {
    verifycxx<std::array<int, 10>> array{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    for (auto _ : state) {
        benchmark::DoNotOptimize(array.verify());
    }
}

BENCHMARK(BM_ArrayVerify);

static void BM_StringModify(benchmark::State& state) {
    verifycxx<std::string> str{ "test_string" };
    for (auto _ : state) {
        auto guard = str.modify();
        *guard = "modified_string";
    }
}

BENCHMARK(BM_StringModify);

static void BM_StringVerify(benchmark::State& state) {
    verifycxx<std::string> str{ "test_string" };
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.verify());
    }
}

BENCHMARK(BM_StringVerify);