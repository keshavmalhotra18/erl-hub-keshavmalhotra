// Unit IV: generic class template PriorityDispatchQueue<T, Compare> built on
// std::vector (binary heap via <algorithm>), plus custom functor comparators.
#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Exceptions.hpp"

namespace ReliefOps {
namespace Logistics {

template <typename T, typename Compare = std::less<T>>
class PriorityDispatchQueue {
public:
    explicit PriorityDispatchQueue(Compare cmp = Compare()) : heap_(), cmp_(std::move(cmp)) {}

    void push(const T& value) {
        heap_.push_back(value);
        std::push_heap(heap_.begin(), heap_.end(), cmp_);
    }
    void push(T&& value) {
        heap_.push_back(std::move(value));
        std::push_heap(heap_.begin(), heap_.end(), cmp_);
    }
    const T& top() const {
        if (heap_.empty()) throw ErlException("PriorityDispatchQueue::top on empty queue");
        return heap_.front();
    }
    T pop() {
        if (heap_.empty()) throw ErlException("PriorityDispatchQueue::pop on empty queue");
        std::pop_heap(heap_.begin(), heap_.end(), cmp_);
        T value = std::move(heap_.back());
        heap_.pop_back();
        return value;
    }
    bool empty() const noexcept { return heap_.empty(); }
    std::size_t size() const noexcept { return heap_.size(); }

    // Copy of the contents ordered from most to least urgent (non-destructive).
    std::vector<T> orderedSnapshot() const {
        std::vector<T> copy = heap_;
        std::sort_heap(copy.begin(), copy.end(), cmp_);  // ascending w.r.t. cmp_
        std::reverse(copy.begin(), copy.end());
        return copy;
    }

private:
    std::vector<T> heap_;
    Compare cmp_;
};

struct DispatchRequest {
    std::string requestId;
    std::string itemId;
    int quantity = 0;
    int triage = 1;            // 1..5
    double hoursWaiting = 0.0;

    double urgencyScore() const noexcept { return triage * 10.0 + hoursWaiting; }
};

// Functor: heap ordering - "a is less urgent than b" so the most urgent sits on top.
struct LessUrgent {
    bool operator()(const DispatchRequest& a, const DispatchRequest& b) const noexcept {
        return a.urgencyScore() < b.urgencyScore();
    }
};

// Functor for std::sort: most urgent medical need first; ties by request id.
struct MostUrgentFirst {
    bool operator()(const DispatchRequest& a, const DispatchRequest& b) const noexcept {
        if (a.urgencyScore() != b.urgencyScore()) return a.urgencyScore() > b.urgencyScore();
        return a.requestId < b.requestId;
    }
};

}  // namespace Logistics
}  // namespace ReliefOps
