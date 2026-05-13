#pragma once

#include <string>
#include <functional>
#include <mutex>
#include "db/database_manager.h"

namespace sophon {
namespace web {
namespace db {

class Transaction {
public:
    explicit Transaction(DatabaseManager& db);
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    void commit();
    void rollback();

    bool isActive() const { return active_; }

private:
    DatabaseManager& db_;
    bool active_ = false;
    bool committed_ = false;
};

template<typename Func>
bool withTransaction(DatabaseManager& db, Func&& func) {
    Transaction txn(db);
    try {
        func();
        txn.commit();
        return true;
    } catch (...) {
        txn.rollback();
        return false;
    }
}

} // namespace db
} // namespace web
} // namespace sophon
