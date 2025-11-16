#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>

#include "mbase/public/access.h"

#if defined(__clang__) && (!defined(SWIG))
# define MBASE_THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
# define MBASE_THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op
#endif

#define MBASE_CAPABILITY(x) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define MBASE_SCOPED_CAPABILITY MBASE_THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define MBASE_GUARDED_BY(x) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define MBASE_PT_GUARDED_BY(x) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define MBASE_ACQUIRED_BEFORE(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define MBASE_ACQUIRED_AFTER(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define MBASE_REQUIRES(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define MBASE_REQUIRES_SHARED(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define MBASE_ACQUIRE(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define MBASE_ACQUIRE_SHARED(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define MBASE_RELEASE(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define MBASE_RELEASE_SHARED(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define MBASE_TRY_ACQUIRE(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define MBASE_TRY_ACQUIRE_SHARED(...) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define MBASE_EXCLUDES(...) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define MBASE_ASSERT_CAPABILITY(x) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define MBASE_ASSERT_SHARED_CAPABILITY(x) \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define MBASE_RETURN_CAPABILITY(x) MBASE_THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define MBASE_NO_THREAD_SAFETY_ANALYSIS \
  MBASE_THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

namespace mbase {

/// Wrapper for `Lockable` types with TSA support.
template<class T>
class MBASE_CAPABILITY("mutex") Lockable {
public:
  Lockable() = default;
  ~Lockable() = default;
  MBASE_DISALLOW_COPY(Lockable);

  void lock() MBASE_ACQUIRE() {
    lockable_.lock();
  }
  void unlock() MBASE_RELEASE() {
    lockable_.unlock();
  }
  bool try_lock() MBASE_TRY_ACQUIRE(true) {
    return lockable_.try_lock();
  }

private:
  T lockable_;
};

/// Wrapper for `SharedLockable` types with TSA support.
template<class T>
class MBASE_CAPABILITY("mutex") SharedLockable {
public:
  SharedLockable() = default;
  ~SharedLockable() = default;
  MBASE_DISALLOW_COPY(SharedLockable);

  void lock() MBASE_ACQUIRE_SHARED() {
    lockable_.lock();
  }
  void unlock() MBASE_RELEASE() {
    lockable_.unlock();
  }
  bool try_lock() MBASE_TRY_ACQUIRE_SHARED(true) {
    return lockable_.try_lock();
  }

  void lock_shared() MBASE_ACQUIRE_SHARED() {
    lockable_.lock_shared();
  }
  void unlock_shared() MBASE_RELEASE() {
    lockable_.unlock_shared();
  }
  bool try_lock_shared() MBASE_TRY_ACQUIRE_SHARED(true) {
    return lockable_.try_lock_shared();
  }

private:
  T lockable_;
};

/// `std::lock_guard` but with TSA support.
template<class Mutex>
class MBASE_SCOPED_CAPABILITY LockGuard {
public:
  explicit LockGuard(Mutex& mutex) MBASE_ACQUIRE(mutex) :
      mutex_(mutex),
      owned_(true)
  {
    mutex_.lock();
  }
  LockGuard(Mutex& mutex, std::adopt_lock_t) MBASE_REQUIRES(mutex) :
    mutex_(mutex),
    owned_(false)
  {
    // Calling thread owns mutex lock.
  }

  ~LockGuard() MBASE_RELEASE() {
    if (owned_) {
      mutex_.unlock();
    }
  }

  MBASE_DISALLOW_COPY(LockGuard);

private:
  Mutex& mutex_;
  bool owned_;
};

/// `std::shared_lock` but with TSA support.
template<class Mutex>
class MBASE_SCOPED_CAPABILITY SharedLockGuard {
public:
  explicit SharedLockGuard(Mutex& mutex) MBASE_ACQUIRE_SHARED(mutex) :
      mutex_(mutex),
      owned_(true)
  {
    mutex_.lock_shared();
  }
  SharedLockGuard(Mutex& mutex, std::adopt_lock_t) MBASE_REQUIRES_SHARED(mutex) :
    mutex_(mutex),
    owned_(false)
  {
    // Calling thread owns mutex lock.
  }

  SharedLockGuard(SharedLockGuard&& rhs) MBASE_NO_THREAD_SAFETY_ANALYSIS :
  mutex_(rhs.mutex_),
    owned_(rhs.owned_)
  {
    rhs.owned_ = false; // Transfer ownership.
  }

  ~SharedLockGuard() MBASE_RELEASE() {
    if (owned_) {
      mutex_.unlock_shared();
    }
  }

  MBASE_DISALLOW_COPY(SharedLockGuard);

private:
  Mutex& mutex_;
  bool owned_;
};

template<class Mutex>
class MBASE_SCOPED_CAPABILITY Lock {
public:
  explicit Lock(Mutex& mutex) MBASE_ACQUIRE(mutex) :
      mutex_(mutex),
      owned_(true)
  {
    mutex_.lock();
  }
  Lock(Mutex& mutex, std::adopt_lock_t) MBASE_REQUIRES(mutex) :
    mutex_(mutex),
    owned_(false)
  {
    // Calling thread owns mutex lock.
  }

  ~Lock() MBASE_RELEASE() {
    if (owned_) {
      mutex_.unlock();
    }
  }

  MBASE_DISALLOW_COPY(Lock);

  void lock() MBASE_ACQUIRE() {
    if (!owned_) {
      mutex_.lock();
      owned_ = true;
    }
  }
  void unlock() MBASE_RELEASE() {
    if (owned_) {
      mutex_.unlock();
      owned_ = false;
    }
  }

  template<class Predicate>
  void wait(
    std::condition_variable_any& cv,
    Predicate&& predicate
  ) MBASE_REQUIRES(this) {
    cv.wait(mutex_, std::forward<Predicate>(predicate));
  }

  template<class Rep, class Period>
  bool wait_for(
    std::condition_variable_any& cv,
    std::chrono::duration<Rep, Period> const& rel_time
  ) MBASE_REQUIRES(this) {
    return cv.wait_for(mutex_, rel_time);
  }

  template<class Rep, class Period, class Predicate>
  bool wait_for(
    std::condition_variable_any& cv,
    std::chrono::duration<Rep, Period> const& rel_time,
    Predicate&& predicate
  ) MBASE_REQUIRES(this) {
    return cv.wait_for(mutex_, rel_time, std::forward<Predicate>(predicate));
  }

private:
  Mutex& mutex_;
  bool owned_;
};

template<class Mutex>
class MBASE_SCOPED_CAPABILITY ScopedLockAssertion {
public:
  ScopedLockAssertion(Mutex& mutex) MBASE_ACQUIRE(mutex) {}
  ~ScopedLockAssertion() MBASE_RELEASE() {}
};

} // namespace mbase
