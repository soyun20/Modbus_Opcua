#include "db.h"

# define WRITE_SIZE 6

int main(int argc, char **argv)
{
    ClientDB client;

    client.get_settings();
    client.conn = mysql_init(NULL);
    client.db_conn(client.conn);
    client.write_db(WRITE_SIZE);

    return 0;
}