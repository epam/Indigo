cd /opt/bingo-postgres/
source bingo-pg-install.sh -y
psql -U postgres -c "create database indigoservice"
psql -U postgres -d indigoservice -f ./bingo_install.sql
psql -U postgres -d indigoservice -f /opt/init_database.sql
cp /opt/postgresql.conf /var/lib/postgresql/data
