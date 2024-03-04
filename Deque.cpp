#include <iostream>
#include <vector>
#include <deque>

template <typename T>
class Deque;

template <typename T, bool is_const>
class Deque_iterator {
 public:
  T** array;
  int pos;
  size_t sz_of_arrays;
  Deque_iterator(T** array, int pos, size_t sz) : array(array), pos(pos), sz_of_arrays(sz) {}
  Deque_iterator() : array(nullptr), pos(0), sz_of_arrays(32) {}

 public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::conditional_t<is_const, const T, T>;
  using difference_type = int;
  using pointer = std::conditional_t<is_const, const T*, T*>;
  using reference = std::conditional_t<is_const, const T&, T&>;


  Deque_iterator& operator++() {
    if (++pos == static_cast<int>(sz_of_arrays)) {
      array++;
      pos = 0;
    }
    return *this;
  }

  Deque_iterator operator++(int) {
    Deque_iterator copy = *this;
    ++*this;
    return copy;
  }

  Deque_iterator& operator--() {
    if (--pos == -1) {
      array--;
      pos = sz_of_arrays - 1;
    }
    return *this;
  }

  Deque_iterator& operator+=(int shift) {
    int array_shift = (pos + shift) / sz_of_arrays;
    T** new_arr = array + array_shift;
    int new_pos = (pos + shift) % sz_of_arrays;
    Deque_iterator a(new_arr, new_pos, sz_of_arrays);
    *this = a;
    return *this;
  }

  Deque_iterator operator+(int shift) const {
    Deque_iterator a = *this;
    a += shift;
    return a;
  }

  Deque_iterator operator-=(int shift) {
    int array_shift = (sz_of_arrays + (shift - pos) - 1) / sz_of_arrays;
    T** new_arr = array - array_shift;
    int new_pos = (pos - shift) % sz_of_arrays;
    new_pos = (new_pos + sz_of_arrays ) % sz_of_arrays;
    Deque_iterator a(new_arr, new_pos, sz_of_arrays);
    *this = a;
    return *this;
  }

  Deque_iterator operator-(int shift) const {
    Deque_iterator a = *this;
    a -= shift;
    return a;
  }


//  friend bool operator<(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b);
//  friend bool operator==(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b);
//  friend int operator-(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b);
//  friend class Deque<T>;

  using type1 = std::conditional_t<is_const, const T&, T&>;
  using type2 = std::conditional_t<is_const, const T*, T*>;

  type1 operator*() {
    return *(*array + pos);
  }

  const type1 operator*() const {
    return *(*array + pos);
  }

  type2 operator->() {
    return *array + pos;
  }

  operator Deque_iterator<T, true>() {
    return (Deque_iterator<T, true>(array, pos, sz_of_arrays));
  }

};

template <typename T, bool is_const>
bool operator<(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  if (a.array == b.array) {
    return a.pos < b.pos;
  } else {
    return a.array < b.array;
  }
}

template <typename T, bool is_const>
bool operator==(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  return (a.array == b.array && a.pos == b.pos);
}

template <typename T, bool is_const>
bool operator>(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  return b < a;
}

template <typename T, bool is_const>
bool operator>=(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  return !(b > a);
}

template <typename T, bool is_const>
bool operator<=(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  return !(b < a);
}

template <typename T, bool is_const>
bool operator!=(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  return !(a == b);
}

template <typename T, bool is_const>
int operator-(const Deque_iterator<T, is_const>& a, const Deque_iterator<T, is_const>& b) {
  if (a.array == b.array) {
    return a.pos - b.pos;
  }
  int s1 = b.sz_of_arrays - b.pos - 1;
  int s2 = ((a.array - b.array) - 1) * a.sz_of_arrays;
  int s3 = a.pos + 1;
  return s1 + s2 + s3;
}

template <typename T>
class Deque {
 public:
  std::vector<T*> source;
  size_t size_of_arrays = 32;
  Deque_iterator<T, false> front_of_start;
  Deque_iterator<T, false> after_last;

  using iterator = Deque_iterator<T, false>;
  using const_iterator = Deque_iterator<T, true>;

