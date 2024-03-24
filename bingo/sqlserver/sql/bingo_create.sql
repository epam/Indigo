-- Enable CLR
USE master 
GO  
sp_configure 'clr enabled', 1;
GO
RECONFIGURE
GO

IF (select is_broker_enabled from sys.databases where name='$(database)') = 0
BEGIN
	-- It is necessary to generate new service_broker_guid when any other database has identical active service_broker_guid
	-- (target database might be used as a template and be created via backup, so that's why duplications are possible
	-- and in addition the same service_broker_guid might be enabled on another db)
	IF (
		select count(*) has_service_broker_guid_conflicts
		from sys.databases
		where name = '$(database)' and service_broker_guid = ANY 
		(
			select service_broker_guid
			from sys.databases
			group by service_broker_guid
			-- check for duplicates with enabled service broker
			having count(*) > 1 and max(convert(int, is_broker_enabled)) = 1
		)
		) = 1
		ALTER DATABASE $(database) SET NEW_BROKER WITH ROLLBACK IMMEDIATE
	ELSE
		-- otherwise enable service broker for event notification purposes
		ALTER DATABASE $(database) SET ENABLE_BROKER
END
go

-- Create key for adding unsafe assembly. It might be already created for different installation
BEGIN TRY
	CREATE ASYMMETRIC KEY bingo_assembly_key FROM EXECUTABLE FILE = '$(bingo_assembly_path).dll'
	CREATE LOGIN bingo_assembly_login FROM ASYMMETRIC KEY bingo_assembly_key
	GRANT UNSAFE ASSEMBLY TO bingo_assembly_login
END TRY
BEGIN CATCH
	PRINT ERROR_NUMBER();
	PRINT ERROR_MESSAGE();
END CATCH;
GO

use $(database)
go

PRINT 'Creating certificate...';
CREATE CERTIFICATE $(bingo)_certificate
		ENCRYPTION BY PASSWORD = '$(bingo_pass)' 
		WITH SUBJECT = 'Certificate for bingo procedures executions',
		START_DATE = '20020101', EXPIRY_DATE = '21000101'
GO

-- Create a bingo user
PRINT 'Creating bingo user with schema...';
CREATE USER $(bingo) FROM CERTIFICATE $(bingo)_certificate;
go

CREATE SCHEMA $(bingo);
go

ALTER AUTHORIZATION ON SCHEMA::$(bingo) TO $(bingo)
go

GRANT create table TO $(bingo);
go

-- Create tables for bingo user
create table [$(bingo)].CONFIG (n int not null, name varchar(100) not null, value varchar(4000), primary key(n, name));
create index CONFIG_N on [$(bingo)].CONFIG(n); 
insert into [$(bingo)].CONFIG values(0, 'treat-x-as-pseudoatom', '0');
insert into [$(bingo)].CONFIG values(0, 'ignore-closing-bond-direction-mismatch', '0');
insert into [$(bingo)].CONFIG values(0, 'nthreads', '-1');
insert into [$(bingo)].CONFIG values(0, 'ignore-stereocenter-errors', 0);
insert into [$(bingo)].CONFIG values(0, 'ignore-cistrans-errors', 0);
insert into [$(bingo)].CONFIG values(0, 'allow-non-unique-dearomatization', 0);
insert into [$(bingo)].CONFIG values(0, 'zero-unknown-aromatic-hydrogens', 0);
insert into [$(bingo)].CONFIG values(0, 'stereochemistry-bidirectional-mode', 0);
insert into [$(bingo)].CONFIG values(0, 'stereochemistry-detect-haworth-projection', 0);
insert into [$(bingo)].CONFIG values(0, 'reject-invalid-structures', 0);
insert into [$(bingo)].CONFIG values(0, 'ignore-bad-valence', 0);
go

insert into [$(bingo)].CONFIG values(0, 'FP_ORD_SIZE', '25');
insert into [$(bingo)].CONFIG values(0, 'FP_ANY_SIZE', '15');
insert into [$(bingo)].CONFIG values(0, 'FP_TAU_SIZE', '10');
insert into [$(bingo)].CONFIG values(0, 'FP_SIM_SIZE', '8');
insert into [$(bingo)].CONFIG values(0, 'SUB_SCREENING_MAX_BITS', '8');
insert into [$(bingo)].CONFIG values(0, 'KEEP_CACHE', '0');
insert into [$(bingo)].CONFIG values(0, 'SIM_SCREENING_PASS_MARK', '128');
insert into [$(bingo)].CONFIG values(0, 'SIMILARITY_TYPE', 'SIM');
insert into [$(bingo)].CONFIG values(0, 'CT_FORMAT_SAVE_DATE', '1');
insert into [$(bingo)].CONFIG values(0, 'CT_FORMAT_MODE', 'AUTO');
go

create table [$(bingo)].CONFIG_BIN (n int not null, name varchar(100) not null, value varbinary(max), primary key(n, name));
create index CONFIG_BIN_N on [$(bingo)].CONFIG_BIN(n); 
go

-- Create context
create table [$(bingo)].CONTEXT (obj_id int not null, database_id int not null, full_table_name varchar(100), id_column varchar(100), data_column varchar(100), type varchar(100), primary key (obj_id, database_id));
create index CONTEXT_ID on [$(bingo)].CONTEXT(obj_id);
go

