@echo off
cls
echo Starting...
SET PGPASSWORD=azerty

createuser -U postgres -drs lsldb_user
psql -U postgres -c "alter user lsldb_user with password 'azerty'"
createdb -U postgres -O lsldb_user lsldb

:: timescale db should be installed and psql restarted
::pg_ctl -D "C:\Program Files\PostgreSQL\9.6\data" stop
::pg_ctl -D "C:\Program Files\PostgreSQL\9.6\data" start

psql -U lsldb_user -h localhost -d lsldb -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;"

psql -U lsldb_user -h localhost -d lsldb -c "CREATE TABLE IF NOT EXISTS lsl_streams_metadata ( name        TEXT       NOT NULL, type 	      TEXT 	 NOT NULL, format INTEGER  NOT NULL, rate    DOUBLE PRECISION  NOT NULL, nb_channels INTEGER NOT NULL, host TEXT NOT NULL);"
pause
