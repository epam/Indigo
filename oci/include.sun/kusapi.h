/*
 * $Header: kusapi.h 14-mar-97.08:14:40 pabingha Exp $
 */

/* Copyright (c) Oracle Corporation 1996, 1997. All Rights Reserved. */ 
 
/* NOTE:  See 'header_template.doc' in the 'doc' dve under the 'forms' 
      directory for the header file template that includes instructions. 
*/
 
/* 
   NAME 
     kusapi.h - Kernel Utility Statistics Application Programming Interface

   DESCRIPTION 
     Declarations of types and functions of the API used to retrieve 
     statistics from the SGA

   PUBLIC FUNCTION(S) 
     kusdb_connect - connect to a database
     kuscx_allocate - allocate continuation context
     kuscx_free - free continuation context
     kuscx_init - initialize continuation context
     kusdb_get_info - retrieve database information
     kusdb_get_memory - copy SGA contents into user-allocated memory
     kusdb_disconnect - disconnect from database
     kusdb_error_text - format message for last error
     kustv_allocate_handle - allocate a TLV handle
     kustv_dump - dump a TLV buffer
     kustv_free_handle - free a TLV handle
     kustv_get - get next entry in a TLV buffer
     kustv_offset - return offset for current entry in TLV buffer
     kussys_get_info - retrieve non-db-related info (noop for Oracle)
     kussys_error_text - format message for last error (noop for Oracle)
     kustvp1_put_one_byte - put TLV entry with 1 byte value into TLV buffer
     kustvp2_put_two_bytes - put TLV entry with 2 byte value into TLV buffer
     kustvp4_put_four_bytes - put TLV entry with 4 byte value into TLV buffer
     kustv_put - put TLV entry into TLV buffer
     kustvptg_put_tag - put TLV entry with empty value into TLV buffer
     kustv_reinit_handle - reset offset for a TLV handle
     kustv_buffer_state - return termination state for tlv buffer

   NOTES
     This interface is subject to change without notice

   MODIFIED   (MM/DD/YY)
   pabingha    03/14/97 - Add get_tagname argument
   jstenois    11/05/96 - "Make kustv_dump() header CONST"
   jstenois    11/01/96 - Use oratypes instead of sx.h
   jstenois    08/15/96 - SGAAPI public include file
   jstenois    08/15/96 - Creation

*/

 
#ifndef KUSAPI
# define KUSAPI

# ifndef ORATYPES
#  include <oratypes.h>
# endif


/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/* values for status code returned by API calls */

typedef enum kusst
  {

    KUSSTOK =		1,	/* Success                              */
    KUSSTEOB =		2,	/* Cursor at end of buffer.	        */
    KUSSTNWR =		3,	/* Attempt to write to TLV when no      */
				/*    length specified at init time	*/
    KUSSTILN =		4,	/* Invalid length specified when	*/
				/*    passing by value			*/
    KUSSTTRNC =		5,	/* buffer was truncated			*/
    KUSSTCONT =		6,	/* buffer is continued			*/
    KUSSTALLC =		7,	/* unable to allocate memory		*/
    KUSSTUNKC =		8,	/* unknown class tag specified		*/
    KUSSTICH =		9,	/* Invalid context handle		*/
    KUSSTIDH =		10,	/* Invalid database handle		*/
    KUSSTITH =		11,	/* Invalid TLV handle			*/
    KUSSTDIS =		12,	/* Stats collection disabled for database */
    KUSSTERR =		13,	/* Unexpected error encountered		*/
    KUSSTBINF =		14,	/* Input buffer contained a bad tag	*/
    KUSSTBDKY =         15,     /* Bad index value for class tag	*/
    KUSSTBFSML =	16,	/* Buffer is too small for data		*/
    KUSSTIBE =		17,	/* Information buffer is empty          */
    KUSSTDBDOWN =       18,     /* Database is unavaliable              */
    KUSSTFILACC =       19,     /* File cannot be accessed              */
    KUSSTINVDB =        20,     /* File is not a db root file (Rdb only)*/
    KUSSTNODB =         21,     /* Db root file doesn't exist (Rdb only)*/
    KUSSTBADVER =       22,     /* Db root file wrong version (Rdb only)*/
    KUSSTCIU =          23,     /* Context is already in use            */
    KUSSTDMPOP =        24,     /* Unable to open dump output file      */
    KUSSTDMPCL =        25      /* Unable to close dump output file     */
  } kusst;


