#!/bin/bash
export PGPASSWORD=azerty


sudo apt update
sudo apt install -y postgresql-10


sudo runuser -l postgres -c $'psql -c "alter user postgres with password \'azerty\'"'

sudo runuser -l postgres -c 'createuser lsldb_user -drs'
sudo runuser -l postgres -c $'psql -c "alter user lsldb_user with password \'azerty\'"'


# Add our PPA
sudo add-apt-repository ppa:timescale/timescaledb-ppa -y
sudo apt-get update

# Now install appropriate package for PG version 
sudo apt install timescaledb-postgresql-10 -y

sudo timescaledb-tune -yes
sudo service postgresql restart

export PGPASSWORD=azerty
sudo runuser -l postgres -c 'createdb lsldb -O lsldb_user'
psql -U lsldb_user -h localhost -d lsldb -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;"


psql -U lsldb_user -h localhost -d lsldb -c "
CREATE TABLE IF NOT EXISTS lsl_streams_metadata (
  name        TEXT       NOT NULL,
  type 	      TEXT 	 NOT NULL,
  format INTEGER  NOT NULL,
  rate    DOUBLE PRECISION  NOT NULL,
  nb_channels INTEGER NOT NULL,
  host TEXT NOT NULL
);"

psql -U lsldb_user -h localhost -d lsldb -c "
CREATE TABLE IF NOT EXISTS ESP (
  time        TIMESTAMPTZ       NOT NULL,
  data DOUBLE PRECISION[]  NOT NULL,
  uid TEXT  NULL
);"


