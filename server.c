#include <sqlite3.h>
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// SQLite database file
const char *db_file = "location.db";

typedef struct location_t {
    double latitude;
    double longitude;
} location_t;

static struct location_t lastLocation = {0, 0};  // To store the last location

const char *create_location_json(double latitude, double longitude) {
    // Create a JSON response string with latitude and longitude
    static char response[128];
    snprintf(response, sizeof(response), "{\"latitude\": %.6f, \"longitude\": %.6f}", latitude, longitude);
    return response;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "GET") == 0 && strcmp(url, "/daistkato") == 0) {
        const char *response = "Not Found";
        struct MHD_Response *response_data;
        int ret;

        // Parse latitude and longitude from query parameters
        double latitude = 0.0, longitude = 0.0;
        const char* ts = ctime(NULL);
        const char *lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lat");
        const char *lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lon");

        if (lat_str && lon_str) {
            latitude = atof(lat_str);
            longitude = atof(lon_str);

            sqlite3 *db;
            char *error_message = 0;
            char sql[256];

            int rc = sqlite3_open(db_file, &db);
            if (rc) {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            } else {
                snprintf(sql, sizeof(sql),
                               "INSERT INTO locations (latitude, longitude, time) VALUES (%g, %g, %s)",
                                latitude, longitude, ts);
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
    } else if (strcmp(method, "GET") == 0 && strcmp(url, "/woistkato") == 0) {
        const char *response = "Not Found";
        struct MHD_Response *response_data;
        int ret;

        // Retrieve the last stored location from the SQLite database
        double latitude = 0.0, longitude = 0.0;
        sqlite3 *db;
        sqlite3_stmt *stmt;

        int rc = sqlite3_open(db_file, &db);
        if (rc) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        } else {
            const char *sql = "SELECT latitude, longitude FROM locations ORDER BY ROWID DESC LIMIT 1";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if (rc == SQLITE_OK) {
                rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    latitude = sqlite3_column_double(stmt, 0);
                    longitude = sqlite3_column_double(stmt, 1);
                    response = create_location_json(latitude, longitude);
                }
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
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
    unsigned short port = 8080;
    char *error_message = 0;

    int rc = sqlite3_open(db_file, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return (1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS locations (latitude REAL, longitude REAL, time TIME)";
    rc = sqlite3_exec(db, sql, 0, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);
        sqlite3_free(error_message);
    }

    sqlite3_close(db);

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                              port, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);

    if (daemon == NULL)
        return 1;

    printf("Starting server on %u ... press enter key to terminate\n", port);

    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
