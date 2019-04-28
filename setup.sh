#!/bin/bash

sudo apt update
sudo apt install -y postgresql-11


sudo runuser -l postgres -c $'psql -c "alter user postgres with password \'azerty\'"'

sudo runuser -l postgres -c 'createuser lsldb_user -drs'
sudo runuser -l postgres -c $'psql -c "alter user lsldb_user with password \'azerty\'"'


# Add our PPA
sudo add-apt-repository ppa:timescale/timescaledb-ppa -y
sudo apt-get update

# Now install appropriate package for PG version 
sudo apt install timescaledb-postgresql-11 -y

sudo timescaledb-tune -y
sudo service postgresql restart

export PGPASSWORD=azerty
sudo runuser -l postgres -c 'createdb lsldb -O lsldb_user'
psql -U lsldb_user -h localhost -d lsldb -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;"
