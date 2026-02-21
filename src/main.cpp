#include "matrix.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Incorrect number of arguments.\n";
    std::cout
        << "Program execution should look like this: ./project <filename>.\n";
    return 1;
  }
  const char *lr_test = argv[1];
  Matrix m(lr_test);
  m.to_msgm();
  m.make_basis();
  m.create_tables();
  for (std::size_t i = 1; i <= m.get_cols(); i++) {
    std::cout << "E(" << i - 1 << ", " << i << "): \n";
    std::cout << "__________________________________\n";
    m.print_edge(i);
    std::cout << '\n';
  }
}
