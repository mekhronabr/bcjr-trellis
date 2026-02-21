#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

#include "matrix.hpp"

void Matrix::update_span(std::size_t ind) {
  std::pair<std::size_t, std::size_t> sp = span[ind];
  std::size_t block_l = (sp.first == -1 ? 0 : sp.first) / Matrix::BLOCK_SIZE;
  std::size_t block_r = BLOCK_NUM((sp.second == -1 ? matrix[0].size() - 1 : sp.second));

  bool found = false;
  for (std::size_t i = block_l; !found && i <= block_r; i++) {
    for (std::size_t j = Matrix::BLOCK_SIZE; j > 0; j--) {
      if ((matrix[ind][i] & (1LL << (j - 1)))) {
        sp.first = i * Matrix::BLOCK_SIZE + (Matrix::BLOCK_SIZE - j);
        found = true;
        break;
      }
    }
  }
  if (!found) {
    span[ind] = {-1, -1};
    return;
  }

  found = false;
  for (std::size_t i = block_r + 1; !found && i > block_l; i--) {
    for (std::size_t j = 0; j < Matrix::BLOCK_SIZE; j++) {
      if ((matrix[ind][i - 1] & (1LL << j))) {
        sp.second = (i - 1) * Matrix::BLOCK_SIZE + (Matrix::BLOCK_SIZE - j - 1);
        found = true;
        break;
      }
    }
  }

  span[ind] = sp;
}

uint64_t str_to_ui64_block(const std::string &line, std::size_t start) {
  uint64_t block{};
  std::size_t sz = std::min(start + Matrix::BLOCK_SIZE, line.size());
  for (std::size_t i = start; i < sz; i++) {
    if (line[i] == '1') {
      block |= (1LL << (Matrix::BLOCK_SIZE - (i - start) - 1));
    }
  }

  return block;
}

Matrix::Matrix() : k(0), n(0) {}

Matrix::Matrix(const char *filename) {
  std::ifstream f(filename);
  f >> k >> n;

  matrix = std::vector<std::vector<uint64_t>>(k);
  span = std::vector<std::pair<int, int>>(k, {-1, -1});

  std::string line;
  static_cast<void>(std::getline(f, line));

  for (std::size_t i = 0; i < k; i++) {
    std::getline(f, line);

    for (std::size_t j = 0; j < line.size(); j += Matrix::BLOCK_SIZE) {
      matrix[i].push_back(str_to_ui64_block(line, j));
    }
  }

  for (std::size_t i = 0; i < k; i++) {
    update_span(i);
  }
}

const std::vector<std::vector<uint64_t>> &Matrix::get_matrix() const {
  return matrix;
}

std::size_t Matrix::get_cols() const { return n; }

std::size_t Matrix::get_rows() const { return k; }

const std::vector<std::pair<int, int>> &Matrix::get_span() const {
  return span;
}

std::ostream &operator<<(std::ostream &os, const Matrix &m) {
  for (std::size_t i = 0; i < m.k; i++) {
    std::size_t c = m.n;
    for (std::size_t j = 0; j < m.matrix[i].size(); j++) {
      for (std::size_t bit = Matrix::BLOCK_SIZE;
           bit > (c < Matrix::BLOCK_SIZE ? Matrix::BLOCK_SIZE - c : 0); bit--) {
        os << ((m.matrix[i][j] & (1LL << (bit - 1))) ? "1" : "0");
      }
      os << '\n';
      c -= Matrix::BLOCK_SIZE;
    }
  }

  for (auto &[a, b] : m.span)
    std::cout << a << ' ' << b << '\n';
  return os;
}

void Matrix::make_basis() {
  std::vector<std::vector<uint64_t>> v;
  std::vector<std::pair<int, int>> new_span;
  for (std::size_t i = 0; i < k; i++) {
    if (span[i] != std::pair<int, int>{-1, -1}) {
      v.push_back(matrix[i]);
      new_span.push_back(span[i]);
    }
  }
  matrix = std::move(v);
  k = matrix.size();
  span = std::move(new_span);
}

void Matrix::to_msgm() {
  bool need_change = true;
  while (need_change) {
    need_change = false;
    for (std::size_t i = 0; i < k; i++) {
      for (std::size_t j = 0; j < k; j++) {
        if (i == j)
          continue;
        if (span[i].first == span[j].first) {
          if (span[i].second < span[j].second) {
            add_rows(i, j);
          } else {
            add_rows(j, i);
          }
          need_change = true;
        }
        if (span[i].second == span[j].second) {
          if (span[i].first < span[j].first) {
            add_rows(j, i);
          } else {
            add_rows(i, j);
          }
          need_change = true;
        }
      }
    }
  }
}

