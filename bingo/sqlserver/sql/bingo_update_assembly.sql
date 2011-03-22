use $(database)
go

BEGIN TRY
   exec [$(bingo)]._FlushInAllSessions;
END TRY
BEGIN CATCH
END CATCH;
GO

ALTER ASSEMBLY $(bingo)_assembly
DROP FILE ALL 
GO

ALTER ASSEMBLY $(bingo)_assembly from '$(bingo_assembly_path).dll' WITH PERMISSION_SET = UNSAFE;
GO

ALTER ASSEMBLY $(bingo)_assembly
ADD FILE FROM '$(bingo_assembly_path).pdb'
GO

