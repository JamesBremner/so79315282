#include <iostream>
#include "sqlite3.h"
#include "cRunWatch.h"

main()
{
    // initialize timer  https://ravenspoint.wordpress.com/2010/06/16/timing/

    raven::set::cRunWatch::Start();

    // construct sqlite3 database
    sqlite3 *db;
    char *dbErrMsg;
    sqlite3_stmt *stmt;

    // open database
    sqlite3_open("test.dat", &db);

    // switch off synchronization
    // This command will cause SQLite to not wait on data to reach the disk surface
    sqlite3_exec(
        db,
        "PRAGMA synchronous=OFF",
        0, 0, &dbErrMsg);

    // create table for insertions
    sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS test "
        " ( v );",
        0, 0, &dbErrMsg);

    // prepare insertion statement
    sqlite3_prepare_v2(
        db,
        "INSERT INTO test "
        "VALUES ( ?1 );",
        -1,
        &stmt,
        0);

    // specify number of insertion to run
    const int count = 500;

    // specify all insertions to be done in one transaction
    sqlite3_exec(
        db,
        "START TRANSACTION",
        0, 0, &dbErrMsg);

    {
        // start timing execution of this scope
        raven::set::cRunWatch aWatcher("testInsert");

        // loop over insertions
        for (int k = 0; k < count; k++)
        {
            // bind insertion number to insertion value
            sqlite3_bind_int(stmt, 1, k);

            // execute insertion
            int rc = sqlite3_step(stmt);

            // check for success
            if (rc != SQLITE_DONE)
                throw std::runtime_error(
                    "INSERT failed"                );

            // reset
            sqlite3_reset(stmt);

            // end insertion loop
        }

        // end timing scope
    }

    // end transaction
    sqlite3_exec(
        db,
        "END TRANSACTION",
        0, 0, &dbErrMsg);

    // report
    std::cout << count << " inserts\n";
    raven::set::cRunWatch::Report();

    return 0;
}