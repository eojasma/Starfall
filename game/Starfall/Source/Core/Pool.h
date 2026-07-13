#pragma once
#include <cstddef>
#include <concepts>

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

    /// <summary>
    /// parallel liveness flags
    /// </summary>
    //std::vector<bool> _active;

    std::size_t _live = 0;
};



template <class T>
    requires requires(T t) {
        { t.active } -> std::convertible_to<bool>;
    }
Pool<T>::Pool(std::size_t capacity) : _slots(capacity) //_active(capacity, false)
{
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
    //_active[i]    = true;
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
    //if (!_active[i])
    if (!_slots[i].active) 
        return;  // guard: already free — double-release is a no-op

    //_active[i] = false;
    _slots[i].active  = false;
    _free.push_back(i);  // back onto the free stack (O(1))
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
        if (_slots[i].active)//(_active[i])
            fn(_slots[i], i);
}
