#include <iostream>
#include <memory>

struct BaseControlBlock {
  int shared_count;
  int weak_count;

  BaseControlBlock(): shared_count(1), weak_count(0) {}

  BaseControlBlock(int a, int b): shared_count(a), weak_count(b) {}

  virtual void useDeleter() = 0;
  virtual void deallocate() = 0;
  virtual ~BaseControlBlock() = default;
};

template<typename T, typename Alloc = std::allocator<T>>
struct ControlBlockMakeShared: BaseControlBlock {
  char object[sizeof(T)];
  Alloc alloc;


  template<typename... Args>
  ControlBlockMakeShared(const Alloc& alloc_1, Args&&... args): alloc(alloc_1) {
    new (reinterpret_cast<T*>(object)) T(std::forward<Args>(args)...);
  }

  ControlBlockMakeShared(const Alloc& alloc_1): alloc(alloc_1) {}

  void useDeleter() override {
    T* ptr = reinterpret_cast<T*>(object);
    std::allocator_traits<Alloc>::destroy(alloc, ptr);
  }

  void deallocate() override {
    using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared>;
    AllocControlBlock newAlloc = alloc;
    std::allocator_traits<AllocControlBlock>::deallocate(newAlloc, this, 1);
  }
};

template<typename T, typename Deleter = std::default_delete<T>, typename Alloc = std::allocator<T>>
struct ControlBlockRegular: BaseControlBlock {
  T* object;
  Deleter deleter;
  Alloc alloc;

  ControlBlockRegular(T* object): BaseControlBlock(), object(object), deleter(Deleter()), alloc(Alloc()) {}

  ControlBlockRegular(T* object, Deleter& deleter): BaseControlBlock(),
                                                   object(object),
                                                   deleter(deleter), alloc(Alloc()) {}

  ControlBlockRegular(T* object, Deleter& deleter, Alloc& alloc_1)
      : BaseControlBlock(),
        object(object),
        deleter(deleter),
        alloc(alloc_1) {}

  template<typename U>
  ControlBlockRegular(const ControlBlockRegular<U>& other): BaseControlBlock(other.shared_count, other.weak_count),
      object(other.object), alloc(other.alloc), deleter(other.deleter) {}

  void useDeleter() override {
    deleter(object);
  }

  void deallocate() override {
    using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular>;
    AllocControlBlock newAlloc = alloc;
    std::allocator_traits<AllocControlBlock>::deallocate(newAlloc, this, 1);
  }
};

template <typename T>
class SharedPtr {
 public:
  template<typename Alloc = std::allocator<T>>
  SharedPtr(ControlBlockMakeShared<T, Alloc>* cb): ptr(reinterpret_cast<T*>(cb->object)), cb(cb) {}

  SharedPtr(T* ptr, BaseControlBlock* cb): ptr(ptr), cb(cb) {
    if (cb) {
      cb->shared_count++;
    }
  }

  T* ptr;
  BaseControlBlock* cb;

 public:

  SharedPtr(): ptr(nullptr), cb(nullptr) {}

  template<typename Deleter>
  SharedPtr(T* ptr, Deleter& deleter): ptr(ptr), cb(new ControlBlockRegular<T, Deleter>(ptr, deleter)) {}

  template<typename Alloc, typename Deleter>
  SharedPtr(T* ptr, Deleter& deleter, Alloc& alloc): ptr(ptr) {
    using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<T, Deleter, Alloc>>;
    AllocControlBlock newAlloc = alloc;
    auto cb_ptr = std::allocator_traits<AllocControlBlock>::allocate(newAlloc, 1);
    new (cb_ptr) ControlBlockRegular<T, Deleter, Alloc>(ptr, deleter, alloc);
    cb = cb_ptr;
  }

  template<typename U, typename Deleter>
  SharedPtr(U* other, Deleter deleter) {
    ptr = other;
    cb = new ControlBlockRegular<Deleter>(other, deleter);
  }
  explicit SharedPtr(T* ptr): ptr(ptr), cb(new ControlBlockRegular(ptr)) {}

  SharedPtr(const SharedPtr& other): ptr(other.ptr), cb(other.cb) {
    if (other.ptr == nullptr) return;
    cb->shared_count++;
  }

  template<typename U>
  SharedPtr(const SharedPtr<U>& other): ptr(other.ptr), cb(other.cb) {
    ptr = other.ptr;
    if (other.ptr == nullptr) return;
    cb->shared_count++;
  }

  template<typename U>
  SharedPtr(SharedPtr<U>&& other): ptr(other.ptr), cb(other.cb) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  template<typename U>
  SharedPtr(U* derived) {
    ptr = derived;
    cb = new ControlBlockRegular(ptr);
  }

