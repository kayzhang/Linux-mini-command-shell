#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char ** argv) {
  for (int i = 1; i < argc; i++) {
    std::cout << "The " << i << "th argument is: " << argv[i] << std::endl;
  }

  return EXIT_SUCCESS;
}
