#include "CircuitBreaker.hpp"

CircuitBreaker::CircuitBreaker(size_t failureThreshold,
                               size_t successThreshold,
                               std::chrono::milliseconds openTimeout)
    : failureThreshold_(failureThreshold),
      successThreshold_(successThreshold),
      openTimeout_(openTimeout) {}

bool CircuitBreaker::allowRequest()
{
  std::scoped_lock lock(mutex_);
  if (state_ == State::Open)
  {
    auto now = std::chrono::steady_clock::now();
    if (now - openedAt_ >= openTimeout_)
    {
      state_ = State::HalfOpen;
      failureCount_ = 0;
      successCount_ = 0;
      return true;
    }
    return false;
  }
  return true;
}

void CircuitBreaker::recordSuccess()
{
  std::scoped_lock lock(mutex_);
  if (state_ == State::HalfOpen)
  {
    ++successCount_;
    if (successCount_ >= successThreshold_)
    {
      state_ = State::Closed;
      failureCount_ = 0;
      successCount_ = 0;
    }
    return;
  }

  failureCount_ = 0;
}

void CircuitBreaker::recordFailure()
{
  std::scoped_lock lock(mutex_);
  if (state_ == State::HalfOpen)
  {
    state_ = State::Open;
    openedAt_ = std::chrono::steady_clock::now();
    failureCount_ = 0;
    successCount_ = 0;
    return;
  }

  ++failureCount_;
  if (failureCount_ >= failureThreshold_)
  {
    state_ = State::Open;
    openedAt_ = std::chrono::steady_clock::now();
    failureCount_ = 0;
    successCount_ = 0;
  }
}

CircuitBreaker::State CircuitBreaker::state() const
{
  std::scoped_lock lock(mutex_);
  return state_;
}

const char *CircuitBreaker::stateName(State state)
{
  switch (state)
  {
  case State::Closed:
    return "CLOSED";
  case State::Open:
    return "OPEN";
  case State::HalfOpen:
    return "HALF_OPEN";
  }

  return "UNKNOWN";
}
