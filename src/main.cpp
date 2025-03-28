#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "sqliteClass.h"
#include "cRunWatch.h"

raven::sqliteClass db;

void testInsert()
{
    sqlite3 *db;
    char *dbErrMsg;
    sqlite3_stmt *stmt;

    sqlite3_open("test.dat", &db);
    sqlite3_exec(
        db,
        "PRAGMA synchronous=OFF",
        0, 0, &dbErrMsg);
    sqlite3_exec(
        db,
        "CREATE TABLE IF NOT EXISTS test "
        " ( v );",
        0, 0, &dbErrMsg);
    sqlite3_prepare_v2(
        db,
        "INSERT INTO test "
        "VALUES ( ?1 );",
        -1,
        &stmt,
        0);

    const int count = 500;

    sqlite3_exec(
        db,
        "START TRANSACTION",
        0, 0, &dbErrMsg);

    {
        raven::set::cRunWatch aWatcher("testInsert");

        for (int k = 0; k < count; k++)
        {
            sqlite3_bind_int(stmt, 1, k);
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE)
            {
                sqlite3_reset(stmt);
            }
        }
    }
    sqlite3_exec(
        db,
        "END TRANSACTION",
        0, 0, &dbErrMsg);
    std::cout << count << " inserts\n";
}

struct sFlight
{
    int name;
    int arr;
    int arrtime;
    int dep;
    int deptime;
};

void createDB()
{
    db.open("flights.dat");
    db.exec("PRAGMA synchronous=OFF");
    db.exec(
        "CREATE TABLE IF NOT EXISTS flights "
        " ( name, dep, dep_time, arr, arr_time );");
    db.exec(
        "CREATE TABLE IF NOT EXISTS conx "
        " ( arr, dep );");
}

void gen(int flightCount, int airportCount)
{
    db.exec(
        "DELETE FROM flights;");
    auto stmt = db.prepare(
        "INSERT INTO flights "
        " VALUES ( ?1, ?2, ?3, ?4, ?5 );");
    for (int k = 0; k < flightCount; k++)
    {
        int dep = rand() % airportCount;
        int arr;
        while (1)
        {
            arr = rand() % airportCount;
            if (arr != dep)
                break;
        }
        int deptime = rand() % 2400;
        int arrtime;
        while (1)
        {
            arrtime = rand() % 2400;
            if (arrtime > deptime)
                break;
        }

        db.bind(stmt, 1, k);
        db.bind(stmt, 2, dep);
        db.bind(stmt, 3, deptime);
        db.bind(stmt, 4, arr);
        db.bind(stmt, 5, arrtime);
        db.exec(stmt,
                [&](raven::sqliteClassStmt &stmt) -> bool
                {
                    return true;
                });
    }
}

void readArrivals(
    std::vector<sFlight> &vArrivals,
    int airport,
    raven::sqliteClassStmt *stmt)
{
    raven::set::cRunWatch aWatcher("readArrivals");
    db.bind(stmt, 1, airport);
    db.exec(stmt,
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                sFlight F;
                F.name = stmt.column_int(0);
                F.dep = stmt.column_int(1);
                F.deptime = stmt.column_int(2);
                F.arr = stmt.column_int(3);
                F.arrtime = stmt.column_int(4);
                vArrivals.push_back(F);
                return true;
            });
}
void readDeps(
    std::vector<sFlight> &vDeps,
    int airport,
    raven::sqliteClassStmt *stmt)
{
    raven::set::cRunWatch aWatcher("readDeps");
    db.bind(stmt, 1, airport);
    db.exec(stmt,
            [&](raven::sqliteClassStmt &stmt) -> bool
            {
                sFlight F;
                F.name = stmt.column_int(0);
                F.dep = stmt.column_int(1);
                F.deptime = stmt.column_int(2);
                F.arr = stmt.column_int(3);
                F.arrtime = stmt.column_int(4);
                vDeps.push_back(F);
                return true;
            });
}

void findConnections(int airportCount)
{
    raven::set::cRunWatch aWatcher("findConnections");

    /*
    LOOP A over every airport
        Read into memory every arrival at A ( one query to DB )
        Read into memory every departure at A ( one query to DB )
        LOOP R over arrivals
           LOOP D over departures
              IF D later than R
                  Save into memory connection between R and D
           ENDLOOP
        ENDLOOP
   Save to DataBase all connections at A ( one query to DB )
   ENDLOOP

*/

    auto stmtArrs = db.prepare(
        "SELECT * FROM flights "
        "WHERE arr = ?1;");
    auto stmtDeps = db.prepare(
        "SELECT * FROM flights "
        "WHERE dep = ?1;");
    auto stmtConx = db.prepare(
        "INSERT INTO conx "
        "VALUES ( ?1, ?2 );");
    db.exec(
        "DELETE FROM conx;");

    // loop over airports
    for (int ka = 0; ka < airportCount; ka++)
    {
        std::vector<sFlight> vArrivals, vDeps;
        std::vector<std::pair<sFlight, sFlight>> vConnects;

        std::cout << "airport " << ka << "\n";

        // read arrivals at airport
        readArrivals(vArrivals, ka, stmtArrs);

        // read departures from airport
        readDeps(vDeps, ka, stmtDeps);

        // std::cout << "airport " << ka << " arrivals " << vArrivals.size()
        //           << " deps " << vDeps.size() << "\n";

        // connect arrivals to a later departures
        db.exec("START TRANSACTION");
        for (auto &r : vArrivals)
        {
            for (auto &d : vDeps)
            {
                if (d.deptime > r.arrtime)
                {
                    // save arrival flight name and departure flight name
                    db.bind(stmtConx, 1, r.name);
                    db.bind(stmtConx, 2, d.name);
                    // db.exec(stmtConx);
                }
            }
        }
        db.exec("END TRANSACTION");
    }
}

main()
{
    raven::set::cRunWatch::Start();
    testInsert();
    // createDB();
    // const int flightCount = 1000;
    // const int airportCount = 10;
    // std::cout << "Flights in DB " << flightCount << "\n";
    // gen(flightCount, airportCount);
    // findConnections(airportCount);
    raven::set::cRunWatch::Report();
    return 0;
}