-- Create table with tautomer rules
create table [$(bingo)].TAUTOMER_RULES (id int identity primary key, begg varchar(100), endd varchar(100));
insert into [$(bingo)].TAUTOMER_RULES(begg, endd) values ('N,O,P,S,As,Se,Sb,Te', 'N,O,P,S,As,Se,Sb,Te');
insert into [$(bingo)].TAUTOMER_RULES(begg, endd) values ('0C', 'N,O,P,S');
insert into [$(bingo)].TAUTOMER_RULES(begg, endd) values ('1C', 'N,O');
go
 
-- Create roles
CREATE ROLE $(bingo)_reader;
GO
CREATE ROLE $(bingo)_operator;
GO

-- Operator role inherts reader role
EXEC sp_addrolemember $(bingo)_reader, $(bingo)_operator
GO

--
-- Create assembly
--
CREATE ASSEMBLY $(bingo)_assembly from '$(bingo_assembly_path).dll' WITH PERMISSION_SET = UNSAFE;
GO

--
-- Create functions and procedures
--

:r bingo_create_methods.sql

--
-- Create notification queue for OnSessionClose()
--
SET quoted_identifier on;

GO
CREATE PROCEDURE [$(bingo)].log_events
AS
SET NOCOUNT ON;
DECLARE     @message_body XML,
            @message_type_name NVARCHAR(256),
            @dialog UNIQUEIDENTIFIER

--  This procedure continues to process messages in the queue until the queue is empty.
WHILE (1 = 1)
BEGIN
	BEGIN TRANSACTION ;

	-- Receive the next available message
	WAITFOR (
		RECEIVE TOP(1) -- just handle one message at a time
			@message_type_name=message_type_name,  --the type of message received
			@message_body=message_body,      -- the message contents
			@dialog = conversation_handle    -- the identifier of the dialog this message was received on
			FROM [$(bingo)].notify_queue
	), TIMEOUT 1000; -- if the queue is empty for one seconds, give up and go away

	-- If RECEIVE did not return a message, roll back the transaction
	-- and break out of the while loop, exiting the procedure.
	IF (@@ROWCOUNT = 0)
		BEGIN
			ROLLBACK TRANSACTION ;
			BREAK ;
		END ;

	-- Check to see if the message is an end dialog message.
	IF (@message_type_name = 'http://schemas.microsoft.com/SQL/ServiceBroker/EndDialog')
	BEGIN
		END CONVERSATION @dialog ;
	END ;
	ELSE
	BEGIN
		DECLARE	@event_type NVARCHAR(max);
		SET @event_type = CAST(@message_body.query('/EVENT_INSTANCE/EventType/text()') AS nvarchar(max));
		
		-- Release session 
		IF (@event_type = 'AUDIT_LOGOUT')
		BEGIN
			DECLARE	@spid nvarchar(max);
			SET @spid = @message_body.value('(/EVENT_INSTANCE/SPID)[1]', 'int')
			EXECUTE [$(bingo)].OnSessionClose @spid;
		END
		
		-- Delete Bingo index if table has been deleted
		IF (@event_type = 'OBJECT_DELETED')
		BEGIN
			DECLARE	@obj_id int, @database_id int;
			
			SET @obj_id = @message_body.value('(/EVENT_INSTANCE/ObjectID)[1]', 'int')
			SET @database_id = @message_body.value('(/EVENT_INSTANCE/DatabaseID)[1]', 'int')

			EXECUTE [$(bingo)]._DropIndexByID @obj_id, @database_id
		END
	END ;
	COMMIT TRANSACTION ;
END ;
GO

create queue [$(bingo)].notify_queue with activation (
 status = on, procedure_name = [$(bingo)].log_events, max_queue_readers = 1, execute as self);
GO

create service $(bingo)_notify_service on queue [$(bingo)].notify_queue 
(
[http://schemas.microsoft.com/SQL/Notifications/PostEventNotification] 
);
GO

-- ALTER AUTHORIZATION ON SERVICE::$(bingo)_notify_service TO $(bingo)
-- go

CREATE ROUTE $(bingo)_notify_route AUTHORIZATION dbo
WITH SERVICE_NAME = N'$(bingo)_notify_service', ADDRESS = N'LOCAL';
GO

-- Creating server-global triggers and events

IF (select count(*) from sys.server_event_notifications where name = '$(bingo)_$(database)_logout_notify') = 1
	-- event notification for the database may exist when db was restored from backup with different service_broker_guid 
	drop event notification $(bingo)_$(database)_logout_notify on server;
GO

create event notification $(bingo)_$(database)_logout_notify on server for
	AUDIT_LOGOUT, OBJECT_DELETED to service '$(bingo)_notify_service', 'current database';
GO

-- Trigger to prevent manual database drop operation with Bingo installed
if (select count(*) from sys.server_triggers where name = '$(bingo)_$(database)_prevent_db_drop') = 1
	DROP TRIGGER $(bingo)_$(database)_prevent_db_drop ON ALL SERVER;
GO

CREATE TRIGGER $(bingo)_$(database)_prevent_db_drop
ON ALL SERVER
FOR DROP_DATABASE
AS
	DECLARE @database_name NVARCHAR(128),
			@event_data XML
	SET @event_data = EVENTDATA()
	SELECT @database_name = @event_data.value('(/EVENT_INSTANCE/DatabaseName)[1]', 'nvarchar(128)')

	IF @database_name = '$(database)'
	BEGIN
		PRINT 'You must uninstall Bingo first to drop database!'
		ROLLBACK;
	END
GO

PRINT 'Done.'
GO
