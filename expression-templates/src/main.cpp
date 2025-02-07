#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;

int main() {
  #ifdef NDEBUG
    std::cout << "Release configuration!\n";
    #else
    std::cout << "Debug configuration!\n";
    #endif

    Matrix3d m;

    std::cout << m;
    return 0;
}