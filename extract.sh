#!/bin/bash
export PGPASSWORD=azerty
read -p 'Table: ' table
read -p 'UID: ' uid
read -p 'CSV file: ' file

psql -U lsldb_user -d lsldb -h localhost -c "\\copy (select (t,data) from $table where uid='$uid') to $file WITH DELIMITER ',' CSV"

sed -i 's/"(//' $file
sed -i 's/""{//' $file
sed -i 's/}"")"//' $file



