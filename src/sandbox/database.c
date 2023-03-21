#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct database database;

struct database {
    char* db_filename;
    void (*create)(database *self, const char* table_name, const char** keys, const char** vals);
    void (*insert)(database *self, const char* table_name, const char** keys, const char** vals);
    void (*fetch)(database *self, const char* table_name, char** keys, char** vals);
};

void create_impl(database *self, const char* table_name, const char** keys, const char** vals)
{
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(self->db_filename, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char* sql = malloc(sizeof(char) * 1024);
    sprintf(sql, "CREATE TABLE IF NOT EXISTS %s (id INTEGER PRIMARY KEY AUTOINCREMENT", table_name);

    for (int i = 0; keys[i]; i++)
    {
        strcat(sql, ", ");
        strcat(sql, keys[i]);
        strcat(sql, " TEXT");
    }

    strcat(sql, ");");

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
    free(sql);
}

void insert_impl(database *self, const char* table_name, const char** keys, const char** vals)
{
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(self->db_filename, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char* sql = malloc(sizeof(char) * 1024);
    sprintf(sql, "INSERT INTO %s (", table_name);

    for (int i = 0; keys[i]; i++)
    {
        if (i > 0) strcat(sql, ", ");
        strcat(sql, keys[i]);
    }

    strcat(sql, ") VALUES (");

    for (int i = 0; vals[i]; i++)
    {
        if (i > 0) strcat(sql, ", ");
        strcat(sql, "'");
        strcat(sql, vals[i]);
        strcat(sql, "'");
    }

    strcat(sql, ");");

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
    free(sql);
}


void fetch_impl(database *self, const char* table_name, char** keys, char** vals)
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(self->db_filename, &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char* sql = malloc(sizeof(char) * 2048);
    sprintf(sql, "SELECT * FROM %s;", table_name);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    int row_count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int col_count = sqlite3_column_count(stmt);

        for (int i = 0; i < col_count; i++)
        {
            const char *col_name = sqlite3_column_name(stmt, i);
            const char *col_val = (const char*)sqlite3_column_text(stmt, i);

            if (row_count == 0) keys[i] = strdup(col_name);

            vals[row_count * col_count + i] = strdup(col_val);
        }

        row_count++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);  
}

void database_ctor(database *self, const char* db_filename)
{
    size_t db_filename_len = strlen(db_filename) + 4;
    self->db_filename = malloc(db_filename_len);
    snprintf(self->db_filename, db_filename_len, "%s", db_filename);

    self->create = create_impl;
    self->insert = insert_impl;
    self->fetch = fetch_impl;
}

void database_dtor(database *self){  }

int main()
{
    database db;
    database_ctor(&db, "person.db");

    const char* table_name = "person";
    const char* keys[] = {"name", "age", "email"};
    const char* vals[] = {"John Doe", "30", "john.doe@example.com"};

    db.create(&db, table_name, keys, vals);
    // db.insert(&db, table_name, keys, vals);

    // char* fetched_keys[] = {"name", "age", "email"};
    // char* fetched_vals[3];

    // db.fetch(&db, table_name, fetched_keys, fetched_vals);

    // for (int i = 0; i < 3; i++) printf("%s: %s\n", fetched_keys[i], fetched_vals[i]);

    return 0;
}