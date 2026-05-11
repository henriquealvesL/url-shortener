#pragma once

#include <chrono>
#include <mutex>

class CircuitBreaker
{
public:
  enum class State
  {
    Closed,
    Open,
    HalfOpen
  };

  CircuitBreaker(size_t failureThreshold,
                 size_t successThreshold,
                 std::chrono::milliseconds openTimeout);

  bool allowRequest();
  void recordSuccess();
  void recordFailure();

  State state() const;
  static const char *stateName(State state);

private:
  size_t failureThreshold_;
  size_t successThreshold_;
  std::chrono::milliseconds openTimeout_;

  mutable std::mutex mutex_;
  State state_{State::Closed};
  size_t failureCount_{0};
  size_t successCount_{0};
  std::chrono::steady_clock::time_point openedAt_{};
};
