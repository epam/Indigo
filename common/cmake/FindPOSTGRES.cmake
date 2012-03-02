# Locate postgres
# This module defines
# POSTGRES_LIBRARY_DIRS
# POSTGRES_INCLUDE_DIRS, where to find the headers

FIND_PATH(POSTGRES_INCLUDE_DIRS postgres.h
    $ENV{BINGO_PG_DIR}/include/server
    $ENV{BINGO_PG_DIR}/server
    $ENV{BINGO_PG_DIR}/include/postgresql/server
    /usr/local/include/server
    /usr/include/postgres/ENV{BINGO_PG_VERSION}/server
    /usr/include/postgresql/ENV{BINGO_PG_VERSION}/server
    /Library/PostgreSQL/$ENV{BINGO_PG_VERSION}/include/postgresql/server
    /usr/include/postgresql/$ENV{BINGO_PG_VERSION}/server
    /opt/local/include/postgresql/$ENV{BINGO_PG_VERSION}/server
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
