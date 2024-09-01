#pragma once

#include <pch.h>

class EventSystem {
   public:
    using EventID = std::string;

    template <typename... Args>
    using Callback = std::function<void(Args...)>;

    template <typename... Args>
    static void RegisterListener(const EventID& event,
                                 Callback<Args...> listener) {
        auto& listeners = getListeners<Args...>(event);
        listeners.push_back(listener);
    }

    template <typename... Args>
    static void TriggerEvent(const EventID& event, Args... args) {
        auto& listeners = getListeners<Args...>(event);
        for (auto& listener : listeners) {
            listener(args...);
        }
    }

    template <typename... Args>
    static void ClearEvent(const EventID& event) {
        listeners<Args...>().erase(event);
    }

    static void ClearAll() { listenersMap().clear(); }

   private:
    struct IListenersBase {
        virtual ~IListenersBase() = default;
    };

    template <typename... Args>
    struct Listeners : IListenersBase {
        std::vector<Callback<Args...>> callbacks;
    };

    template <typename... Args>
    static std::vector<Callback<Args...>>& getListeners(const EventID& event) {
        auto& basePtr = listeners<Args...>()[event];
        if (!basePtr) {
            basePtr = std::make_unique<Listeners<Args...>>();
        }
        return static_cast<Listeners<Args...>*>(basePtr.get())->callbacks;
    }

    template <typename... Args>
    static std::unordered_map<EventID, std::unique_ptr<IListenersBase>>&
    listeners() {
        static std::unordered_map<EventID, std::unique_ptr<IListenersBase>> map;
        return map;
    }

    static std::unordered_map<
        std::string,
        std::unordered_map<EventID, std::unique_ptr<IListenersBase>>>&
    listenersMap() {
        static std::unordered_map<
            std::string,
            std::unordered_map<EventID, std::unique_ptr<IListenersBase>>>
            map;
        return map;
    }
};
