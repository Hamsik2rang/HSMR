//
//  ListExample.cpp
//  Core
//
//  Usage examples and performance comparison of custom List vs std::vector
//
#include "Core/Container/List.h"
#include "Core/Log.h"
#include <vector>
#include <chrono>
#include <random>

HS_NS_BEGIN

namespace ListExample
{
    struct Vertex
    {
        float x, y, z;
        float u, v;
        uint32 color;
        
        Vertex() = default;
        Vertex(float x, float y, float z) : x(x), y(y), z(z), u(0), v(0), color(0xFFFFFFFF) {}
    };

    struct TestResult
    {
        double timeMs;
        size_t memoryUsage;
        const char* testName;
    };

    void PrintResults(const TestResult& customResult, const TestResult& stdResult)
    {
        double speedup = stdResult.timeMs / customResult.timeMs;
        double memoryRatio = static_cast<double>(stdResult.memoryUsage) / customResult.memoryUsage;
        
        HS_LOG(info, "=== {} ===", customResult.testName);
        HS_LOG(info, "Custom List: {:.2f}ms, {}KB", customResult.timeMs, customResult.memoryUsage / 1024);
        HS_LOG(info, "std::vector: {:.2f}ms, {}KB", stdResult.timeMs, stdResult.memoryUsage / 1024);
        HS_LOG(info, "Speedup: {:.2f}x, Memory: {:.2f}x", speedup, memoryRatio);
        HS_LOG(info, "");
    }

