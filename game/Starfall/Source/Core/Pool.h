#pragma once
#include <cstdint>
#include <cstddef>
#include <concepts>

struct Handle
{
    std::uint32_t index                  = 0;
    std::uint32_t generation             = 0;  // 0 == never-valid sentinel
    bool operator==(const Handle&) const = default;
};

template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    } //requiring an active flag so that the itteration over alive entities only, the entities liveness is handled by it and the game logic
class Pool
{
public:
    explicit Pool(std::size_t capacity);

    /// <summary>
    /// returns a pointer to an inactive slot or nullptr if exhausted.
    /// Caller reactivates/initializes the T; the pool just tracks liveness
    /// </summary>
    /// <returns>T* Inactive T</returns>
    T* acquire();

    /// <summary>
    /// marks a slot free for reuse. Precondition: p came from this pool.
    /// </summary>
    /// <param name="p">T* to be returned to the pool</param>
    void release(T* p);

    /// <summary>
    /// Visit every currently active slot (for update/render/collision sweeps)
    /// </summary>
    template <class Fn> void forEachActive(Fn&& fn);

    std::size_t liveCount() const { return _live; }
    std::size_t capacity() const { return _slots.size(); }


    // pointer -> stable id, pairing the slot with its current generation
    Handle handleOf(const T* p) const;

    // id -> pointer, or nullptr if that occupant is gone
    T* resolve(Handle h);


    //we disable these so that pool can never be passed by value
    Pool(const Pool&)            = delete;
    Pool& operator=(const Pool&) = delete;

private:
    /// <summary>
    /// storage - allocated once, never grows
    /// </summary>
    std :: vector<T> _slots;

    /// <summary>
    /// stack of free indices
    /// </summary>
    std::vector<std::size_t> _free;

    std::vector<std::uint32_t> _generation;  // sized to capacity, starts at 1

    std::size_t _live = 0;
};



template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
Pool<T>::Pool(std::size_t capacity) : _slots(capacity)
{
    _generation.assign(capacity, 1);
    _free.reserve(capacity);

    for (std::size_t i = capacity; i-- > 0;) // fill high→low so acquire() hands out 0, 1, 2…
        _free.push_back(i);
}

template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
T* Pool<T>::acquire()
{
    if (_free.empty()) //free-list empty == pool exhausted; caller must handle null.
    {
        return nullptr;
    }

    std::size_t i = _free.back();
    _free.pop_back();

    _slots[i].active = true;
    ++_live;

    return &_slots[i];
}

template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
void Pool<T>::release(T* p)
{
    if (!p)
    {
        return;
    }


     // Pointer → index. p points inside _slots, so the offset from the front is its index.
    std::size_t i = static_cast<std::size_t>(p - _slots.data());

    AXASSERT(i < _slots.size(), "release: pointer not from this pool");
 
    if (!_slots[i].active) 
        return;  // guard: already free — double-release is a no-op

  
    _slots[i].active  = false;
    _free.push_back(i);  // back onto the free stack (O(1))
    ++_generation[i];    // every outstanding Handle to slot i is now stale
    --_live;
}

template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
template <class Fn>
void Pool<T>::forEachActive(Fn&& fn)
{
    for (std::size_t i = 0; i < _slots.size(); ++i)
        if (_slots[i].active)
            fn(_slots[i], i);
}


template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
    // pointer -> stable id, pairing the slot with its current generation
Handle Pool<T>::handleOf(const T* p) const
{
    auto i = static_cast<std::uint32_t>(p - _slots.data());
    return {i, _generation[i]};
}

template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
// id -> pointer, or nullptr if that occupant is gone
T* Pool<T>::resolve(Handle h)
{
    if (h.index >= _slots.size())
        return nullptr;  // bounds
    if (h.generation != _generation[h.index])
        return nullptr;  // reused since
    if (!_slots[h.index].active)
        return nullptr;  // freed, not reused
    return &_slots[h.index];
}
