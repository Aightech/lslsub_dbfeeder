# lslsub_dbfeeder

## Requierements
###PostreSQL
sudo apt install postgresql

sudo -i -u postgres
psql
\password postgres
...
psql -h localhost -U postgres

Time scale db
sudo sh -c "echo 'deb http://apt.postgresql.org/pub/repos/apt/ `lsb_release -c -s`-pgdg main' >> /etc/apt/sources.list.d/pgdg.list"
wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
sudo apt-get update

# Add our PPA
sudo add-apt-repository ppa:timescale/timescaledb-ppa
sudo apt-get update

# Now install appropriate package for PG version
sudo apt install timescaledb-postgresql-10

sudo timescaledb-tune
sudo service postgresql restart

sudo apt install libpqxx-dev

