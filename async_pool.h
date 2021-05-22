#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>


class AsyncPool {
private:
  struct Payload {
    std::mutex mutex;
    std::condition_variable cond;
    bool is_shutdown = false;
    std::queue<std::packaged_task<void()>> tasks;
  };
  std::shared_ptr<Payload> payload;

public:
  explicit AsyncPool(size_t thread_count)
      : payload(std::make_shared<Payload>()) {
    for (size_t i = 0; i < thread_count; ++i) {
      std::thread([payload_ = payload] {
        std::unique_lock<std::mutex> lck(payload_->mutex);
        while (true) {
          while (!payload_->tasks.empty()) {
            auto top_task = std::move(payload_->tasks.front());
            payload_->tasks.pop();
            lck.unlock();
            top_task();
            lck.lock();
          }
          if (payload_->is_shutdown) {
            break;
          }
          payload_->cond.wait(lck);
        }
      }).detach();
    }
  }

  AsyncPool() = default;
  AsyncPool(AsyncPool&&) = default;

  ~AsyncPool() {
    if (payload) {
      {
        std::lock_guard<std::mutex> lck(payload->mutex);
        payload->is_shutdown = true;
      }
      payload->cond.notify_all();
    }
  }
 
  template <class F>
  std::future<void> async(F&& task) {
    std::future<void> ret = do_async(std::forward<F>(task));
    payload->cond.notify_one();
    return ret;
  }

private:
  template <class F>
  std::future<void> do_async(F&& task) {
    std::lock_guard<std::mutex> lck(payload->mutex);
    return payload->tasks.emplace(std::forward<F>(task)).get_future();
  }
};
