#pragma once

#include <functional>
#include <unordered_set>

template <typename... Args>
class Observer;
template <typename... Args>
class Subject;

template <typename... Args>
class Subject {
  public:
    Subject(const Subject &) = delete;
    Subject(Subject &&) = delete;
    Subject &operator=(const Subject &) = delete;
    Subject &operator=(Subject &&) = delete;

    Subject() = default;
    ~Subject() = default;

    void Add(Observer<Args...> *observer) {
        _observers.emplace(observer);
    };
    void Remove(Observer<Args...> *observer) {
        if (_observers.contains(observer)) {
            _observers.erase(observer);
        }
    };

    void Notify(Args... args) const {
        for (auto observer : _observers) {
            observer(std::forward<Args>(args)...);
        }
    };

  private:
    std::unordered_set<Observer<Args...> *> _observers;
};

template <typename... Args>
class Observer {
  public:
    Observer(const Observer &) = delete;
    Observer(Observer &&) = delete;
    Observer &operator=(const Observer &) = delete;
    Observer &operator=(Observer &&) = delete;

    Observer() = default;
    ~Observer() = default;

    template <class T, typename... Args>
    Observer(T *inst, void (T::*func)(Args...)) : _inst(inst), _func(func){};

    void operator()(Args... args) {
        (T->*_func)(std::forward<Args>(args)...);
    }

  private:
    T *_inst;
    void (T::*_func)(Args...);
};
