#pragma once

#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>

template <typename Key, typename Value>
class LRUCache
{
public:
  explicit LRUCache(size_t capacity)
      : capacity_(capacity) {}

  bool get(const Key &key, Value &out)
  {
    std::scoped_lock lock(mutex_);
    auto it = index_.find(key);
    if (it == index_.end())
    {
      return false;
    }
    items_.splice(items_.begin(), items_, it->second);
    out = it->second->second;
    return true;
  }

  void put(const Key &key, const Value &value)
  {
    std::scoped_lock lock(mutex_);
    auto it = index_.find(key);
    if (it != index_.end())
    {
      it->second->second = value;
      items_.splice(items_.begin(), items_, it->second);
      return;
    }

    if (capacity_ == 0)
    {
      return;
    }

    if (items_.size() >= capacity_)
    {
      auto last = items_.end();
      --last;
      index_.erase(last->first);
      items_.pop_back();
    }

    items_.emplace_front(key, value);
    index_[key] = items_.begin();
  }

  bool erase(const Key &key)
  {
    std::scoped_lock lock(mutex_);
    auto it = index_.find(key);
    if (it == index_.end())
    {
      return false;
    }
    items_.erase(it->second);
    index_.erase(it);
    return true;
  }

  bool contains(const Key &key) const
  {
    std::scoped_lock lock(mutex_);
    return index_.find(key) != index_.end();
  }

  size_t size() const
  {
    std::scoped_lock lock(mutex_);
    return items_.size();
  }

  size_t capacity() const
  {
    return capacity_;
  }

private:
  size_t capacity_;
  mutable std::mutex mutex_;
  std::list<std::pair<Key, Value>> items_;
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> index_;
};