  void ReallocateAndPush(bool at_the_end, const T& value) {
    size_t sz = source.size();
    int count_of_elements = size();
    std::vector<T*> new_arr(sz * 3);

    for (size_t i = 0; i < sz; ++i) {
      new_arr[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }
    for (size_t i = sz; i < 2 * sz; ++i) {
      new_arr[i] = source[i - sz];
    }
    for (size_t i = 2 * sz; i < 3 * sz; ++i) {
      new_arr[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }
    int arr_of_begin = front_of_start.array - &source[0];
    Deque_iterator<T, false> new_start(&new_arr[sz + arr_of_begin],
                                       front_of_start.pos, size_of_arrays);
    Deque_iterator<T, false> new_finish = new_start + count_of_elements + 1;

    try {
      if (at_the_end) {
        new ((*new_finish.array) + new_finish.pos) T(value);
        ++new_finish;
      } else {
        new ((*new_start.array) + new_start.pos) T(value);
        --new_start;
      }
    } catch (...) {
      for (Deque_iterator<T, false> it = new_start + 1; it < new_finish; ++it) {
        (*it.array + it.pos)->~T();
      }
      for (size_t k = 0; k < 3 * sz; ++k) {
        delete[] reinterpret_cast<char*>(new_arr[k]);
      }
      throw;
    }

    source = new_arr;
    Deque_iterator<T, false> new_star(&new_arr[sz + arr_of_begin],
                                      front_of_start.pos, size_of_arrays);

    front_of_start = Deque_iterator<T, false>(&source[sz + arr_of_begin],
                                              front_of_start.pos, size_of_arrays);
    if (!at_the_end) {
      --front_of_start;
    }

    after_last = front_of_start + count_of_elements + 2;
  }

 public:

  Deque() {
    source = std::vector<T*>(1);
    source[0] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    front_of_start = Deque_iterator<T, false>(&source[0], 0, size_of_arrays);
    after_last = Deque_iterator<T, false>(&source[0], 1, size_of_arrays);
  }

  Deque(Deque const &src) : size_of_arrays(src.size_of_arrays) {
    source = std::vector<T*>(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
      source[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }
    int arr_of_begin = src.front_of_start.array - &src.source[0];
    int arr_of_end = src.after_last.array - &src.source[0];
    front_of_start = Deque_iterator<T, false>(&source[arr_of_begin],
                                              src.front_of_start.pos, size_of_arrays);
    after_last = Deque_iterator<T, false>(&source[arr_of_end],
                                          src.after_last.pos, size_of_arrays);
    auto it1 = src.begin();
    for (auto it0 = begin(); it0 < end(); ++it0) {
      new (*it0.array + it0.pos) T(*it1);
      ++it1;
    }
  }

  Deque(const int size) {
    int count_of_arrays = (size + size_of_arrays + 1) / size_of_arrays;
    source = std::vector<T*>(count_of_arrays);
    for (int i = 0 ; i < count_of_arrays; ++i) {
      source[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }
    Deque_iterator<T, false> new_start(&source[0],
                                       0, size_of_arrays);
    Deque_iterator<T, false> new_finish = new_start + 1;

    try {
      for (int i = 1; i <= size; ++i) {
        new (*new_finish.array + new_finish.pos) T();
        ++new_finish;
      }
    } catch (...) {
      for (Deque_iterator<T, false> it = new_start + 1; it < new_finish; ++it) {
        (*it.array + it.pos)->~T();
      }
      for (size_t k = 0; k < count_of_arrays; ++k) {
        delete[] reinterpret_cast<char*>(source[k]);
      }
      throw;
    }

    after_last = new_finish;
    front_of_start = new_start;
  }

  Deque(int size, const T& el) {
    int count_of_arrays = (size + size_of_arrays + 1) / size_of_arrays;
    source = std::vector<T*>(count_of_arrays);
    for (int i = 0 ; i < count_of_arrays; ++i) {
      source[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }

    for (int i = 1; i <= size; ++i) {
      new (source[i / size_of_arrays] + (i % size_of_arrays)) T(el);
    }

    front_of_start = Deque_iterator<T, false>(&source[0],
                                              0, size_of_arrays);

    after_last = Deque_iterator<T, false>(&source[count_of_arrays - 1],
                                          (size + 1) % size_of_arrays, size_of_arrays);
  }

  Deque& operator=(const Deque& src) {
    size_of_arrays = src.size_of_arrays;
    source = std::vector<T*>(src.source.size());
    for (size_t i = 0; i < src.source.size(); ++i) {
      source[i] = reinterpret_cast<T*>(new char[size_of_arrays * sizeof(T)]);
    }
    int arr_of_begin = src.front_of_start.array - &src.source[0];
    int arr_of_end = src.after_last.array - &src.source[0];
    front_of_start = Deque_iterator<T, false>(&source[arr_of_begin],
                                              src.front_of_start.pos, size_of_arrays);
    after_last = Deque_iterator<T, false>(&source[arr_of_end],
                                          src.after_last.pos, size_of_arrays);
    auto it1 = src.begin();
    for (auto it0 = begin(); it0 < end(); ++it0) {
      new (*it0.array + it0.pos) T(*it1);
      ++it1;
    }
    return *this;
  }

  size_t size() const {
    return (after_last - front_of_start) - 1;
  }

  T& operator[](int pos) {
    return *(front_of_start + (1 + pos));
  }

  const T& operator[](int pos) const {
    return *(front_of_start + (1 + pos));
  }

  T& at(size_t pos) {
    if (pos >= size()) {
      throw std::out_of_range("out of range");
    }
    return *(front_of_start + (1 + pos));
  }

  const T& at(size_t pos) const {
    if (pos >= size()) {
      throw std::out_of_range("out of range");
    }
    return *(front_of_start + (1 + pos));
  }

  void push_back(const T& value) {
    if (static_cast<unsigned long>(after_last.array - &source[0]) > source.size() - 1) {
      try {
        ReallocateAndPush(true, value);
      } catch(...) {
        throw;
      }
    } else {
      new (*after_last.array + after_last.pos) T(value);
      ++after_last;
    }
  }

  void push_front(const T& value) {
    if ((front_of_start.array == &source[0] && front_of_start.pos == 0) ) {
      try {
        ReallocateAndPush(false, value);
      } catch(...) {
        throw;
      }
    } else {
      new (*front_of_start.array + front_of_start.pos) T(value);
      --front_of_start;
    }
  }

  void pop_back() {
    --after_last;
    (*after_last.array + after_last.pos)->~T();
  }

  void pop_front() {
    ++front_of_start;
    (*front_of_start.array + front_of_start.pos)->~T();
  }

  Deque_iterator<T, false> begin() {
    return {(front_of_start + 1).array, (front_of_start + 1).pos, size_of_arrays};
  }

  Deque_iterator<T, true> begin() const {
    return {(front_of_start + 1).array, (front_of_start + 1).pos, size_of_arrays};
  }

  Deque_iterator<T, true> cbegin() {
    return {(front_of_start + 1).array, (front_of_start + 1).pos, size_of_arrays};
  }

  Deque_iterator<T, false> end() {
    return after_last;
  }

  Deque_iterator<T, true> end() const {
    return {(after_last).array, (after_last).pos, size_of_arrays};
  }

  Deque_iterator<T, true> cend() {
    return { after_last.array, after_last.pos, size_of_arrays };
  }


  std::reverse_iterator<iterator> rbegin() {
    std::reverse_iterator it (end());
    return it;
  }

  std::reverse_iterator<const_iterator> rbegin() const {
    std::reverse_iterator it (cend());
    return it;
  }

  std::reverse_iterator<Deque_iterator<T, true>> crbegin() const {
    std::reverse_iterator it (cend());
    return it;
  }

  std::reverse_iterator<iterator> rend() {
    std::reverse_iterator it ((iterator(front_of_start) + 1));
    return it;
  }


  std::reverse_iterator<Deque_iterator<T, true>> rend() const {
    std::reverse_iterator it ((const_iterator(front_of_start)) + 1);
    return it;
  }

  std::reverse_iterator<Deque_iterator<T, true>> crend() {
    std::reverse_iterator it ((const_iterator(front_of_start)) + 1);
    return it;
  }

  void insert(Deque_iterator<T, false> it, const T& value) {
    if (it == end()) {
      push_back(value);
    } else {
      T replaced1_value = *(it);
      T replaced2_value = *(it);
      *it = value;
      ++it;
      for (; it < end(); ++it) {
        replaced1_value = *it;
        *it = replaced2_value;
        replaced2_value = replaced1_value;
      }
      push_back(replaced2_value);
    }
  }

  void erase(Deque_iterator<T, false> it) {
    for (; it < end() - 1; ++it) {
      *(it) = *(it + 1);
    }
    pop_back();
  }

  ~Deque() {
    for (Deque_iterator<T, false> it = begin() + 1; it < end(); ++it) {
      (*it.array + it.pos)->~T();
    }
    for (size_t k = 0; k < source.size(); ++k) {
      delete[] reinterpret_cast<char*>(source[k]);
    }
  }

};
