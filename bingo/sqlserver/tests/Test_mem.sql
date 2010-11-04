sp_configure 'max server memory', 4096;
GO
RECONFIGURE;
GO

USE master
EXEC sp_configure 'max server memory (MB)'

USE master
EXEC sp_configure 'max server memory (MB)', 640000
RECONFIGURE WITH OVERRIDE


exec bingo._WriteLog 'hello!';

use mols;
exec bingo._CheckMemoryAllocate 150, 151;

exec bingo._ForceGC;

select single_pages_kb + multi_pages_kb + 
	virtual_memory_committed_kb from sys.dm_os_memory_clerks 
	where type = 'MEMORYCLERK_SQLCLR'
	
	
-- exec bingo._CheckCreateMoleculeIndex 'acd2d', 'id', 'molfile'