    // Test 1: Large sequential insertions
    void TestSequentialInsertion()
    {
        constexpr size_t TEST_SIZE = 1000000;
        
        // Custom List test
        auto start = std::chrono::high_resolution_clock::now();
        {
            List<Vertex> customList;
            customList.Reserve(TEST_SIZE); // Pre-allocate to avoid reallocations
            
            for (size_t i = 0; i < TEST_SIZE; ++i)
            {
                customList.EmplaceBack(static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3));
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto customTime = std::chrono::duration<double, std::milli>(end - start).count();

        // std::vector test
        start = std::chrono::high_resolution_clock::now();
        {
            std::vector<Vertex> stdVector;
            stdVector.reserve(TEST_SIZE); // Pre-allocate for fair comparison
            
            for (size_t i = 0; i < TEST_SIZE; ++i)
            {
                stdVector.emplace_back(static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3));
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto stdTime = std::chrono::duration<double, std::milli>(end - start).count();

        TestResult customResult = {customTime, TEST_SIZE * sizeof(Vertex), "Sequential Insertion"};
        TestResult stdResult = {stdTime, TEST_SIZE * sizeof(Vertex), "Sequential Insertion"};
        
        PrintResults(customResult, stdResult);
    }

    // Test 2: Frequent reallocations (worst case for std::vector)
    void TestFrequentReallocations()
    {
        constexpr size_t TEST_SIZE = 100000;
        
        // Custom List test (1.5x growth vs 2x)
        auto start = std::chrono::high_resolution_clock::now();
        {
            List<Vertex> customList;
            for (size_t i = 0; i < TEST_SIZE; ++i)
            {
                customList.PushBack(Vertex(static_cast<float>(i), 0, 0));
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto customTime = std::chrono::duration<double, std::milli>(end - start).count();

        // std::vector test (typically 2x growth)
        start = std::chrono::high_resolution_clock::now();
        {
            std::vector<Vertex> stdVector;
            for (size_t i = 0; i < TEST_SIZE; ++i)
            {
                stdVector.push_back(Vertex(static_cast<float>(i), 0, 0));
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto stdTime = std::chrono::duration<double, std::milli>(end - start).count();

        // Custom List uses 1.5x growth, so less memory overhead
        size_t customMemory = TEST_SIZE * sizeof(Vertex) * 3 / 2; // Approximate
        size_t stdMemory = TEST_SIZE * sizeof(Vertex) * 2;        // std::vector typically uses 2x

        TestResult customResult = {customTime, customMemory, "Frequent Reallocations"};
        TestResult stdResult = {stdTime, stdMemory, "Frequent Reallocations"};
        
        PrintResults(customResult, stdResult);
    }

    // Test 3: Random access patterns
    void TestRandomAccess()
    {
        constexpr size_t TEST_SIZE = 1000000;
        constexpr size_t ACCESS_COUNT = 10000000;
        
        // Setup data
        List<int> customList;
        std::vector<int> stdVector;
        
        customList.Reserve(TEST_SIZE);
        stdVector.reserve(TEST_SIZE);
        
        for (size_t i = 0; i < TEST_SIZE; ++i)
        {
            customList.PushBack(static_cast<int>(i));
            stdVector.push_back(static_cast<int>(i));
        }

        // Random indices
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, TEST_SIZE - 1);
        
        std::vector<size_t> indices;
        indices.reserve(ACCESS_COUNT);
        for (size_t i = 0; i < ACCESS_COUNT; ++i)
        {
            indices.push_back(dis(gen));
        }

        // Custom List test
        auto start = std::chrono::high_resolution_clock::now();
        volatile int sum = 0; // Prevent optimization
        for (size_t idx : indices)
        {
            sum += customList[idx];
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto customTime = std::chrono::duration<double, std::milli>(end - start).count();

        // std::vector test
        start = std::chrono::high_resolution_clock::now();
        sum = 0;
        for (size_t idx : indices)
        {
            sum += stdVector[idx];
        }
        end = std::chrono::high_resolution_clock::now();
        auto stdTime = std::chrono::duration<double, std::milli>(end - start).count();

        TestResult customResult = {customTime, TEST_SIZE * sizeof(int), "Random Access"};
        TestResult stdResult = {stdTime, TEST_SIZE * sizeof(int), "Random Access"};
        
        PrintResults(customResult, stdResult);
    }

    // Test 4: Batch operations
    void TestBatchOperations()
    {
        constexpr size_t BATCH_SIZE = 10000;
        constexpr size_t BATCH_COUNT = 100;
        
        // Custom List test
        auto start = std::chrono::high_resolution_clock::now();
        {
            List<float> customList;
            customList.Reserve(BATCH_SIZE * BATCH_COUNT);
            
            for (size_t batch = 0; batch < BATCH_COUNT; ++batch)
            {
                std::vector<float> tempData;
                tempData.reserve(BATCH_SIZE);
                for (size_t i = 0; i < BATCH_SIZE; ++i)
                {
                    tempData.push_back(static_cast<float>(batch * BATCH_SIZE + i));
                }
                
                // Batch insert
                customList.PushBackRange(tempData.data(), tempData.data() + tempData.size());
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto customTime = std::chrono::duration<double, std::milli>(end - start).count();

        // std::vector test (no native batch insert)
        start = std::chrono::high_resolution_clock::now();
        {
            std::vector<float> stdVector;
            stdVector.reserve(BATCH_SIZE * BATCH_COUNT);
            
            for (size_t batch = 0; batch < BATCH_COUNT; ++batch)
            {
                for (size_t i = 0; i < BATCH_SIZE; ++i)
                {
                    stdVector.push_back(static_cast<float>(batch * BATCH_SIZE + i));
                }
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto stdTime = std::chrono::duration<double, std::milli>(end - start).count();

        TestResult customResult = {customTime, BATCH_SIZE * BATCH_COUNT * sizeof(float), "Batch Operations"};
        TestResult stdResult = {stdTime, BATCH_SIZE * BATCH_COUNT * sizeof(float), "Batch Operations"};
        
        PrintResults(customResult, stdResult);
    }

    // Usage examples
    void ShowUsageExamples()
    {
        HS_LOG(info, "=== Usage Examples ===");
        
        // Basic usage
        List<int> numbers = {1, 2, 3, 4, 5};
        
        // Adding elements
        numbers.PushBack(6);
        numbers.EmplaceBack(7);
        
        // Accessing elements
        HS_LOG(info, "First: {}, Last: {}", numbers.Front(), numbers.Back());
        HS_LOG(info, "Size: {}, Capacity: {}", numbers.Size(), numbers.Capacity());
        
        // Iteration
        HS_LOG(info, "Elements:");
        for (const auto& num : numbers)
        {
            HS_LOG(info, "  {}", num);
        }
        
        // Find and erase
        auto it = numbers.Find(4);
        if (it != numbers.End())
        {
            numbers.Erase(it);
            HS_LOG(info, "Erased 4, new size: {}", numbers.Size());
        }
        
        // Fast erase (swap with last)
        numbers.EraseSwap(0); // Remove first element
        HS_LOG(info, "After fast erase, first element: {}", numbers.Front());
        
        // Batch operations
        std::vector<int> moreNumbers = {10, 11, 12};
        numbers.PushBackRange(moreNumbers.begin(), moreNumbers.end());
        
        HS_LOG(info, "Final size: {}", numbers.Size());
        HS_LOG(info, "");
    }

    // Run all performance tests
    void RunPerformanceTests()
    {
        HS_LOG(info, "Starting List vs std::vector performance comparison...");
        HS_LOG(info, "");
        
        ShowUsageExamples();
        TestSequentialInsertion();
        TestFrequentReallocations();
        TestRandomAccess();
        TestBatchOperations();
        
        HS_LOG(info, "Performance tests completed!");
    }
}

HS_NS_END