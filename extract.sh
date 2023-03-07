#!/bin/bash
export PGPASSWORD=azerty
read -p 'Table: ' table
read -p 'UID: ' uid

file="${table}_id${uid}.csv"

psql -U lsldb_user -d lsldb -h localhost -c "\\copy (select (time,t,data) from $table where uid='$uid' limit 320000) to $file WITH DELIMITER ',' CSV"

sed -i 's/"(//' $file
sed -i 's/""{//' $file
sed -i 's/}"")"//' $file



