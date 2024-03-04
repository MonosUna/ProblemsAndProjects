#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

template <size_t N>
class StackStorage {
 private:
  char array[N];
  size_t shift;

 public:
  StackStorage() : shift(0) {}

  StackStorage(const StackStorage& src) = delete;

  void* allocate(size_t count, size_t align) {
    if (shift + count > N) {
      throw std::bad_alloc();
    } else {
      std::size_t space = N;
      void* ptr = static_cast<char*>(array) + shift;
      std::align(align, count, ptr, space);
      shift += count;
      return ptr;
    }
  }

  StackStorage& operator=(const StackStorage<N>& src) = delete;
};

template <typename T, size_t N>

class StackAllocator {
 public:
  StackStorage<N>* storage;

 public:
  template <typename T1>
  friend bool operator==(StackAllocator<T1, N>& a, StackAllocator<T1, N>& b);
  using value_type = T;
  template <class U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };
  template <typename U>
  StackAllocator(StackAllocator<U, N> allocator) : storage(allocator.storage) {}
  StackAllocator(StackStorage<N>& src) : storage(&src) {}
  StackAllocator(const StackAllocator<T, N>& src) : storage(src.storage) {}

  StackAllocator select_on_container_copy_construction()  {
    return StackAllocator(&storage);
  }

  StackAllocator(StackAllocator& src) = default;
  StackAllocator& operator=(const StackAllocator& src) {
    storage = src.storage;
    return *this;
  }

  T* allocate(size_t count) {
    try {
      return static_cast<T*>(storage->allocate(count * sizeof(T), alignof(T)));
    } catch (...) {
      throw std::bad_alloc();
    }
  }

  void deallocate(T* ptr, size_t) {
    std::ignore = ptr;
  }

  void construct(T* ptr, T& value) { new (ptr) T(value); }

  void destroy(T* ptr) { ptr->~T(); }

};

template <typename T1, size_t N>
bool operator==(StackAllocator<T1, N>& a, StackAllocator<T1, N>& b) {
  return a.storage == b.storage;
}

