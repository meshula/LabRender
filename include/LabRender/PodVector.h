
#pragma once

// Define assertion handler.
#ifndef IMM_ASSERT
#include <assert.h>
#define IMM_ASSERT(_EXPR)    assert(_EXPR)
#endif

#include "string.h"
#include "stddef.h"

namespace lab
{
inline void ImmMemFree(void* p) { free(p); }
inline void* ImmMemAlloc(size_t sz) { return malloc(sz); }


// Lightweight std::vector<> like class to avoid dragging dependencies
// Does not call constructors and destructors; use with caution.
// Adapted from ImGui's ImVector, and under the ImGui license.

template<typename T>
class PodVector
{
public:
    int                         Size;
    int                         Capacity;
    T*                          Data;

    typedef T                   value_type;
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    inline PodVector()           { Size = Capacity = 0; Data = NULL; }
    inline ~PodVector()          { if (Data) ImmMemFree(Data); }

    inline bool                 empty() const                   { return Size == 0; }
    inline int                  size() const                    { return Size; }
    inline int                  capacity() const                { return Capacity; }

    inline value_type&          operator[](int i)               { IMM_ASSERT(i < Size); return Data[i]; }
    inline const value_type&    operator[](int i) const         { IMM_ASSERT(i < Size); return Data[i]; }

    inline void                 clear()                         { if (Data) { Size = Capacity = 0; ImmMemFree(Data); Data = NULL; } }
    inline iterator             begin()                         { return Data; }
    inline const_iterator       begin() const                   { return Data; }
    inline iterator             end()                           { return Data + Size; }
    inline const_iterator       end() const                     { return Data + Size; }
    inline value_type&          front()                         { IMM_ASSERT(Size > 0); return Data[0]; }
    inline const value_type&    front() const                   { IMM_ASSERT(Size > 0); return Data[0]; }
    inline value_type&          back()                          { IMM_ASSERT(Size > 0); return Data[Size - 1]; }
    inline const value_type&    back() const                    { IMM_ASSERT(Size > 0); return Data[Size - 1]; }
    inline void                 swap(PodVector<T>& rhs)          { int rhs_size = rhs.Size; rhs.Size = Size; Size = rhs_size; int rhs_cap = rhs.Capacity; rhs.Capacity = Capacity; Capacity = rhs_cap; value_type* rhs_data = rhs.Data; rhs.Data = Data; Data = rhs_data; }

    inline int                  _grow_capacity(int sz) const    { int new_capacity = Capacity ? (Capacity + Capacity/2) : 8; return new_capacity > sz ? new_capacity : sz; }

    inline void                 resize(int new_size)            { if (new_size > Capacity) reserve(_grow_capacity(new_size)); Size = new_size; }
    inline void                 resize(int new_size, const T& v){ if (new_size > Capacity) reserve(_grow_capacity(new_size)); if (new_size > Size) for (int n = Size; n < new_size; n++) Data[n] = v; Size = new_size; }
    inline void                 reserve(int new_capacity)
    {
        if (new_capacity <= Capacity)
            return;
        T* new_data = (value_type*)ImmMemAlloc((size_t)new_capacity * sizeof(T));
        if (Data)
            memcpy(new_data, Data, (size_t)Size * sizeof(T));
        ImmMemFree(Data);
        Data = new_data;
        Capacity = new_capacity;
    }

    // NB: &v cannot be pointing inside the PodVector Data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void                 push_back(const value_type& v)  { if (Size == Capacity) reserve(_grow_capacity(Size + 1)); Data[Size++] = v; }
    inline void                 pop_back()                      { IMM_ASSERT(Size > 0); Size--; }
    inline void                 push_front(const value_type& v) { if (Size == 0) push_back(v); else insert(Data, v); }

    inline iterator             erase(const_iterator it)                        { IMM_ASSERT(it >= Data && it < Data+Size); const ptrdiff_t off = it - Data; memmove(Data + off, Data + off + 1, ((size_t)Size - (size_t)off - 1) * sizeof(value_type)); Size--; return Data + off; }
    inline iterator             insert(const_iterator it, const value_type& v)  { IMM_ASSERT(it >= Data && it <= Data+Size); const ptrdiff_t off = it - Data; if (Size == Capacity) reserve(_grow_capacity(Size + 1)); if (off < (int)Size) memmove(Data + off + 1, Data + off, ((size_t)Size - (size_t)off) * sizeof(value_type)); Data[off] = v; Size++; return Data + off; }
    inline bool                 contains(const value_type& v) const             { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data++ == v) return true; return false; }
};


}
