# Locate postgres
# This module defines
# POSTGRES_LIBRARY_DIRS
# POSTGRES_INCLUDE_DIRS, where to find the headers

FIND_PATH(POSTGRES_INCLUDE_DIRS pg_config.h
    $ENV{BINGO_PG_DIR}/include
    $ENV{BINGO_PG_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /usr/include/postgres
	/usr/include/postgresql
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (MSVC)
	SET(LIBPOSTGRES "postgres.lib")
	FIND_PATH(POSTGRES_LIBRARY_DIRS ${LIBPOSTGRES}
		$ENV{BINGO_PG_DIR}/lib
		$ENV{BINGO_PG_DIR}
		~/Library/Frameworks
		/Library/Frameworks
		/usr/local/lib
		/usr/lib
		/sw/lib
		/opt/local/lib
		/opt/csw/lib
		/opt/lib
		/usr/freeware/lib64
	)
ENDIF()

SET(POSTGRES_FOUND "NO")
IF((POSTGRES_LIBRARY_DIRS OR (UNIX OR APPLE)) AND POSTGRES_INCLUDE_DIRS)
	SET(POSTGRES_LIBRARY ${POSTGRES_LIBRARY_DIRS}/${LIBPOSTGRES})
    SET(POSTGRES_FOUND "YES")
ENDIF()
