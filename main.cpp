#include <cstddef>
#include <vector>
#include <iostream>

int main()
{
    std::vector<float> my_values {1, 2, 3, 4};
    float values_sum = 0.f;
    for (float value: my_values) {
        values_sum += value;
    }

    return 0;
}
