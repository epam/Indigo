use $(database)

exec [$(bingo)]._DropAllIndices
go

:r bingo_drop_methods.sql

drop procedure [$(bingo)].log_events
go

drop event notification $(bingo)_$(database)_logout_notify on server;
go

if (select count(*) from sys.server_triggers where name = '$(bingo)_$(database)_prevent_db_drop') = 1
	DROP TRIGGER $(bingo)_$(database)_prevent_db_drop ON ALL SERVER;
go

drop route $(bingo)_notify_route;
go

drop service $(bingo)_notify_service;
go

drop queue [$(bingo)].notify_queue;
go
                     
drop table [$(bingo)].CONFIG
go

drop table [$(bingo)].CONFIG_BIN
go

drop table [$(bingo)].CONTEXT;
go

drop table [$(bingo)].TAUTOMER_RULES
go

DROP ROLE $(bingo)_operator;
GO

DROP ROLE $(bingo)_reader;
GO

DROP SCHEMA $(bingo);
GO

DROP USER $(bingo);
GO

DROP CERTIFICATE $(bingo)_certificate;
GO

DROP ASSEMBLY $(bingo)_assembly;
GO


use master;

IF ($(fulldelete) = 1)
BEGIN
	DROP LOGIN $(bingo)_assembly_login;
	DROP ASYMMETRIC KEY bingo_assembly_key;
END
GO
