#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Store.h"

#include <gmock/gmock.h>

#include "Id.h"

using chaoskit::state::Id;
using chaoskit::state::IdTypeMismatchError;
using chaoskit::state::MissingIdError;
using chaoskit::state::Store;
using testing::AllOf;
using testing::ElementsAre;
using testing::Eq;
using testing::Field;
using testing::Ne;
using testing::NotNull;
using testing::Pair;
using testing::Pointee;

struct Simple {
  int property = 0;

  bool operator==(const Simple &other) const {
    return property == other.property;
  }
};
struct Another {
  float field = 0;
};

class StoreTest : public ::testing::Test {};

template <typename... Ts>
void triggerStoreException(Store<Ts...> &store) {
  Id invalid;
  store.remove(invalid);
}

// Store::create()

TEST_F(StoreTest, CreatesIdsForEntities) {
  Store<Simple> store;

  Id id = store.create<Simple>();

  Id expected{1, 1};
  EXPECT_EQ(expected, id);
}

TEST_F(StoreTest, AssignsUniqueIdsWithinType) {
  Store<Simple> store;

  Id one = store.create<Simple>();
  Id two = store.create<Simple>();

  EXPECT_NE(one, two);
}

TEST_F(StoreTest, AssignsUniqueTypesAcrossTypes) {
  Store<Simple, Another> store;

  Id one = store.create<Simple>();
  Id two = store.create<Another>();

  EXPECT_NE(one, two);
}

TEST_F(StoreTest, AllowsToModifyObjectAfterCreation) {
  Store<Simple> store;

  Id id = store.create<Simple>([](Simple *simple) { simple->property = 42; });

  EXPECT_THAT(store.find<Simple>(id),
              Pointee(Field(&Simple::property, Eq(42))));
}

// Store::find()

TEST_F(StoreTest, FindsEntities) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  const auto *entity = store.find<Simple>(id);

  EXPECT_THAT(entity, NotNull());
}

TEST_F(StoreTest, DoesNotFindEntitiesForInvalidIds) {
  Store<Simple> store;
  Id invalid = store.lastId<Simple>();
  store.create<Simple>();

  const auto *entity = store.find<Simple>(invalid);

  EXPECT_EQ(nullptr, entity);
}

TEST_F(StoreTest, ThrowsWhenFindingDifferentTypeThanStoredInId) {
  Store<Simple> store;
  Id id{0, 0};

  EXPECT_THROW(store.find<Simple>(id), IdTypeMismatchError);
}

// Store::lastId()

TEST_F(StoreTest, ReturnsLastId) {
  Store<Simple> store;
  store.create<Simple>();
  Id second = store.create<Simple>();

  Id last = store.lastId<Simple>();

  EXPECT_EQ(second, last);
}

// Store::isValid()

TEST_F(StoreTest, RecognizesInvalidIds) {
  Store<Simple> store;
  Id invalid{0, 0};

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesZeroIdsAsInvalid) {
  Store<Simple> store;
  Id invalid = store.lastId<Simple>();

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesWrongTypesInIds) {
  Store<Simple> store;
  Id invalid{2137, 42};

  EXPECT_FALSE(store.isValid(invalid));
}

TEST_F(StoreTest, RecognizesValidIds) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  EXPECT_TRUE(store.isValid(id));
}

// Store::update()

TEST_F(StoreTest, UpdatesFields) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  store.update<Simple>(id, [](Simple *s) { s->property = 42; });
  auto *entity = store.find<Simple>(id);

  EXPECT_THAT(entity, Pointee(Field(&Simple::property, Eq(42))));
}

TEST_F(StoreTest, UpdateReturnsWhatTheUpdaterReturns) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  int result = store.update<Simple>(id, [](Simple *s) { return 42; });

  EXPECT_EQ(42, result);
}

TEST_F(StoreTest, ThrowsWhenUpdatingMissingEntities) {
  Store<Simple> store;
  Id missing = store.lastId<Simple>();

  EXPECT_THROW(
      store.update<Simple>(missing, [](Simple *s) { s->property = 42; }),
      MissingIdError);
}

TEST_F(StoreTest, ThrowsWhenUpdatingDifferentTypeThanStoredInId) {
  Store<Simple> store;
  Id differentType{2137, 2};

  EXPECT_THROW(store.update<Simple>(differentType, [](Simple *s) {}),
               IdTypeMismatchError);
}

// Store::has()

TEST_F(StoreTest, RecognizesIfIdExists) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  EXPECT_TRUE(store.has(id));
}

TEST_F(StoreTest, RecognizesIfIdDoesNotExist) {
  Store<Simple> store;
  Id missing = store.lastId<Simple>();

  EXPECT_FALSE(store.has(missing));
}