/* values returned by kustvstate */ 

typedef enum kustvs
  {
    KUSTVS_CONT =	1,	/* tlv buffer terminated with KUSSTCONT	*/
    KUSTVS_END =	2,	/* tlv buffer terminated with KUSSTBEND	*/
    KUSTVS_TRUNC =	3,	/* tlv buffer terminated with KUSSTTRNC	*/
    KUSTVS_UNK =	4	/* handle specified has not been used	*/
                                /* to read or write a termination tag   */
  } kustvs;


/* Type Definitions for handles */

typedef dvoid *kusdb_handle;	/* handle for connected database */
typedef dvoid *kuscx_handle;	/* handle for continuation context */
typedef dvoid *kustv_handle;	/* handle for TLV context */

/* Data structure for use by kusdb_get_memory */

typedef struct kusmem
  {
    ub1 *sga_address_kusmem;    /* SGA memory address */
    ub4 length_kusmem;          /* Number of bytes to copy */
    ub1 *dest_address_kusmem;   /* User-allocated destination for SGA data */
  } kusmem;


/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/


/* ---------------------------- kuscx_allocate ----------------------------- */
/*
  NAME
    kuscx_allocate -  allocate continuation context
  DESCRIPTION
    Allocate space to track information about continuation context for a 
    kusdb_get_info call.
  PARAMETERS
    context_handle - handle for the continuation context (in/out)
  RETURN VALUE
    status of call
  NOTES
    kuscx_init can be used to reinitilize a context that has been allocated 
    and is no longer used for a TLV buffer
*/

kusst kuscx_allocate (/*_ kuscx_handle *context_handle _*/);




/* ------------------------------ kuscx_free_handle ------------------------ */
/*
  NAME
    kuscx_free_handle -  free continuation context
  DESCRIPTION
    free space used to track information about continuation context for a 
    kusdb_get_info call.
  PARAMETERS
    context_handle - handle for the continuation context (in/out)
  RETURN VALUE
    status of call
  NOTES
    kuscx_init can be used to reinitilize a context that has been allocated
    and is no longer used for a TLV buffer
*/

kusst kuscx_free_handle (/*_ kuscx_handle *context_handle _*/);




/* ------------------------------ kuscx_init ------------------------------- */
/*
  NAME
    kuscx_init -  initialize continuation context
  DESCRIPTION
    Reinitialize a context handle that has been previously allocated and used.
  PARAMETERS
    context_handle - handle for the continuation context (in/out)
  RETURN VALUE
    status of call
  NOTES
    kuscx_init is the semantic equivalent of freeing a context handle and then
    allocating a new one.
*/

kusst kuscx_init (/*_ kuscx_handle *context_handle _*/);



/* ----------------------------- kusdb_allocate ---------------------------- */
/*
  NAME
    kusdb_allocate - allocate a database handle
  DESCRIPTION
    allocate resources for a database handle
  PARAMETERS
    db_handle - handle for the database context (in/out)
  RETURN VALUE
    kusst - status of attempt to allocate
*/

kusst kusdb_allocate (/*_ kusdb_handle *db_handle _*/);




/* ----------------------------- kusdb_connect ----------------------------- */
/*
  NAME
    kusdb_connect - connect to a database
  DESCRIPTION
    Connects to a database so that statistics for that database can be
    retrieved.
  PARAMETERS
    db_handle - handle for the database context (in/out)
    database_name_length - number of bytes in database_name parameter
    database name - name of database
    security_info_length - number of bytes in security_info parameter
    security_info - security information used to attach to database
  RETURN VALUE
    status of attempt to connect
  NOTES
    If the status is not successful, call kusdb_error_text to get more 
    information about the failure.
    Refer to the documentation for the format of information in the
    database_name and security_info parameters.
*/

kusst kusdb_connect (/*_
	kusdb_handle db_handle,
	ub4 database_name_length,
	char *database_name,
	ub4 security_info_length,
	char *security_info
	_*/);




/* ---------------------------- kusdb_disconnect --------------------------- */
/*
  NAME
    kusdb_disconnect - disconnect from a database
  DESCRIPTION
    Disconnect from the current database
  PARAMETERS
    db_handle - handle for the database (in/out)
  RETURN VALUE
    status of call
  NOTES
*/

