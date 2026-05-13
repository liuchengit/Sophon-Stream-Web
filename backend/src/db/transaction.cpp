#include "db/transaction.h"
#include <sqlite3.h>
#include <iostream>

namespace sophon {
namespace web {
namespace db {

Transaction::Transaction(DatabaseManager& db) : db_(db) {
    if (db_.execute("BEGIN TRANSACTION")) {
        active_ = true;
    } else {
        std::cerr << "Failed to begin transaction" << std::endl;
    }
}

Transaction::~Transaction() {
    if (active_ && !committed_) {
        rollback();
    }
}

void Transaction::commit() {
    if (active_ && !committed_) {
        if (db_.execute("COMMIT")) {
            committed_ = true;
            active_ = false;
        } else {
            std::cerr << "Failed to commit transaction" << std::endl;
            rollback();
        }
    }
}

void Transaction::rollback() {
    if (active_) {
        db_.execute("ROLLBACK");
        active_ = false;
        committed_ = false;
    }
}

} // namespace db
} // namespace web
} // namespace sophon