// Store::count

TEST_F(StoreTest, CountsEntitiesOfSingleType) {
  Store<Simple, Another> store;
  store.create<Simple>();
  store.create<Simple>();
  store.create<Another>();

  size_t simpleCount = store.count<Simple>();
  size_t anotherCount = store.count<Another>();

  EXPECT_EQ(2, simpleCount);
  EXPECT_EQ(1, anotherCount);
}

// Store::size()

TEST_F(StoreTest, TellsHowManyEntitiesItContains) {
  Store<Simple, Another> store;
  store.create<Simple>();
  store.create<Simple>();
  store.create<Another>();

  size_t size = store.size();

  EXPECT_EQ(3, size);
}

// Store::remove()

TEST_F(StoreTest, RemovesEntities) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  store.remove(id);

  EXPECT_FALSE(store.has(id));
}

TEST_F(StoreTest, ThrowsWhenRemovingMissingEntities) {
  Store<Simple> store;
  Id id = store.create<Simple>();
  store.remove(id);

  EXPECT_THROW(store.remove(id), MissingIdError);
}

// Store::clear()

TEST_F(StoreTest, ClearsEntitiesOfSingleType) {
  Store<Simple, Another> store;
  Id simple1 = store.create<Simple>();
  Id simple2 = store.create<Simple>();
  Id another1 = store.create<Another>();
  Id another2 = store.create<Another>();

  store.clear<Simple>();

  EXPECT_FALSE(store.has(simple1));
  EXPECT_FALSE(store.has(simple2));
  EXPECT_TRUE(store.has(another1));
  EXPECT_TRUE(store.has(another2));
}

// Store::clearAll()

TEST_F(StoreTest, ClearsEverything) {
  Store<Simple, Another> store;
  Id simple1 = store.create<Simple>();
  Id simple2 = store.create<Simple>();
  Id another1 = store.create<Another>();
  Id another2 = store.create<Another>();

  store.clearAll();

  EXPECT_FALSE(store.has(simple1));
  EXPECT_FALSE(store.has(simple2));
  EXPECT_FALSE(store.has(another1));
  EXPECT_FALSE(store.has(another2));
}

// Store::containsType()

TEST_F(StoreTest, ContainsTypeReturnsTrueIfStoreContainsType) {
  EXPECT_TRUE(Store<Simple>::containsType<Simple>());
}

TEST_F(StoreTest, ContainsTypeReturnsFalseIfStoreDoesNotContainType) {
  EXPECT_FALSE(Store<Simple>::containsType<Another>());
}

// Store::matchesType()

TEST_F(StoreTest, MatchesTypeReturnsTrueIfIdMatchesType) {
  Store<Simple> store;
  Id id = store.lastId<Simple>();

  EXPECT_TRUE(Store<Simple>::matchesType<Simple>(id));
}

TEST_F(StoreTest, MatchesTypeReturnsFalseIfIdDoesNotMatchType) {
  Store<Simple, Another> store;
  Id id = store.lastId<Simple>();

  EXPECT_FALSE(Store<Simple>::matchesType<Another>(id));
}

TEST_F(StoreTest, MatchesTypeReturnsFalseIfIdDoesNotExistInStore) {
  Store<Simple> store;
  Id id = store.lastId<Simple>();

  EXPECT_FALSE(Store<Simple>::matchesType<Another>(id));
}

// Store::transaction()

TEST_F(StoreTest, ExecutesOperations) {
  Store<Simple> store;

  bool success = store.transaction([&store]() {
    Id id = store.create<Simple>();
    store.update<Simple>(id, [](Simple *s) { s->property = 1; });
  });

  EXPECT_TRUE(success);
  const auto *entity = store.find<Simple>(store.lastId<Simple>());
  EXPECT_THAT(entity, Pointee(Field(&Simple::property, Eq(1))));
}

TEST_F(StoreTest, AbortsTransactionOnUpdateFailure) {
  Store<Simple> store;

  bool success = store.transaction([&store]() {
    Id invalid;
    store.update<Simple>(invalid, [](Simple *s) {});
  });

  EXPECT_FALSE(success);
}

TEST_F(StoreTest, AbortsTransactionOnRemoveFailure) {
  Store<Simple> store;

  bool success = store.transaction([&store]() {
    Id invalid;
    store.remove(invalid);
  });

  EXPECT_FALSE(success);
}

TEST_F(StoreTest, TransactionRollsBackCreates) {
  Store<Simple> store;

  store.transaction([&store]() {
    store.create<Simple>();
    triggerStoreException(store);
  });

  EXPECT_EQ(0, store.size());
}