kusst kusdb_disconnect (/*_ kusdb_handle db_handle _*/);




/* ------------------------------- kusdb_free ------------------------------ */
/*
  NAME
    kusdb_free - free a database handle
  DESCRIPTION
    free resources for a database handle
  PARAMETERS
    db_handle - handle for the database context (in/out)
  RETURN VALUE
    kusst - status of attempt to free the handle
*/

kusst kusdb_free (/*_ kusdb_handle *db_handle _*/);




/* ----------------------------- kusdb_get_info ---------------------------- */
/*
  NAME
    kusdb_get_info - retrieve statistics for a database
  DESCRIPTION
    Retrieve requested statistics for the current database
  PARAMETERS
    db_handle - handle for the database (in)
    context_handle - handle for the continuation context (in/out)
    info_buf - TLV buffer indicating data to be returned (in)
    info_buf_len - number of bytes in info_buffer (in)
    result_buf - allocated TLV buffer to hold data returned (in/out)
    result_buf_len - number of bytes in result_buf (in/out)
    buffer_termination_status - indicates how buffer was terminated (normally,
      truncated, or continued) (out)
  RETURN VALUE
    status of call
  NOTES
    If the status is not successful, call kusdb_error_text to get more 
    information about the failure.
    Refer to documentation for information on the format of the information
    buffer and the result buffer
*/

kusst kusdb_get_info (/*_
	kusdb_handle db_handle,
	kuscx_handle ctx_handle,
	ub1 *info_buffer,
	ub4 info_buffer_length,
	ub1 *result_buffer,
	ub4 *result_buffer_length,
	kustvs *buffer_termination_status
	_*/);



/* ----------------------------- kusdb_get_memory -------------------------- */
/*
  NAME
    kusdb_get_memory - copy SGA contents into user-allocated memory
  DESCRIPTION
    Copy SGA memory into user-allocated memory regions. User provides
    an array of kusmem structures. Each kusmem structure in the array
    describes a distinct copy to be done by this routine.
  PARAMETERS
    db_handle - handle for the database (in)
    mem_array_length - number of elements in mem_array (in)
    mem_array - address of an array of kusmem structures (in/out)
  RETURN VALUE
    status of call:
        KUSSTOK - call succeeded, all requested data copied
        KUSSTIDH - invalid database handle
        KUSSTDBDOWN - database is down
        KUSSTERR - unexpected error
  NOTES
     User should prepare each kusmem structure in array by filling in
     values for sga_address_kusmem, length_kusmem and dest_address_kusmem
     prior to making this call. 

     User is reponsible for allocating and deallocating the memory used
     by mem_array and each region that a dest_address_kusmem points to.
*/

kusst kusdb_get_memory (/*_
	kusdb_handle db_handle,
        ub4 mem_array_length,
        kusmem *mem_array
	_*/);




/* ---------------------------- kusdb_error_text --------------------------- */
/*
  NAME
    kusdb_error_text - error text
  DESCRIPTION
    Display information about status of last kusdb call
  PARAMETERS
    db_ctx - context whose errors are to be returned (in)
    text_buf - buffer to hold error message (in/out)
    text_buf_len - number of bytes in text buffer (in)
    text_len_out - number of bytes written into text_buf
  RETURN VALUE
    status of call
  NOTES
*/

kusst kusdb_error_text (/*_ kusdb_handle db_ctx, char *text_buf,
			  ub4 text_buf_len, ub4 *text_len_out _*/);




/* ------------------------ kustv_allocate_handle -------------------------- */
/*
  NAME
    kustv_allocate_handle - allocate a handle for a TLV buffer
  DESCRIPTION
    allocates and initializes information for a TLV handle
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tlv_buffer - address of the TLV buffer for this handle (in)
    buf_size - number of bytes in tlv_buffer (in)
  RETURN VALUE
    KUSSTALL	Unable to allocate memory
  NOTES
    kustv_reinit can be used to reinitialize a TLV handle that is no longer
    needed.
*/

kusst kustv_allocate_handle (/*_
	kustv_handle *tlv_handle,
	ub1 *tlv_buffer,
	ub4 buf_size
	_*/);


#ifndef _WINDOWS


