#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

#define BLOCK_NUM(n) (n / (Matrix::BLOCK_SIZE - 1)) / Matrix::BLOCK_SIZE


class Matrix {
public:
  static constexpr inline std::size_t BLOCK_SIZE = 64;
  Matrix();
  Matrix(const char *filename);
  void update_span(std::size_t ind);

  friend std::ostream &operator<<(std::ostream &os, const Matrix &m);

  const std::vector<std::vector<uint64_t>> &get_matrix() const;
  std::size_t get_rows() const;
  std::size_t get_cols() const;
  const std::vector<std::pair<int, int>> &get_span() const;

  void make_basis();
  void to_msgm();
  void create_tables();
  void print_edge(std::size_t ind) const;
  void print_table_data(std::size_t ind);

private:
  std::size_t k, n;
  std::vector<std::vector<uint64_t>> matrix;
  std::vector<std::pair<int, int>> span;
  std::vector<std::unordered_set<uint32_t>> b;
  std::vector<std::vector<int32_t>> a_coord;
  std::vector<std::vector<uint32_t>> a, init_, fin_;
  std::vector<std::string> g_;
  void add_rows(std::size_t v1, std::size_t v2);
  void init_and_fill_init();
  void init_and_fill_fin();
  void init_and_fill_lambda();
  void fill_sets();
};

#endif // MATRIX_H