template <typename T, size_t N>
bool operator!=(StackAllocator<T, N>& a, StackAllocator<T, N>& b) {
  return !(a == b);
}

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  struct BaseNode;
  struct Node : BaseNode {
    T value;
    Node(const T& value) : BaseNode(), value(value)  {}
    Node() : BaseNode(), value(T()) {}
  };

  struct BaseNode {
    BaseNode* prev;
    BaseNode* next;

    BaseNode() {
      prev = &(*this);
      next = &(*this);
    }
  };

  using NodeAlloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  size_t size_ = 0;
  NodeAlloc node_alloc_;
  BaseNode fakeNode;

 public:
  List() : fakeNode() {}

  List(Alloc allocator) : node_alloc_(allocator), fakeNode() {}

  List(size_t count, const T& value, Alloc allocator)
      :  size_(count), node_alloc_(allocator), fakeNode() {
    BaseNode* node = &fakeNode;
    size_t times = 1;
    Node* next_node;
    try {
      for (; times <= count; ++times) {
        next_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
        std::allocator_traits<NodeAlloc>::construct(node_alloc_, next_node,
                                                    value);
        node->next = next_node;
        next_node->prev = static_cast<Node*>(node);
        node = next_node;
      }
      node->next = static_cast<Node*>(&fakeNode);
      fakeNode.prev = static_cast<Node*>(node);
    } catch (...) {
      BaseNode* prev_node = node->prev;
      std::allocator_traits<NodeAlloc>::deallocate(node_alloc_,
                                                   static_cast<Node*>(node), 1);
      node = prev_node;
      for (size_t i = 1; i < times; ++i) {
        prev_node = node->prev;
        std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                                  static_cast<Node*>(node));
        std::allocator_traits<NodeAlloc>::deallocate(
            node_alloc_, static_cast<Node*>(node), 1);
        node = prev_node;
      }
      throw;
    }
  }

  List(size_t count, Alloc allocator)
      : size_(count), node_alloc_(allocator), fakeNode() {
    BaseNode* node = &fakeNode;
    size_t times = 1;
    Node* next_node;
    try {
      for (; times <= count; ++times) {
        next_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
        std::allocator_traits<NodeAlloc>::construct(node_alloc_, next_node);
        node->next = next_node;
        next_node->prev = static_cast<Node*>(node);
        node = next_node;
      }
      node->next = static_cast<Node*>(&fakeNode);
      fakeNode.prev = static_cast<Node*>(node);
    } catch (...) {
      BaseNode* prev_node;
      std::allocator_traits<NodeAlloc>::deallocate(
          node_alloc_, static_cast<Node*>(next_node), 1);
      for (size_t i = 1; i < times; ++i) {
        prev_node = node->prev;
        std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                                  static_cast<Node*>(node));
        std::allocator_traits<NodeAlloc>::deallocate(
            node_alloc_, static_cast<Node*>(node), 1);
        node = prev_node;
      }
      throw;
    }
  }

  List(size_t count, const T& value)
      : List(count, value, std::allocator<T>()) {}

  List(size_t count) : List(count, std::allocator<T>()) {}

  size_t size() const { return size_; }

  NodeAlloc get_allocator() const { return node_alloc_; }

  List(const List& other) : size_(other.size_), node_alloc_(std::allocator_traits<NodeAlloc>::select_on_container_copy_construction(other.node_alloc_)) {
    fakeNode = BaseNode();
    BaseNode* node = &fakeNode;
    size_t count = other.size();
    size_t times = 1;
    Node* next_node;
    BaseNode* other_node = other.fakeNode.next;
    try {
      for (; times <= count; ++times) {
        next_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
        std::allocator_traits<NodeAlloc>::construct(
            node_alloc_, next_node, static_cast<Node*>(other_node)->value);
        node->next = next_node;
        next_node->prev = static_cast<Node*>(node);
        node = next_node;
        other_node = other_node->next;
      }
      node->next = static_cast<Node*>(&fakeNode);
      fakeNode.prev = static_cast<Node*>(node);
    } catch (...) {
      BaseNode* prev_node = node->prev;
      std::allocator_traits<NodeAlloc>::deallocate(
          node_alloc_, static_cast<Node*>(next_node), 1);
      for (size_t i = 1; i < times; ++i) {
        prev_node = node->prev;
        std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                                  static_cast<Node*>(node));
        std::allocator_traits<NodeAlloc>::deallocate(
            node_alloc_, static_cast<Node*>(node), 1);
        node = prev_node;
      }
      throw;
    }
  }

  List& operator=(List& other) {
    if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
      node_alloc_ = other.node_alloc_;
    }
    List tmp(other);
    std::swap(size_, tmp.size_);
    std::swap(fakeNode, tmp.fakeNode);
    return *this;
  }

  ~List() {
    BaseNode* node = fakeNode.next;
    BaseNode* next_node;
    for (size_t i = 1; i <= size_; ++i) {
      next_node = node->next;
      std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                                static_cast<Node*>(node));
      std::allocator_traits<NodeAlloc>::deallocate(node_alloc_,
                                                   static_cast<Node*>(node), 1);
      node = next_node;
    }
    fakeNode = BaseNode();
  }

  void push_back(const T& value) {
    Node* new_node;
    new_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
    try {
      BaseNode* prev_node = fakeNode.prev;
      std::allocator_traits<NodeAlloc>::construct(node_alloc_, new_node, value);
      prev_node->next = new_node;
      new_node->prev = prev_node;
      fakeNode.prev = new_node;
      new_node->next = &fakeNode;
      size_++;
    } catch (...) {
      std::allocator_traits<NodeAlloc>::deallocate(node_alloc_, new_node, 1);
      throw;
    }
  }

  void push_front(const T& value) {
    Node* new_node;
    new_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
    try {
      BaseNode* next_node = fakeNode.next;
      std::allocator_traits<NodeAlloc>::construct(node_alloc_, new_node, value);
      new_node->next = next_node;
      next_node->prev = new_node;
      fakeNode.next = new_node;
      new_node->prev = &fakeNode;
      size_++;
    } catch (...) {
      std::allocator_traits<NodeAlloc>::deallocate(node_alloc_, new_node, 1);
      throw;
    }
  }

  void pop_back() {
    BaseNode* last_node = fakeNode.prev;
    BaseNode* new_last_node = last_node->prev;
    new_last_node->next = &fakeNode;
    fakeNode.prev = new_last_node;
    std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                              static_cast<Node*>(last_node));
    std::allocator_traits<NodeAlloc>::deallocate(
        node_alloc_, static_cast<Node*>(last_node), 1);
    size_--;
  }

  void pop_front() {
    BaseNode* first_node = fakeNode.next;
    BaseNode* new_first_node = first_node->next;
    new_first_node->prev = &fakeNode;
    fakeNode.next = new_first_node;
    std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                              static_cast<Node*>(first_node));
    std::allocator_traits<NodeAlloc>::deallocate(
        node_alloc_, static_cast<Node*>(first_node), 1);
    size_--;
  }

  template <bool is_const>
  struct List_Iterator {
    BaseNode* node;

    List_Iterator(const BaseNode* node1) : node(const_cast<BaseNode*>(node1)) {}
    List_Iterator(BaseNode* node1) : node(node1) {}

    List_Iterator& operator++() {
      node = node->next;
      return *this;
    }

    List_Iterator operator++(int) {
      iterator copy = *this;
      ++*this;
      return copy;
    }

    List_Iterator& operator--() {
      node = node->prev;
      return *this;
    }

    using type1 = std::conditional_t<is_const, const T&, T&>;
    using type2 = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional_t<is_const, const T, T>;
    using difference_type = int;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using reference = std::conditional_t<is_const, const T&, T&>;

    type1 operator*() { return static_cast<Node*>(node)->value; }

    const type1 operator*() const { return static_cast<Node*>(node)->value; }

    type2 operator->() { return *(static_cast<Node*>(node)->value); }

    operator List_Iterator<true>() { return List_Iterator<true>(node); }

    bool operator==(const List_Iterator& another) const = default;
    bool operator!=(const List_Iterator& another) const = default;
  };

  using iterator = List_Iterator<false>;
  using const_iterator = List_Iterator<true>;

  template <bool is_const>
  struct Reverse_List_Iterator {
    List_Iterator<is_const> it;

    Reverse_List_Iterator(BaseNode* node) : it(node) {}

    Reverse_List_Iterator(List_Iterator<is_const> it1) : it(it1) {}

    Reverse_List_Iterator& operator++() {
      --it;
      return *this;
    }

    Reverse_List_Iterator& operator--() {
      ++it;
      return *this;
    }

    using type1 = std::conditional_t<is_const, const T&, T&>;
    using type2 = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional_t<is_const, const T, T>;
    using difference_type = int;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using reference = std::conditional_t<is_const, const T&, T&>;

    type1 operator*() { return *it; }

    const type1 operator*() const { return *it; }

    type2 operator->() { return *(static_cast<Node*>(it.node)->value); }

    operator Reverse_List_Iterator<true>() {
      return Reverse_List_Iterator<true>(it);
    }

    List_Iterator<is_const> base() {
      List_Iterator<is_const> new_it = it;
      ++new_it;
      return new_it;
    }

    bool operator==(const Reverse_List_Iterator& another) const = default;
    bool operator!=(const Reverse_List_Iterator& another) const = default;
  };

  using reverse_iterator = Reverse_List_Iterator<false>;
  using const_reverse_iterator = Reverse_List_Iterator<true>;

  List_Iterator<false> begin() { return {fakeNode.next}; }

  List_Iterator<true> begin() const { return {fakeNode.next}; }

  List_Iterator<true> cbegin() { return {fakeNode.next}; }

  List_Iterator<false> end() { return {&fakeNode}; }

  const_iterator end() const { return {&fakeNode}; }

  List_Iterator<true> cend() const { return {&fakeNode}; }

  reverse_iterator rbegin() {
    reverse_iterator it(end());
    ++it;
    return it;
  }

  const_reverse_iterator rbegin() const {
    const_reverse_iterator it(cend());
    ++it;
    return it;
  }

  const_reverse_iterator crbegin() {
    const_reverse_iterator it(cend());
    ++it;
    return it;
  }

  Reverse_List_Iterator<false> rend() { return {fakeNode}; }

  Reverse_List_Iterator<true> rend() const { return {fakeNode}; }

  Reverse_List_Iterator<true> crend() { return {fakeNode}; }

  template <bool is_const>
  void insert(List_Iterator<is_const> it, const T& value) {
    Node* new_node;
    new_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
    BaseNode* old_node = it.node;
    BaseNode* prev_node = old_node->prev;
    try {
      std::allocator_traits<NodeAlloc>::construct(node_alloc_, new_node, value);
      prev_node->next = new_node;
      new_node->prev = prev_node;
      old_node->prev = new_node;
      new_node->next = old_node;
      ++size_;
    } catch (...) {
      std::allocator_traits<NodeAlloc>::deallocate(node_alloc_, new_node, 1);
      throw;
    }
  }

  template <bool is_const>
  void erase(List_Iterator<is_const> it) {
    BaseNode* node = it.node;
    BaseNode* prev_node = node->prev;
    BaseNode* next_node = node->next;
    std::allocator_traits<NodeAlloc>::destroy(node_alloc_,
                                              static_cast<Node*>(node));
    std::allocator_traits<NodeAlloc>::deallocate(node_alloc_,
                                                 static_cast<Node*>(node), 1);
    --size_;
    prev_node->next = next_node;
    next_node->prev = prev_node;
  }
};