TEST_F(StoreTest, TransactionRollsBackUpdates) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  store.transaction([&store, id]() {
    store.update<Simple>(id, [](Simple *s) { s->property = 21; });
    store.update<Simple>(id, [](Simple *s) { s->property = 42; });
    triggerStoreException(store);
  });

  const auto *entity = store.find<Simple>(id);
  EXPECT_THAT(entity, Pointee(Field(&Simple::property, AllOf(Ne(21), Ne(42)))));
}

TEST_F(StoreTest, TransactionRollsBackRemovals) {
  Store<Simple> store;
  Id id = store.create<Simple>();

  store.transaction([&store, id]() {
    store.remove(id);
    triggerStoreException(store);
  });

  EXPECT_TRUE(store.has(id));
}

TEST_F(StoreTest, TransactionRollsBackClears) {
  Store<Simple> store;
  store.create<Simple>();
  store.create<Simple>();

  store.transaction([&store]() {
    store.clear<Simple>();
    triggerStoreException(store);
  });

  EXPECT_EQ(2, store.size());
}

TEST_F(StoreTest, TransactionRollsBackClearAlls) {
  Store<Simple, Another> store;
  store.create<Simple>();
  store.create<Another>();

  store.transaction([&store]() {
    store.clearAll();
    triggerStoreException(store);
  });

  EXPECT_EQ(2, store.size());
}

// Store::trackingChanges()

TEST_F(StoreTest, TracksChangesIfThereIsATransaction) {
  Store<Simple> store;

  store.transaction([&store]() { EXPECT_TRUE(store.trackingChanges()); });
}

TEST_F(StoreTest, TracksChangesIfEnabledExplicitly) {
  Store<Simple> store;
  store.setTrackingChanges(true);

  EXPECT_TRUE(store.trackingChanges());
}

TEST_F(StoreTest, DoesNotTrackChangesByDefault) {
  Store<Simple> store;

  EXPECT_FALSE(store.trackingChanges());
}

class StoreChangeTrackingTest : public ::testing::Test {
 protected:
  Store<Simple> store;

  void SetUp() override { store.setTrackingChanges(true); }

  void TearDown() override {
    store.resetChanges();
    store.setTrackingChanges(false);
    store.clearAll();
  }
};

// Store::changes()

TEST_F(StoreChangeTrackingTest, TracksAdditions) {
  Id id = store.create<Simple>();

  EXPECT_THAT(store.changes().created, ElementsAre(id));
}

TEST_F(StoreChangeTrackingTest, TracksUpdates) {
  Id id = store.create<Simple>();
  store.update<Simple>(id, [](Simple *s) { s->property = 42; });

  EXPECT_THAT(store.changes().updated, ElementsAre(Pair(id, Simple{0})));
}

TEST_F(StoreChangeTrackingTest, TracksOnlyFirstPreUpdateValue) {
  Id id = store.create<Simple>();
  store.update<Simple>(id, [](Simple *s) { s->property = 42; });
  store.update<Simple>(id, [](Simple *s) { s->property = 2137; });

  EXPECT_THAT(store.changes().updated, ElementsAre(Pair(id, Simple{0})));
}

TEST_F(StoreChangeTrackingTest, TracksRemovals) {
  Id id = store.create<Simple>();
  store.remove(id);

  EXPECT_THAT(store.changes().removed, ElementsAre(Pair(id, Simple{0})));
}

TEST_F(StoreChangeTrackingTest, TracksTransactions) {
  store.transaction([this]() {
    Id id = store.create<Simple>();
    store.update<Simple>(id, [](Simple *s) { s->property = 42; });
    store.remove(id);
  });

  using Changes = Store<Simple>::Changes;
  Id id = store.lastId<Simple>();
  EXPECT_THAT(store.changes(),
              AllOf(Field("created", &Changes::created, ElementsAre(id)),
                    Field("updated", &Changes::updated,
                          ElementsAre(Pair(id, Simple{0}))),
                    Field("removed", &Changes::removed,
                          ElementsAre(Pair(id, Simple{42})))));
}

// Store::resetChanges()

TEST_F(StoreChangeTrackingTest, ResetsChanges) {
  Id id = store.create<Simple>();
  store.update<Simple>(id, [](Simple *s) { s->property = 42; });
  store.remove(id);

  store.resetChanges();

  using Changes = Store<Simple>::Changes;
  EXPECT_THAT(store.changes(),
              AllOf(Field("created", &Changes::created, ElementsAre()),
                    Field("updated", &Changes::updated, ElementsAre()),
                    Field("removed", &Changes::removed, ElementsAre())));
}

#pragma clang diagnostic pop