/* ------------------------------- kustv_dump -------------------------------*/
/*
  NAME
    kustv_dump - dump tlv buffer
  DESCRIPTION
    Formats the content of a TLV buffer into the file specified by the caller
  PARAMETERS
    tlv_handle - handle of the TLV buffer to be dumped (in)
    header - ASCIZ string to be displayed before dumping the buffer (in)
    data_flg - flag to indicate if the contents of the value fueld in the TLV
	should be displayed (in)
    append_flg - flag to indicate if an existing instance of the output
        file should be appended to (or replaced)
    get_tagname - routine to get text associated with tags
    directory - ASCIZ string containing a directory specification for the
        output file (if absent, current default directory is used)
    file_name - ASCIZ string containing a file name for the output file.
        If absent, stderr is used and append_flg and directory are ignored.
        If specified without an extension, the Oracle extension for an output
        file on the given platform is used.
  RETURN VALUE
    KUSSTOK	Success
    KUSSTITH	invalid tlv handle
    KUSSTDMPOP  can't open dump file
    KUSSTDMPCL  can't close dump file
  NOTES
*/

kusst kustv_dump (/*_
	kustv_handle tlv_handle,
	CONST char *header,
	int data_flg,
        int append_flg,
        CONST char *(*get_tagname)(ub2),
        CONST char *directory,
        CONST char *file_name
	_*/);

#endif


/* ------------------------- kustv_free_handle ----------------------------- */
/*
  NAME
    kustv_free_handle - free a handle for a TLV buffer
  DESCRIPTION
    Free a TLV handle that has already been created
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
  RETURN VALUE
    status of call
  NOTES
    kustv_reinit_handle can be used to reinitialize a handle that is no 
    longer needed.
*/

kusst kustv_free_handle (/*_ kustv_handle *tlv_handle _*/);

/* ----------------------------- kussys_get_info --------------------------- */
/*
  NAME
    kussys_get_info - retrieve non database specific data
  DESCRIPTION
    Retrieve non database specific information
  PARAMETERS
    context_handle - handle for the continuation context (in/out)
    info_buf - TLV buffer indicating data to be returned (in)
    info_buf_len - number of bytes in info_buffer (in)
    result_buf - allocated TLV buffer to hold data returned (in/out)
    result_buf_len - number of bytes in result_buf (in/out)
    buffer_termination_status - indicates how buffer was terminated (normally,
      truncated, or continued) (out)
  RETURN VALUE
    status of call
  NOTES
    This function is a noop for Oracle. It is included for compatability
    with Rdb only.
*/

kusst kussys_get_info (/*_
	kuscx_handle ctx_handle,
	ub1 *info_buffer,
	ub4 info_buffer_length,
	ub1 *result_buffer,
	ub4 *result_buffer_length,
	kustvs *buffer_termination_status
	_*/);





/* ---------------------------- kussys_error_text -------------------------- */
/*
  NAME
    kussys_error_text - error text
  DESCRIPTION
    Display information about status of last kustv call
  PARAMETERS
    text_buf - buffer to hold error message (in/out)
    text_buf_len - number of bytes in text buffer (in)
    text_len_out - number of bytes written into text_buf
  RETURN VALUE
    status of call
  NOTES
    This function is a noop for Oracle. It is included for compatability
    with Rdb only.
*/

kusst kussys_error_text (/*_ char *text_buf,
			  ub4 text_buf_len, ub4 *text_len_out _*/);




/* ------------------------------ kustv_get -------------------------------- */
/*
  NAME
    kustv_get - get next entry in tlv buffer
  DESCRIPTION
    Retrieves information about next entry in the tlv buffer and advances
    pointer into the TLV buffer to the following entry.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in)
    tag - tag for the next entry (out)
    length - number of bytes in the value for the TLV entry (out)
    value - pointer to value field for the TLV entry (out)
  RETURN VALUE
    KUSSTOK	Success
    KUSSTITH	Invalid TLV handle
  NOTES
*/

kusst kustv_get (/*_
	kustv_handle tlv_handle,
	ub2 *tag,
	ub2 *length,
	ub1 **value
	_*/);





/* ------------------------ kustv_reinit_handle ---------------------------- */
/*
  NAME
    kustv_reinit_handle - reinitialize a TLV handle
  DESCRIPTION
    Reimitializes a TLV handle so that it can be used to read or write a new
    TLV buffer.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    buffer - address of the TLV buffer
    buffer_size - number of bytes in the TLV buffer
  RETURN VALUE
    KUSSTOK	Success
    KUSSTITH	Invalid TLV handle
  NOTES
*/

