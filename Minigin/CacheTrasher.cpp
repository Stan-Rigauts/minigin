#include "CacheTrasher.h"
#include <chrono>
#include <algorithm>
#include <numeric>

static constexpr size_t BUFFER_SIZE = 1000000;

static float Average(std::vector<long long>& times)
{
    if (times.empty())
        return 0.f;

    float sum = 0;
    for (long long t : times)
        sum += t;

    return static_cast<float>(sum / times.size());
}

TimingResult CacheTrasher::RunExercise1(int samples)
{
    TimingResult result;
    result.label = "int buffer";

    std::vector<int> buffer(BUFFER_SIZE, 1);

    for (size_t stepSize = 1; stepSize <= 1024; stepSize *= 2)
    {
        std::vector<long long> timingsForThisStep;

        for (int sample = 0; sample < samples; ++sample)
        {
            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < BUFFER_SIZE; i += stepSize)
                buffer[i] *= 2;

            auto end = std::chrono::high_resolution_clock::now();
            timingsForThisStep.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        result.timings.push_back(Average(timingsForThisStep));
    }

    return result;
}

TimingResult CacheTrasher::RunExercise2_GameObject3D(int samples)
{
    TimingResult result;
    result.label = "GameObject3D (pointer chase)";

    const size_t objectCount = BUFFER_SIZE ;
    std::vector<Transform>    transforms(objectCount);
    std::vector<GameObject3D> objects(objectCount);

    for (size_t i = 0; i < objectCount; ++i)
    {
        objects[i].local = &transforms[i];
        objects[i].id = static_cast<int>(i);
    }

    for (size_t stepSize = 1; stepSize <= 1024; stepSize *= 2)
    {
        std::vector<long long> timingsForThisStep;

        for (int sample = 0; sample < samples; ++sample)
        {
            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < objectCount; i += stepSize)
                objects[i].local->matrix[0] *= 2.0f;

            auto end = std::chrono::high_resolution_clock::now();
            timingsForThisStep.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        result.timings.push_back(Average(timingsForThisStep));
    }

    return result;
}

TimingResult CacheTrasher::RunExercise2_GameObject3DAlt(int samples)
{
    TimingResult result;
    result.label = "GameObject3DAlt (direct transforms)";

    const size_t objectCount = BUFFER_SIZE;
    std::vector<Transform>    transforms(objectCount);
    std::vector<GameObject3D> objects(objectCount);

    for (size_t i = 0; i < objectCount; ++i)
    {
        objects[i].local = &transforms[i];
        objects[i].id = static_cast<int>(i);
    }

    for (size_t stepSize = 1; stepSize <= 1024; stepSize *= 2)
    {
        std::vector<long long> timingsForThisStep;

        for (int sample = 0; sample < samples; ++sample)
        {
            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < objectCount; i += stepSize)
                transforms[i].matrix[0] *= 2.0f;

            auto end = std::chrono::high_resolution_clock::now();
            timingsForThisStep.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        }

        result.timings.push_back(Average(timingsForThisStep));
    }

    return result;
}