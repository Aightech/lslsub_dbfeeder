#!/bin/bash
export PGPASSWORD=azerty
#read -p 'Table: ' table
read -p 'UID: ' uid

table="ft_sensor"
file="${table}_id${uid}.csv"
psql -U lsldb_user -d lsldb -h localhost -c "\\copy (select (index,time,data) from ${table}_${uid} limit 3200000) to $file WITH DELIMITER ',' CSV"
sed -i 's/["{}()]//g' $file



table="forces_8"
file="${table}_id${uid}.csv"
psql -U lsldb_user -d lsldb -h localhost -c "\\copy (select (index,time,data) from ${table}_${uid} limit 3200000) to $file WITH DELIMITER ',' CSV"
sed -i 's/["{}()]//g' $file
#sed -i 's/{//' $file
#sed -i 's/}"")"//' $file