kusst kustv_reinit_handle (/*_ 
	kustv_handle tlv_handle,
	ub1 *buffer,
	ub4 buffer_length
	_*/);





/* --------------------------- kustv_length -------------------------------- */

/*
  NAME
    kustv_length - return length of a TLV buffer
  DESCRIPTION
    return the length of a TLV buffer associated with a TLV handle
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in)
    buffer_length - number of bytes in the buffer (out)
  RETURN VALUE
    KUSSTOK	Success
    KUSSTITL	Invalid TLV handle
  NOTES
*/

kusst kustv_length (/*_ 
	kustv_handle tlv_handle,
	ub4 buffer_length
	_*/);




/* ---------------------------- kustv_offset ------------------------------- */

/*
  NAME
    kustv_offset - return offset of a TLV buffer
  DESCRIPTION
    return the current offset of a TLV buffer associated with a TLV handle
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in)
    offset - current offset for TLV buffer (out)
  RETURN VALUE
    KUSSTOK - success
    KUSSTITH - invalid TLV handle
  NOTES
*/

kusst kustv_offset (/*_ 
	kustv_handle tlv_handle, 
	ub4 *offset 
	_*/);




/* ------------------------ kustvp1_put_one_byte --------------------------- */

/*
  NAME
    kustvp1_put_one_byte - Insert a TLV entry with a value field of 1 byte
  DESCRIPTION
    Adds a new TLV entry to the end of the current TLV buffer.  This entry
    contains 1 byte.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tag - tag for the new entry (in)
    value - value for the new entry (in)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustvp1_put_one_byte (/*_
	kustv_handle tlv_handle,
	ub2 tag,
	ub1 value
	_*/);




/* ------------------------- kustvp2_put_two_bytes ------------------------- */

/*
  NAME
    kustvp2_put_two_bytes - Insert a TLV entry with a value field of 2 byte
  DESCRIPTION
    Adds a new TLV entry to the end of the current TLV buffer.  This entry
    contains 2 bytes.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tag - tag for the new entry (in)
    value - value for the new entry (in)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustvp2_put_two_bytes (/*_
	kustv_handle tlv_handle,
	ub2 tag,
	ub2 value
	_*/);




/* ----------------------- kustvp4_put_four_bytes -------------------------- */

/*
  NAME
    kustvp4_put_four_bytes - Insert a TLV entry with a value field of 4 byte
  DESCRIPTION
    Adds a new TLV entry to the end of the current TLV buffer.  This entry
    contains 4 bytes.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tag - tag for the new entry (in)
    value - value for the new entry (in)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustvp4_put_four_bytes (/*_
	kustv_handle tlv_handle,
	ub2 tag,
	ub4 value
	_*/);




/* ----------------------------- kustv_put --------------------------------- */

/*
  NAME
    kustv_put - Insert a TLV entry
  DESCRIPTION
    Adds a new TLV entry to the end of the current TLV buffer.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tag - tag for the new entry (in)
    length - number of bytes in the new entry (in)
    value - value for the new entry (in)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustv_put (/*_
	kustv_handle tlv_handle,
	ub2 tag,
	ub2 length,
	dvoid *value
	_*/);




/* -------------------------- kustvptg_put_tag ----------------------------- */

/*
  NAME
    kustvptg_put_tag - Insert a TLV entry with no value field
  DESCRIPTION
    Adds a new TLV entry to the end of the current TLV buffer.  the entry
    contains a 0-length value field.
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in/out)
    tag - tag for the new entry (in)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustvptg_put_tag (/*_
	kustv_handle tlv_handle,
	ub2 tag
	_*/);





/* -------------------------- kustv_buffer_state --------------------------- */

/*
  NAME
    kustv_buffer_state - return termination state of TLV buffer
  DESCRIPTION
    Returns informationon how a TLV buffer was terminated.  This call returns
    useful information only if a TLV handle was used to read or write the
    termination tag.  The termination information is preserved 
  PARAMETERS
    tlv_handle - handle of the TLV buffer (in)
    state - termination state for the TLV (out)
  RETURN VALUE
    status of the operation
  NOTES
*/

kusst kustv_buffer_state (/*_
	kustv_handle tlv_handle,
	kustvs *state
	_*/);


#endif                                              /* kusapi */
