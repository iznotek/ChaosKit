#ifndef CHAOSKIT_STATE_HIERARCHICALSTORE_H
#define CHAOSKIT_STATE_HIERARCHICALSTORE_H

#include <unordered_map>
#include "Store.h"
#include "hierarchy.h"

namespace chaoskit::state {

/** Store that manages a predefined hierarchy of types. */
template <typename... Ts>
class HierarchicalStore : public Store<Ts...> {
  std::unordered_multimap<Id, Id> children_;
  std::unordered_multimap<Id, Id> parents_;

 public:
  template <typename P, typename C>
  void associateChildWith(Id parentId, Id childId) {
    static_assert(Store<Ts...>::template containsType<P>());
    static_assert(Store<Ts...>::template containsType<C>());
    static_assert(isValidRelation<P, C>());

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addChild()");
    }
    if (!this->template matchesType<C>(childId)) {
      throw IdTypeMismatchError("childId in HierarchicalStore::addChild()");
    }

    auto* child = const_cast<C*>(this->template find<C>(childId));
    if (!child) {
      throw MissingIdError("childId in HierarchicalStore::addChild()");
    }

    children_.emplace(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
  }

  template <typename P, typename C>
  Id associateNewChildWith(Id parentId) {
    static_assert(Store<Ts...>::template containsType<P>());
    static_assert(Store<Ts...>::template containsType<C>());
    static_assert(isValidRelation<P, C>());

    if (!this->template matchesType<P>(parentId)) {
      throw IdTypeMismatchError("parentId in HierarchicalStore::addNewChild()");
    }

    const Id childId = this->template create<C>();
    auto* child = const_cast<C*>(this->template find<C>(childId));
    children_.emplace(parentId, childId);
    parents_.emplace(childId, parentId);
    this->template update<P>(
        parentId, [child](P* parent) { associateChild(*parent, child); });
    return childId;
  }

  template <typename T>
  void remove(Id id) {
    static_assert(Store<Ts...>::template containsType<T>());
    if (!this->template matchesType<T>(id)) {
      throw IdTypeMismatchError("in HierarchicalStore::remove()");
    }

    if constexpr (isChild<T>()) {
      ParentsOf<T>::List::forEach([this, id](auto type) {
        using ParentType = std::remove_pointer_t<decltype(type)>;

        if constexpr (Store<Ts...>::template containsType<ParentType>()) {
          for (auto it = parents_.begin(); it != parents_.end();) {
            if (it->first != id ||
                !this->template matchesType<ParentType>(it->second)) {
              ++it;
              continue;
            }

            T* child = const_cast<T*>(this->template find<T>(id));
            this->template update<ParentType>(it->second,
                                              [child](ParentType* parent) {
                                                dissociateChild(*parent, child);
                                              });
            it = parents_.erase(it);
          }
        }
      });
    }

    if constexpr (isParent<T>()) {
      ChildrenOf<T>::List::forEach([this, id](auto type) {
        using ChildType = std::remove_pointer_t<decltype(type)>;

        if constexpr (Store<Ts...>::template containsType<ChildType>()) {
          // We can't use equal_range here because it might get invalidated when
          // we remove entities recursively.
          for (;;) {
            auto it = std::find_if(
                children_.begin(), children_.end(),
                [this, id](const std::pair<const Id, Id>& childPair) {
                  return childPair.first == id &&
                         this->template matchesType<ChildType>(
                             childPair.second);
                });
            if (it == children_.end()) {
              break;
            }
            remove<ChildType>(it->second);
            children_.erase(it);
          }
        }
      });
    }

    Store<Ts...>::template remove<T>(id);
  }

  // TODO: implement clear<T>()

  void clearAll() {
    parents_.clear();
    children_.clear();
    Store<Ts...>::clearAll();
  }
};

}  // namespace chaoskit::state

#endif  // CHAOSKIT_STATE_HIERARCHICALSTORE_H