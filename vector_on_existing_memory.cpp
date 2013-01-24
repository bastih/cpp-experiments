#include <vector>
#include <iostream>
#include<cstdlib>

// creating a std::vector on top of already existing memory

template<typename T>
class FixedAlloc {
 public:
  typedef T        value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T&       reference;
  typedef const T& const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  const pointer _store;
  const size_t _size;

  FixedAlloc(pointer store, size_t size) : _store(store), _size(size) {}

  template <class U>
  struct rebind {
    typedef FixedAlloc<U> other;
  };

  size_type max_size() const {
    return _size;
  }

  pointer allocate (size_t num, const void* = 0) {
    return _store;
  }

  void deallocate (pointer p, size_t num) {
    // do nothing
  }

  void construct (pointer p, const T& value) {
    // do nothing, alread initialized
  }

  void destroy (pointer p) {
    // do nothing, alread initialized
  }
};

int main(int argc, char *argv[])
{
  int* area = (int*) std::calloc(10, sizeof(int));
  for (size_t a=0; a < 10; ++a) {
    area[a] = a;
  }

  FixedAlloc<int> fa(area, 10);
  std::vector<int, FixedAlloc<int> > vec(10, 0, fa);

  for(const auto& i: vec) {
    std::cout << i << std::endl;
  }
  return 0;
}
