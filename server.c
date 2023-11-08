#include <microhttpd.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

// SQLite database file
const char *db_file = "location.db";

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "GET") == 0 && strcmp(url, "/store_location") == 0) {
        const char *response = "Not Found";
        struct MHD_Response *response_data;
        int ret;

        // Parse latitude and longitude from query parameters
        double latitude = 0.0, longitude = 0.0;
        const char *lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lat");
        const char *lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lon");

        if (lat_str && lon_str) {
            latitude = atof(lat_str);
            longitude = atof(lon_str);

            sqlite3 *db;
            char *error_message = 0;
            const char *sql;

            int rc = sqlite3_open(db_file, &db);
            if (rc) {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            } else {
                sql = "INSERT INTO locations (latitude, longitude) VALUES (?, ?)";
                rc = sqlite3_exec(db, sql, 0, 0, &error_message);
                if (rc != SQLITE_OK) {
                    fprintf(stderr, "SQL error: %s\n", error_message);
                    sqlite3_free(error_message);
                } else {
                    response = "Location data stored";
                }

                sqlite3_close(db);
            }
        }

        response_data = MHD_create_response_from_buffer(strlen(response), (void *)response, MHD_RESPMEM_MUST_COPY);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response_data);
        MHD_destroy_response(response_data);

        return ret;
    } else {
        const char *response = "Not Found";
        struct MHD_Response *response_data;
        int ret;

        response_data = MHD_create_response_from_buffer(strlen(response), (void *)response, MHD_RESPMEM_MUST_COPY);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response_data);
        MHD_destroy_response(response_data);

        return ret;
    }
}

int main() {
    struct MHD_Daemon *daemon;

    sqlite3 *db;
    char *error_message = 0;

    int rc = sqlite3_open(db_file, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS locations (latitude REAL, longitude REAL)";
    rc = sqlite3_exec(db, sql, 0, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    }

    sqlite3_close(db);

    daemon = MHD_start_daemon(MHD_USE_AUTO | MHD_USE_SUSPEND_RESUME | MHD_USE_DEBUG,
                              8080, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);

    if (daemon == NULL)
        return 1;

    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
