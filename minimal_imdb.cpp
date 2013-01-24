#include "vector"
#include "string"
#include <numeric>
#include <iostream>
#include <assert.h>

using namespace std;

// A minimal impl of a main memory "database"

#include <stdio.h>
#include <stdint.h>

inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx");
    return (uint64_t)hi << 32 | lo;
}

typedef vector<size_t> sizes;
typedef unsigned char byte;

sizes partial_sum_with_leading0(sizes fields)
{
	sizes result(fields.size()+1);
  result[0] = 0;
  partial_sum(fields.begin(), fields.end()-1, result.begin()+1);
  return result;
}

class MemoryMeta
{
private:
  const size_t _width;
  const sizes _field_offsets;
  const sizes _widths;
  const size_t _rows;
  const size_t _cols;
  void * _memory;
public:
  MemoryMeta(size_t rows, sizes fields):
    _width(accumulate(fields.begin(), fields.end(), 0)),
    _field_offsets(partial_sum_with_leading0(fields)),
    _widths(fields),
    _rows(rows),
    _cols(fields.size()),
    _memory(malloc(_rows*_width))
  {
  }
  
  inline void * dataAt(size_t col, size_t row) const
  {
    return ((byte*) _memory) + row * _width + _field_offsets[col];
  }
  
  inline void dataSet(size_t col, size_t row, void * value)
  {
    memcpy(dataAt(col, row), value, _widths[col]);
  }
  
  class Iterator {
  private:
    
    size_t* _offsets;
    byte* _rowptr;
    const size_t _width;
    
  public:
    Iterator(MemoryMeta* mm) : _rowptr((byte*) mm->_memory), _width(mm->_width)
    {
      _offsets = (size_t*) malloc(mm->_cols * sizeof(size_t));
      
      size_t i=0;
      for(auto val : mm->_field_offsets)
      {
        _offsets[i++]=val;
      }
    }
    
    ~Iterator()
    {
      free(_offsets);
    }
    
    void next() 
    {
      _rowptr += _width;
    }
    
    byte* row()
    {
      return _rowptr;
    }
    
    template<typename T>
    const T& getValue(const size_t& col) const
    {
      return *(T*) (_rowptr + _offsets[col]);
    }
  };
  
  Iterator begin()
  {
        return Iterator(this);
  }

};

class Expression
{
public:
  virtual ~Expression() {};
  virtual bool operator()(const MemoryMeta::Iterator& it) const = 0;
};

template<template<typename T> class Operand>
class CompoundExpression : public Expression
{
  Expression* _left;
  Expression* _right;
  Operand<bool> op;
public:
  CompoundExpression(Expression* left, Expression* right) : _left(left), _right(right)
  {}
    
  bool inline operator()(const MemoryMeta::Iterator& it) const
  {
    return op((*_left)(it), (*_right)(it));
  }
};

template<typename Value, template<typename T> class Operand>
class CompareExpression : public Expression
{
  const Value _value;
  const size_t _column;
  Operand<Value> op;
public:
  CompareExpression(Value value, size_t column): _value(value), _column(column)
  {
  }
    
  bool inline operator()(const MemoryMeta::Iterator& it) const
  {
    return op(it.getValue<Value>(_column), _value);
  }
};


const size_t table_size = 10000000;
static size_t cnt;

int main (int /*argc*/, char ** /*argv*/)
{
  sizes fields;
  fields.push_back(sizeof(int));
  fields.push_back(sizeof(double));
  fields.push_back(sizeof(char[10]));
  MemoryMeta mm1(table_size, fields);

  int ivalue;
  double dvalue = 1.3;
  char cvalue[10] = "01234567\0";
  for(size_t i = 0; i < table_size; ++i)
  {
    ivalue = i;
    mm1.dataSet(0, i, &ivalue);
    mm1.dataSet(1, i, &dvalue);
    mm1.dataSet(2, i, &cvalue);
  }
  
  auto it = mm1.begin();
  
  auto cmp5 = new CompareExpression<int, equal_to>(5 /*value*/, 0);
  auto cmp6 = new CompareExpression<double, equal_to>(3 /*value*/, 1);
  auto comp = new CompoundExpression<logical_or>(cmp5, cmp6);
  
  auto start = rdtsc();
  for(size_t i = 0; i < table_size; ++i)
  {
    if (comp->operator()(it))
    {
      ++cnt;
    }
    it.next();
  }
  auto end = rdtsc();
  cout << end - start << endl;
  

  return 0;
};
