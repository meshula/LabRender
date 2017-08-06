
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#pragma once

#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>
#include <random>

class LSystemRules;

class LSystem
{
public:
    class Rand {
    public:
        Rand(uint32_t seed = 0x1337) : mersenneTwister(seed), dist(0, 1) {}

        float rand(float min, float max) {
            if (min == max)
                return min;

            float r = dist(mersenneTwister);
            return r * (max - min) + min;
        }

        void seed(int) { }

        std::mt19937 mersenneTwister;
        std::uniform_real_distribution<float> dist;
    };

    struct Shape
    {
        Shape(const std::string&s, const glm::mat4x4& m, int d)
        : shapeName(s), m(m), depth(d) {}
        Shape(const Shape& rhs)
        : shapeName(rhs.shapeName), m(rhs.m), depth(rhs.depth) {}

        std::string shapeName;
        glm::mat4x4 m;
        int depth;
    };

    LSystem(std::ostream & console);
    ~LSystem();

    void parseRules(char const*const rules);

    // returns maximum depth
    int instance(int seed, bool showProgress);

    std::vector<Shape> shapes;

private:
    int max_depth;
    LSystemRules* rules;
    std::ostream& _console;
    Rand* _random;
    std::string _name;
};
