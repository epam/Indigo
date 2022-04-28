System Requirements
-------------------

### Operating Systems ###

  * Linux (all modern distributions, 32-bit and 64-bit)
  * Windows (all modern distributions, 32-bit and 64-bit)
  * Mac OS X (10.5 Leopard and 10.6 Snow Leopard 64 bit)

### Database Servers ###

  * PostreSQL 12.8
  * PostreSQL 11.13
  * PostreSQL 10.18

### Tested Configurations ###

 Bingo has been successfully tested for 10-12 PostgreSQL versions mentioned above.


Installation Prerequisities
---------------------------

### All Systems ###

 The shared buffer parameter in the PostgreSQL database configuration file (postgresql.conf) should be increased. For the optimal performance EPAM recommends to increase the value . 

    shared_buffers=64MB

### Linux ###

The Bingo engine requires a lot of shared memory. 
For linux systems EPAM recommends to change kernel.shmmax and kernel.shmall

 Add the following line to `/etc/sysctl.conf` file:

    kernel.shmmax=<value>
    kernel.shmall=<value>

The recommended value is appr. 50% of the RAM (in bytes)

Execute (with the root privilegies)

     sysctl -p /etc/sysctl.conf


Installation Procedure
----------------------

### All Systems ###

Download and unzip the cartridge archive. The Bingo library is located in the 'bin' directory (bingo_postgres.dll for WINDOWS, or bingo_postgres.so for LINUX, or bingo_postgres.dylib for MacOSX). The Bingo library is built with specific PostgreSQL headers, so there is no need to run the building procedure. You can simply run the pre-generated SQL script with specified path to the Bingo library. 

   1. Copy the library file to a desired directory. The library can be copied into the PostgreSQL package directory (`{POSTGRES_HOME}/lib` by default). In the second case the `-pglibdir` option should be added to the SQL-gen script (see the full options list below)

   2. Run SQL-gen script (specific for an OS). The script generates two SQL scripts:

    bingo_install.sql

    bingo_uninstall.sql

 The `bingo_install.sql` installs all the functions and procedures related to the Bingo cartridge. 

   3. Execute the script from the database. Usually it can be done by the following command:
    
    psql -U $admin -d $database -f bingo_install.sql

There are several important notes below.

 **Note:** The installation can be done only to an existing database.

 **Note:** You must have an admin role to install bingo cartridge on your database.

 **Note:** The installation script creates a new schema (usually `bingo`),with the default tablespace. Please consider, that the specifying schema  will be deleted while calling the uninstall script, thus the installer checks for a schema name and will raise an error if a schema already exists.

 **Note:** You cannot install Bingo on top of the existing installation. You have to drop the cartridge schema
(usually `bingo`) of the existing installation.

 **Note:** You cannot install Bingo on a different PostgreSQL version, say, if you have PostgreSQL 9.1 and the Bingo library for the version 9.0 then you can get unexpected crashes.

### Linux ###

 Run the `bingo-pg-install.sh` file located in the root folder of the Bingo installation file set. The help
message from the script is the following:

    Usage: bingo-pg-install.sh [parameters]
    Parameters:
    -?, -help
      Print this help message
    -libdir path
      Target directory with the installed bingo_postgres.so (defaut {CURRENT_DIR}/bin/).
    -schema name
      Postgres schema name (default "bingo").
    -pglibdir
      Use postgreSQL $libdir option (default "false")
      Notice: bingo_postgres.so must be placed in the package library directory
    -y
      Process default options (default "false")

 Execute bingo_install.sql for your database.


### Windows ###

 Run the `bingo-pg-install.bat` file located in the root folder of the Bingo installation file set. The help
message from the script is the following:

    Usage: bingo-pg-install.bat [parameters]
    Parameters:
    -?, -help
      Print this help message
    -libdir path
      Target directory with the installed bingo_postgres.dll (defaut {CURRENT_DIR}/bin/).
    -schema name
      Postgres schema name (default "bingo").
    -pglibdir
      Use postgreSQL $libdir option (default "false")
      Notice: bingo_postgres.dll must be placed in the package library directory
    -y
      Process default options (default "false")


 Execute bingo_install.sql for your database.

### Mac OS X ###

 Run the `bingo-pg-install.sh` file located in the root folder of the Bingo installation file set. The help
message from the script is the following:

    Usage: bingo-pg-install.sh [parameters]
    Parameters:
    -?, -help
      Print this help message
    -libdir path
      Target directory with the installed bingo_postgres.dylib (defaut {CURRENT_DIR}/bin/).
    -schema name
      Postgres schema name (default "bingo").
    -pglibdir
      Use postgreSQL $libdir option (default "false")
      Notice: bingo_postgres.dylib must be placed in the package library directory
    -y
      Process default options (default "false")

 Execute bingo_install.sql for your database.


### Examples ###

 For the most simple installation, the defaults are taken: `CURRENT_DIR/bin` directory for binary, `bingo` for the schema name, `test` for the database name and `postgres` for the admin user.

    bingo-pg-install.sh
    psql -U postgres -d test -f bingo_install.sql
    

 If you have copied the library to the directory '/home/myself/':

    bingo-pg-install.sh -libdir /home/myself
    psql -U postgres -d test -f bingo_install.sql

 If you have copied the library to the Postges package directory '/usr/lib/postgresql/9.0/lib/' and want to create another `bingo2` schema for storing the procedures :

    bingo-pg-install.sh -pglibdir -schema bingo2
    psql -U postgres -d test -f bingo_install.sql

 
### Checking the Installation ###

 To check that the shared library file is loaded properly by Postgres, you can try this simple query:

    SELECT Bingo.GetVersion();

Uninstalling the Cartridge
--------------------------

 To uninstall the cartridge, you must:

   1. Execute bingo_uninstall.sql (generated on the installation step) for your database.