  SharedPtr(SharedPtr&& other): ptr(other.ptr), cb(other.cb) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  SharedPtr& operator=(const SharedPtr& other) {
    SharedPtr copy(other);
    swap(copy);
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& other) noexcept {
    SharedPtr copy = std::move(other);
    swap(copy);
    return *this;
  }

  template<typename U>
  SharedPtr& operator=(const SharedPtr<U>& other) {
    SharedPtr copy = other;
    swap(copy);
    return *this;
  }

  template<typename U>
  SharedPtr& operator=(SharedPtr<U>&& other) {
    SharedPtr copy = std::move(other);
    swap(copy);
    return *this;
  }

  int use_count() const {
    return cb->shared_count;
  }

  void swap(SharedPtr& other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }


  ~SharedPtr() {
    if (cb == nullptr) return;
    cb->shared_count--;
    if (cb->shared_count == 0) {
      if (cb->weak_count == 0) {
        if (ptr) {
          cb->useDeleter();
        }
        cb->deallocate();
      } else {
        if (ptr) cb->useDeleter();
      }
    }
  }

  T& operator*() const {
    return *ptr;
  }

  T* operator->() const {
    return ptr;
  }

  T* get() const {
    return ptr;
  }

  void reset() noexcept {
    SharedPtr().swap(*this);
  }

  void reset(T* new_ptr){
    SharedPtr<T>(new_ptr).swap(*this);
  }
};

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
  using AllocControlBlock = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared<T, Alloc>>;
  AllocControlBlock newAlloc = alloc;

  ControlBlockMakeShared<T, Alloc>* place = std::allocator_traits<AllocControlBlock>::allocate(newAlloc, 1);
  std::allocator_traits<AllocControlBlock>::construct(newAlloc, place, alloc);

  new (reinterpret_cast<T*>(place->object)) T(std::forward<Args>(args)...);

  return SharedPtr<T>(place);
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  return allocateShared<T, std::allocator<T>, Args...>(std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {

  template<typename Y>
  friend class WeakPtr;

 public:

  WeakPtr(SharedPtr<T>& shr_ptr): cb(shr_ptr.cb), ptr(shr_ptr.ptr) {
    cb->weak_count++;
  }

  template<typename U>
  WeakPtr(SharedPtr<U>& shr_ptr): cb(shr_ptr.cb), ptr(shr_ptr.ptr) {
    cb->weak_count++;
  }

  WeakPtr(): cb(nullptr), ptr(nullptr) {}

  explicit WeakPtr(T* ptr): cb(new ControlBlockRegular(ptr)), ptr(ptr) {}

  WeakPtr(const WeakPtr& other): cb(other.cb), ptr(other.ptr) {
    if (other.ptr == nullptr) return;
    cb->weak_count++;
  }

  template<typename U>
  WeakPtr(const WeakPtr<U>& other): cb(other.cb), ptr(other.ptr) {
    if (other.ptr == nullptr) return;
    cb->weak_count++;
  }

  template<typename U>
  WeakPtr(U* derived) {
    ptr = derived;
    cb = new ControlBlockRegular(ptr);
  }

  WeakPtr(WeakPtr&& other): cb(other.cb), ptr(other.ptr) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  template<typename U>
  void swap(WeakPtr<U>& other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }

  WeakPtr& operator=(const WeakPtr&other) {
    WeakPtr copy(other);
    swap(copy);
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& other)  noexcept {
    ptr = other.ptr;
    cb = other.cb;
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  template<typename U>
  WeakPtr& operator=(const SharedPtr<U>& other) {
    if (cb == nullptr) {
      ptr = other.ptr;
      cb = other.cb;
      if (other.ptr == nullptr) return *this;
      cb->weak_count++;
      return *this;
    }
    cb->weak_count--;
    ptr = other.ptr;
    cb = other.cb;
    if (other.ptr == nullptr) return *this;
    cb->weak_count++;
    return *this;
  }

  bool expired() const noexcept {
    return cb->shared_count == 0;
  }

  int use_count() const noexcept {
    return cb->shared_count;
  }

  ~WeakPtr() {
    if (cb == nullptr) return;
    cb->weak_count--;
    if (cb->weak_count == 0 && cb->shared_count == 0) {
      cb->deallocate();
    }
  }

  SharedPtr<T> lock() const noexcept {
    return SharedPtr<T>(ptr, cb);
  }

 private:
  BaseControlBlock* cb;
  T* ptr;
};

template<typename T>
class EnableSharedFromThis {
 public:
  SharedPtr<T> shared_from_this() {
    return weak_ptr.lock();
  }

 private:
  WeakPtr<T> weak_ptr;
};