void Matrix::add_rows(std::size_t v1, std::size_t v2) {
  for (std::size_t i = 0; i < matrix[v1].size(); i++) {
    matrix[v2][i] ^= matrix[v1][i];
  }

  update_span(v2);
}

void Matrix::fill_sets() {
  a = std::vector<std::vector<uint32_t>>(n + 1);
  b = std::vector<std::unordered_set<uint32_t>>(n + 1);
  a_coord =
      std::vector<std::vector<int32_t>>(n + 1, std::vector<int32_t>(k + 1, -1));

  for (std::size_t i = 0; i <= n; i++) {
    for (std::size_t j = 0; j < k; j++) {
      if (span[j].first + 1 <= i && i <= span[j].second + 1) {
        a_coord[i][j + 1] = a[i].size();
        a[i].push_back(j + 1);
        if (i != span[j].second + 1) {
          b[i].insert(j + 1);
        }
      }
    }
  }
}

void Matrix::init_and_fill_init() {
  init_ = std::vector<std::vector<uint32_t>>(a.size());
  for (std::size_t ind = 1; ind < a.size(); ind++) {
    for (const auto &el : a[ind]) {
      if (b[ind - 1].count(el)) {
        init_[ind].push_back(el);
      }
    }
  }
}

void Matrix::init_and_fill_fin() {
  fin_ = std::vector<std::vector<uint32_t>>(a.size());
  for (std::size_t ind = 1; ind < a.size(); ind++) {
    for (const auto &el : a[ind]) {
      if (b[ind].count(el)) {
        fin_[ind].push_back(el);
      }
    }
  }
}

std::pair<std::size_t, std::size_t> get_pos(std::size_t ind) {
  return {BLOCK_NUM(ind),
          (Matrix::BLOCK_SIZE - ind % Matrix::BLOCK_SIZE) % Matrix::BLOCK_SIZE};
}

void Matrix::init_and_fill_lambda() {
  g_ = std::vector<std::string>(n + 1);
  for (std::size_t ind = 1; ind <= n; ind++) {
    auto [y1, y2] = get_pos(ind);
    for (const auto &x : a[ind]) {
      g_[ind] += ((matrix[x - 1][y1] & (1LL << y2)) != 0 ? '1' : '0');
    }
  }
}

std::string to_binary(uint32_t x, std::size_t length) {
  std::string res;
  while (x) {
    res += (x % 2 + '0');
    x /= 2;
  }
  res += std::string(length - res.size(), '0');
  std::reverse(res.begin(), res.end());
  return res;
}

void Matrix::print_edge(std::size_t ind) const {
  std::vector<std::string> a_binary((1LL << a[ind].size()));
  for (std::size_t i = 0; i <= (1LL << a[ind].size()) - 1; i++) {
    a_binary[i] = to_binary(i, a[ind].size());
  }

  std::cout << "u         | ";
  for (const auto &u : a_binary) {
    std::cout << u << " | ";
  }
  std::cout << '\n';

  std::cout << "init(u)   | ";
  for (const auto &b : a_binary) {
    std::string v;
    for (const auto &u : init_[ind]) {
      v += b[a_coord[ind][u]];
    }
    std::cout << v << std::string(a[ind].size() - v.size(), ' ') << " | ";
  }
  std::cout << '\n';

  std::cout << "fin(u)    | ";
  for (const auto &b : a_binary) {
    std::string v;
    for (const auto &u : fin_[ind]) {
      v += b[a_coord[ind][u]];
    }
    std::cout << v << std::string(a[ind].size() - v.size(), ' ') << " | ";
  }
  std::cout << '\n';

  std::cout << "lambda(u) | ";
  uint8_t l = 0;
  for (const std::string &b : a_binary) {
    l = 0;
    for (std::size_t i = 0; i < b.size(); i++) {
      l = ((l + (b[i] - '0') * (g_[ind][i] - '0')) & 0x1);
    }
    std::cout << (int)l << std::string(a[ind].size() - 1, ' ') << " | ";
  }
  std::cout << '\n';
}

void Matrix::create_tables() {
  fill_sets();
  init_and_fill_init();
  init_and_fill_fin();
  init_and_fill_lambda();
}
