#include <iostream>
#include <vector>

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  struct BaseNode;
  struct Node : BaseNode {
    T value;

    Node(const T& value) : BaseNode(), value(value)  {}
    Node() : BaseNode(), value(T()) {}
    Node(T&& value) : BaseNode(), value(std::move(value)) {
      value = T();
    }
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

  void clear_list() {
    while (size_ > 0) {
      pop_back();
    }
  }

 public:

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

  List() : fakeNode() {}

  List(Alloc allocator) : node_alloc_(allocator), fakeNode() {}

  List(List&& other): size_(other.size_),
                      node_alloc_(std::move(other.node_alloc_)),
                      fakeNode() {
    other.size_ = 0;
    fakeNode.next = other.fakeNode.next;
    fakeNode.prev = other.fakeNode.prev;
    other.fakeNode.prev->next = &fakeNode;
    other.fakeNode.next->prev = &fakeNode;
    other.fakeNode.next = &other.fakeNode;
    other.fakeNode.prev = &other.fakeNode;
    node_alloc_ = std::allocator<NodeAlloc>();
  }

  template<class... Args>
  void emplace_back(Args&&... args) {
    Node* new_node;
    new_node = std::allocator_traits<NodeAlloc>::allocate(node_alloc_, 1);
    try {
      BaseNode* prev_node = fakeNode.prev;
      std::allocator_traits<NodeAlloc>::construct(node_alloc_, new_node, std::forward<Args>(args)...);
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

  List& operator=(List&& other) {
    if (&other == this) {
      return *this;
    }
    if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value || node_alloc_ == other.node_alloc_) {
      clear_list();
      fakeNode.next = other.fakeNode.next;
      fakeNode.prev = other.fakeNode.prev;
      other.fakeNode.prev->next = &fakeNode;
      other.fakeNode.next->prev = &fakeNode;
      other.fakeNode.next = &other.fakeNode;
      other.fakeNode.prev = &other.fakeNode;
      size_ = other.size_;
      other.size_ = 0;
    }
    if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value) {
      node_alloc_ = std::move(other.node_alloc_);
      return *this;
    }
    if (node_alloc_ == other.node_alloc_) {
      return *this;
    }
    if (other.size_ > size_) {
      list_iterator first_iter = begin();
      list_iterator second_iter = other.begin();
      for (size_t i = 0; i < size_; ++i, ++first_iter, ++second_iter) {
        (*first_iter) = std::move(*second_iter);
      }
      while(size_ < other.size_) {
        emplace_back(std::move(*second_iter));
        ++second_iter;
      }
    } else {
      list_iterator first_iter = begin();
      list_iterator second_iter = other.begin();
      for (int i = 0; i < other.size_; ++i, ++first_iter, ++second_iter) {
        (*first_iter) = std::move(*second_iter);
      }
      while (size_ > other.size_) {
        pop_back();
      }
    }
    return *this;
  }

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
      list_iterator copy = *this;
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

  using list_iterator = List_Iterator<false>;
  using list_const_iterator = List_Iterator<true>;

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

  list_const_iterator end() const { return {&fakeNode}; }

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

template <typename Key, typename Value, typename Hash = std::hash<Key>,
    typename Equal = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using NodeType = std::pair<Key, Value>;

 public:

  template <bool is_const>
  struct UnorderedMap_Iterator;

  using iterator = UnorderedMap_Iterator<false>;
  using const_iterator = UnorderedMap_Iterator<true>;;

  using list_iterator = typename List<NodeType*, Alloc>::list_iterator;
  using list_const_iterator = typename List<NodeType*, Alloc>::list_const_iterator;

  template <bool is_const>
  struct UnorderedMap_Iterator {
    typename List<NodeType*, Alloc>::template List_Iterator<is_const> iter;

    UnorderedMap_Iterator (list_iterator itr): iter(itr) {}

    UnorderedMap_Iterator (list_const_iterator itr): iter(itr) {}

    UnorderedMap_Iterator& operator++() {
      ++iter;
      return *this;
    }

    iterator operator++(int) {
      iterator copy = *this;
      ++*this;
      return copy;
    }

    using type1 = std::conditional_t<is_const, const NodeType&, NodeType&>;
    using type2 = std::conditional_t<is_const, const NodeType*, NodeType*>;

    using iterator_category = std::forward_iterator_tag;
    using value_type = std::conditional_t<is_const, const NodeType, NodeType>;
    using difference_type = int;
    using pointer = std::conditional_t<is_const, const NodeType*, NodeType*>;
    using reference = std::conditional_t<is_const, const NodeType&, NodeType&>;

    type1 operator*() { return *(*iter); }

    const type1 operator*() const { return *(*iter); }

    type2 operator->() { return (*iter); }

    operator const_iterator() { return const_iterator(iter); }

    bool operator==(const UnorderedMap_Iterator& another) const = default;

    bool operator!=(const UnorderedMap_Iterator& another) const = default;
  };

  UnorderedMap_Iterator<false> begin() { return {list_.begin()}; }

  const_iterator begin() const { return {list_.begin()}; }

  UnorderedMap_Iterator<true> cbegin() { return {list_.begin()}; }

  UnorderedMap_Iterator<false> end() { return { list_.end() }; }

  UnorderedMap_Iterator<true> end() const { return {list_.end()}; }

  UnorderedMap_Iterator<true> cend() const { return {list_.end()}; }

 private:

  using NodeTypeAlloc =
      typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>;

  using backet = std::vector<list_iterator>;
  using backet_array = std::vector<backet>;

  List<NodeType*, Alloc> list_;
  size_t buckets_size_;
  backet_array buckets_;
  float load_factor_ = 1.0;
  Hash hash_function_;
  Equal equal_;
  NodeTypeAlloc node_alloc_;

 public:

  UnorderedMap(): list_(), buckets_size_(8),
                  buckets_(backet_array(8, backet())),
                  hash_function_(), equal_(), node_alloc_() {}

  UnorderedMap(const UnorderedMap& other) : buckets_size_(other.buckets_size_),
  buckets_(other.buckets_), load_factor_(other.load_factor_), hash_function_(other.hash_function_),
  equal_(other.equal_), node_alloc_(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.node_alloc_)) {
    list_const_iterator iter = other.list_.begin();
    for (; iter != other.list_.end(); ++iter) {
      NodeType* object = std::allocator_traits<NodeTypeAlloc>::allocate(node_alloc_, 1);
      std::allocator_traits<NodeTypeAlloc>::construct(node_alloc_, object, *(*iter));
      list_.emplace_back(object);
    }
    for (auto &bucket : buckets_) {
      bucket.clear();
    }
    for (list_iterator it = list_.begin(); it != list_.end(); ++it) {
      size_t hash = hash_function_((*it)->first) % buckets_size_;
      buckets_[hash].push_back(it);
    }
  }

  UnorderedMap(UnorderedMap&& other) : list_(std::move(other.list_)), buckets_size_(other.buckets_size_),
                                       buckets_(std::move(other.buckets_)), load_factor_(other.load_factor_), hash_function_(std::move(other.hash_function_)),
                                       equal_(std::move(other.equal_)), node_alloc_(std::move(other.node_alloc_)) {
  }

  ~UnorderedMap() {
    for (list_iterator it = list_.begin(); it != list_.end(); ++it) {
      std::allocator_traits<NodeTypeAlloc>::destroy(node_alloc_,(*it));
      std::allocator_traits<NodeTypeAlloc>::deallocate(node_alloc_, *it, 1);
    }
  }
  UnorderedMap& operator=(const UnorderedMap& other) {
    if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
      node_alloc_ = other.node_alloc_;
    }
    UnorderedMap tmp(other);
    std::swap(buckets_size_, tmp.buckets_size_);
    std::swap(buckets_, tmp.buckets_);
    std::swap(list_, tmp.list_);
    std::swap(hash_function_, tmp.hash_function_);
    std::swap(equal_, tmp.equal_);
    std::swap(load_factor_, tmp.load_factor_);
    return *this;
  }

  UnorderedMap& operator=(UnorderedMap&& other) {
    for (list_iterator it = list_.begin(); it != list_.end(); ++it) {
      std::allocator_traits<NodeTypeAlloc>::destroy(node_alloc_,(*it));
      std::allocator_traits<NodeTypeAlloc>::deallocate(node_alloc_, *it, 1);
    }
    buckets_size_= other.buckets_size_;
    buckets_ = other.buckets_;
    load_factor_ = other.load_factor_;
    list_ = std::move(other.list_);
    hash_function_ = std::move(other.hash_function_);
    equal_ = std::move(other.equal_);
    if (std::allocator_traits<NodeTypeAlloc>::propagate_on_container_copy_assignment::value) {
      node_alloc_ = other.node_alloc_;
      return *this;
    }
    if (std::allocator_traits<NodeTypeAlloc>::propagate_on_container_move_assignment::value) {
      node_alloc_ = std::move(node_alloc_);
      return *this;
    }
    if (std::allocator_traits<NodeTypeAlloc>::propagate_on_container_swap::value) {
      std::swap(node_alloc_, other.node_alloc_);
    }
    return *this;
  }

  size_t size() const {
    return list_.size();
  }

  void rehash() {
    if (load_factor() > load_factor_) {
      backet_array new_array(2 * buckets_size_);
      for (list_iterator it = list_.begin(); it != list_.end(); ++it) {
        size_t new_hash = hash_function_((*it)->first) % (2*buckets_size_);
        new_array[new_hash].push_back(it);
      }
      buckets_ = new_array;
      buckets_size_ = buckets_.size();
    }
  }

  void rehash(size_t sz) {
    if (sz <= buckets_size_) {
      throw std::logic_error("");
    }
    if (load_factor() > load_factor_) {
      backet_array new_array(sz);
      for (list_iterator it = list_.begin(); it != list_.end(); ++it) {
        size_t new_hash = hash_function_((*it)->first) % (sz);
        new_array[new_hash].push_back(it);
      }
      buckets_ = new_array;
      buckets_size_ = buckets_.size();
    }
  }

  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    NodeType* object = std::allocator_traits<NodeTypeAlloc>::allocate(node_alloc_, 1);
    std::allocator_traits<NodeTypeAlloc>::construct(node_alloc_, object,  std::forward<Args>(args)...);
    size_t hash = hash_function_(object->first) % buckets_size_;
    size_t bucket_size = buckets_[hash].size();
    if (bucket_size == 0) {
      list_.emplace_back(object);
      list_iterator ans = --list_.end();
      buckets_[hash].push_back(ans);
      rehash();
      return {iterator(ans), true};
    }
    size_t i = 0;
    list_iterator start = buckets_[hash][i];
    while (i < bucket_size && !equal_((*start)->first, object->first)) {
      start = buckets_[hash][i];
      ++i;
    }
    if (equal_((*start)->first, object->first)) {
      std::allocator_traits<NodeTypeAlloc>::destroy(node_alloc_,object);
      std::allocator_traits<NodeTypeAlloc>::deallocate(node_alloc_, object, 1);
      return {start, false};
    }
    else {
      list_.emplace_back(object);
      list_iterator ans = --list_.end();
      buckets_[hash].push_back(ans);
      rehash();
      return {ans, true};
    }
  }

  std::pair<iterator, bool> insert(const NodeType& object) {
    return emplace(object);
  }

  std::pair<iterator, bool> insert(NodeType&& object) {
    return emplace(std::move(object));
  }

  template<class str>
  std::pair<iterator, bool> insert(str&& object) {
    return emplace(std::forward<str>(object));
  }

  Value& operator[](const Key& key) {
    size_t hash = hash_function_(key) % buckets_size_;
    size_t sz_of_hash_bct = buckets_[hash].size();
    if (sz_of_hash_bct == 0) {
      insert({key, Value()});
      return buckets_[hash][0]->second;
    }
    size_t i = 0;
    list_iterator start = buckets_[hash][i];
    while ( i < sz_of_hash_bct && !equal_((*start)->first, key)) {
      ++start;
      ++i;
    }
    if (i == sz_of_hash_bct) {
      insert({key, Value()});
      return buckets_[hash][buckets_[hash].size() - 1]->second;
    }
    else {
      return (*start)->second;
    }
  }

  Value& operator[](Key&& key) {
    size_t hash = hash_function_(key) % buckets_size_;
    size_t sz_of_hash_bct = buckets_[hash].size();
    if (sz_of_hash_bct == 0) {
      emplace(std::move(key), std::move(Value()));
      return (*buckets_[hash][0])->second;
    }
    size_t i = 0;
    list_iterator start = buckets_[hash][i];
    while ( i < sz_of_hash_bct && !equal_((*start)->first, key)) {
      ++start;
      ++i;
    }
    if (i == sz_of_hash_bct) {
      NodeType* object = std::allocator_traits<NodeTypeAlloc>::allocate(node_alloc_, 1);
      std::allocator_traits<NodeTypeAlloc>::construct(node_alloc_, object,  key, Value());
      insert(std::move(*object));
      std::allocator_traits<NodeTypeAlloc>::deallocate(node_alloc_, object, 1);
      return (*buckets_[hash][buckets_[hash].size() - 1])->second;
    }
    else {
      return (*start)->second;
    }
  }

  Value& at(const Key& key) {
    size_t hash = hash_function_(key) % buckets_size_;
    size_t sz_of_hash_bct = buckets_[hash].size();
    if (sz_of_hash_bct == 0) {
      throw std::out_of_range("out");
    }
    size_t i = 0;
    list_iterator start = buckets_[hash][i];
    while (i < sz_of_hash_bct && !equal_((*start)->first, key)) {
      ++i;
      ++start;
    }
    if (i == sz_of_hash_bct) {
      throw std::out_of_range("out");
    }
    else {
      return (*start)->second;
    }
  }

  template< class InputIt >
  void insert(InputIt first, InputIt last) {
    while (first != last) {
      insert(*first);
      ++first;
    }
  }

  void erase(const_iterator itr) {
    list_const_iterator list_itr = itr.iter;
    size_t hash = hash_function_((*list_itr)->first) % buckets_size_;
    for (size_t i = 0; i < buckets_[hash].size(); ++i) {
      if (equal_((*buckets_[hash][i])->first, (*list_itr)->first)) {
        buckets_[hash].erase(buckets_[hash].begin() + i);
      }
    }
    std::allocator_traits<NodeTypeAlloc>::destroy(node_alloc_,*(list_itr));
    std::allocator_traits<NodeTypeAlloc>::deallocate(node_alloc_, *list_itr, 1);
    list_.erase(list_itr);
  }

  void erase(iterator start, iterator finish) {
    iterator copy = start;
    while (start != finish) {
      ++start;
      erase(copy);
      copy = start;
    }
  }

  iterator find(const Key& key) {
    try {
      at(key);
    } catch (...) {
      return end();
    }
    size_t hash = hash_function_(key) % buckets_size_;
    for (list_iterator i : buckets_[hash]) {
      if (equal_((*i)->first, key)) {
        return iterator {i};
      }
    }
    return end();
  }


  void reserve(size_t new_size) {
    if (new_size <= buckets_size_) {
      return;
    } else {
      rehash(new_size);
    }
  }

  float load_factor() const {
    return static_cast<float>(size()) / static_cast<float>(buckets_size_);
  }

  float max_load_factor() const {
    return load_factor_;
  }

  void max_load_factor(float new_load_factor) {
    load_factor_ = new_load_factor;
    rehash(static_cast<size_t>(new_load_factor * buckets_size_) + 1);
  }

};
