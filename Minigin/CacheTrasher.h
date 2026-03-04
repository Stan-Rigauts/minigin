#pragma once
#include <vector>
#include <string>

struct Transform {
    float matrix[16]{
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        1,2,3,1
    };
};

class GameObject3D {
public:
    Transform* local;
    int id;
};

struct TimingResult {
    std::vector<float> timings; 
    std::string label;
};

class CacheTrasher {
public:
    static TimingResult RunExercise1(int samples);
    static TimingResult RunExercise2_GameObject3D(int samples);
    static TimingResult RunExercise2_GameObject3DAlt(int samples);
};