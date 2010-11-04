--
-- This file was generated automatically --
--

use $(database)
go

--
-- _CheckMemoryAllocate
--
CREATE PROCEDURE [$(bingo)].__CheckMemoryAllocate 
    @dotnet_size_mb int,
    @block_size_mb int,
    @core_size_mb int,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._CheckMemoryAllocate
GO
ADD SIGNATURE TO [$(bingo)].__CheckMemoryAllocate BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._CheckMemoryAllocate 
    @dotnet_size_mb int,
    @block_size_mb int,
    @core_size_mb int
AS
BEGIN
  EXEC [$(bingo)].__CheckMemoryAllocate @dotnet_size_mb, @block_size_mb, @core_size_mb, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)]._CheckMemoryAllocate BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)]._CheckMemoryAllocate to $(bingo)_operator
GO

--
-- _DropAllIndices
--
CREATE PROCEDURE [$(bingo)].__DropAllIndices 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._DropAllIndices
GO
ADD SIGNATURE TO [$(bingo)].__DropAllIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._DropAllIndices 
AS
BEGIN
  EXEC [$(bingo)].__DropAllIndices '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)]._DropAllIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- _FlushInAllSessions
--
CREATE PROCEDURE [$(bingo)].__FlushInAllSessions 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._FlushInAllSessions
GO
ADD SIGNATURE TO [$(bingo)].__FlushInAllSessions BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._FlushInAllSessions 
AS
BEGIN
  EXEC [$(bingo)].__FlushInAllSessions '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)]._FlushInAllSessions BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- _ForceGC
--
CREATE PROCEDURE [$(bingo)]._ForceGC 
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils]._ForceGC
GO
ADD SIGNATURE TO [$(bingo)]._ForceGC BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- _OnDeleteRecordTrigger
--
CREATE PROCEDURE [$(bingo)].__OnDeleteRecordTrigger 
    @full_table_name nvarchar(max),
    @id int,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._OnDeleteRecordTrigger
GO
ADD SIGNATURE TO [$(bingo)].__OnDeleteRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._OnDeleteRecordTrigger 
    @full_table_name nvarchar(max),
    @id int
AS
BEGIN
  EXEC [$(bingo)].__OnDeleteRecordTrigger @full_table_name, @id, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)]._OnDeleteRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)]._OnDeleteRecordTrigger to $(bingo)_operator
GO

--
-- _OnInsertRecordTrigger
--
CREATE PROCEDURE [$(bingo)].__OnInsertRecordTrigger 
    @full_table_name nvarchar(max),
    @id int,
    @data nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._OnInsertRecordTrigger
GO
ADD SIGNATURE TO [$(bingo)].__OnInsertRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._OnInsertRecordTrigger 
    @full_table_name nvarchar(max),
    @id int,
    @data nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].__OnInsertRecordTrigger @full_table_name, @id, @data, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)]._OnInsertRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)]._OnInsertRecordTrigger to $(bingo)_operator
GO

--
-- _UnloadLibrary
--
CREATE PROCEDURE [$(bingo)]._UnloadLibrary 
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoDll]._UnloadLibrary
GO
ADD SIGNATURE TO [$(bingo)]._UnloadLibrary BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- _WriteLog
--
CREATE PROCEDURE [$(bingo)]._WriteLog 
    @message nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoLog]._WriteLog
GO
ADD SIGNATURE TO [$(bingo)]._WriteLog BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- AAM
--
CREATE FUNCTION [$(bingo)]._AAM 
  (
    @reaction nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].AAM
GO
ADD SIGNATURE TO [$(bingo)]._AAM BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].AAM 
  (
    @reaction nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._AAM (@reaction, @options, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].AAM BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].AAM to $(bingo)_reader
GO

--
-- CanSmiles
--
CREATE FUNCTION [$(bingo)]._CanSmiles 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CanSmiles
GO
ADD SIGNATURE TO [$(bingo)]._CanSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CanSmiles 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._CanSmiles (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].CanSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].CanSmiles to $(bingo)_reader
GO

--
-- CheckMolecule
--
CREATE FUNCTION [$(bingo)]._CheckMolecule 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CheckMolecule
GO
ADD SIGNATURE TO [$(bingo)]._CheckMolecule BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CheckMolecule 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._CheckMolecule (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].CheckMolecule BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].CheckMolecule to $(bingo)_reader
GO

--
-- CheckReaction
--
CREATE FUNCTION [$(bingo)]._CheckReaction 
  (
    @reaction nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CheckReaction
GO
ADD SIGNATURE TO [$(bingo)]._CheckReaction BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CheckReaction 
  (
    @reaction nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._CheckReaction (@reaction, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].CheckReaction BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].CheckReaction to $(bingo)_reader
GO

--
-- CreateMoleculeIndex
--
CREATE PROCEDURE [$(bingo)]._CreateMoleculeIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CreateMoleculeIndex
GO
ADD SIGNATURE TO [$(bingo)]._CreateMoleculeIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].CreateMoleculeIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._CreateMoleculeIndex @table, @id_column, @data_column, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].CreateMoleculeIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].CreateMoleculeIndex to $(bingo)_operator
GO

--
-- CreateReactionIndex
--
CREATE PROCEDURE [$(bingo)]._CreateReactionIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CreateReactionIndex
GO
ADD SIGNATURE TO [$(bingo)]._CreateReactionIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].CreateReactionIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._CreateReactionIndex @table, @id_column, @data_column, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].CreateReactionIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].CreateReactionIndex to $(bingo)_operator
GO

--
-- DropIndex
--
CREATE PROCEDURE [$(bingo)]._DropIndex 
    @table nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].DropIndex
GO
ADD SIGNATURE TO [$(bingo)]._DropIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].DropIndex 
    @table nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._DropIndex @table, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].DropIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].DropIndex to $(bingo)_operator
GO

--
-- DropInvalidIndices
--
CREATE PROCEDURE [$(bingo)]._DropInvalidIndices 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].DropInvalidIndices
GO
ADD SIGNATURE TO [$(bingo)]._DropInvalidIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].DropInvalidIndices 
AS
BEGIN
  EXEC [$(bingo)]._DropInvalidIndices '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].DropInvalidIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].DropInvalidIndices to $(bingo)_operator
GO

--
-- Exact
--
CREATE FUNCTION [$(bingo)]._Exact 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Exact
GO
ADD SIGNATURE TO [$(bingo)]._Exact BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Exact 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._Exact (@target, @query, @options, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Exact BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Exact to $(bingo)_reader
GO

--
-- FlushOperations
--
CREATE PROCEDURE [$(bingo)]._FlushOperations 
    @table_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].FlushOperations
GO
ADD SIGNATURE TO [$(bingo)]._FlushOperations BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].FlushOperations 
    @table_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._FlushOperations @table_name, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].FlushOperations BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].FlushOperations to $(bingo)_operator
GO

--
-- GetAtomCount
--
CREATE FUNCTION [$(bingo)]._GetAtomCount 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetAtomCount
GO
ADD SIGNATURE TO [$(bingo)]._GetAtomCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetAtomCount 
  (
    @molecule nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._GetAtomCount (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].GetAtomCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].GetAtomCount to $(bingo)_operator
GO

--
-- GetBondCount
--
CREATE FUNCTION [$(bingo)]._GetBondCount 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetBondCount
GO
ADD SIGNATURE TO [$(bingo)]._GetBondCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetBondCount 
  (
    @molecule nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._GetBondCount (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].GetBondCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].GetBondCount to $(bingo)_operator
GO

--
-- GetStatistics
--
CREATE FUNCTION [$(bingo)]._GetStatistics 
  (
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetStatistics
GO
ADD SIGNATURE TO [$(bingo)]._GetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetStatistics 
  (
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._GetStatistics ('$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].GetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].GetStatistics to $(bingo)_operator
GO

--
-- GetVersion
--
CREATE FUNCTION [$(bingo)]._GetVersion 
  (
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetVersion
GO
ADD SIGNATURE TO [$(bingo)]._GetVersion BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetVersion 
  (
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._GetVersion ('$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].GetVersion BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].GetVersion to $(bingo)_reader
GO

--
-- Gross
--
CREATE FUNCTION [$(bingo)]._Gross 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Gross
GO
ADD SIGNATURE TO [$(bingo)]._Gross BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Gross 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._Gross (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Gross BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Gross to $(bingo)_reader
GO

--
-- ImportRDF
--
CREATE PROCEDURE [$(bingo)]._ImportRDF 
    @table_name nvarchar(max),
    @react_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportRDF
GO
ADD SIGNATURE TO [$(bingo)]._ImportRDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportRDF 
    @table_name nvarchar(max),
    @react_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._ImportRDF @table_name, @react_column_name, @file_name, @additional_parameters, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].ImportRDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ImportRDF to $(bingo)_reader
GO

--
-- ImportSDF
--
CREATE PROCEDURE [$(bingo)]._ImportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportSDF
GO
ADD SIGNATURE TO [$(bingo)]._ImportSDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._ImportSDF @table_name, @mol_column_name, @file_name, @additional_parameters, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].ImportSDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ImportSDF to $(bingo)_reader
GO

--
-- ImportSMILES
--
CREATE PROCEDURE [$(bingo)]._ImportSMILES 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @id_column_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportSMILES
GO
ADD SIGNATURE TO [$(bingo)]._ImportSMILES BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportSMILES 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @id_column_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)]._ImportSMILES @table_name, @mol_column_name, @file_name, @id_column_name, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].ImportSMILES BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ImportSMILES to $(bingo)_reader
GO

--
-- Mass
--
CREATE FUNCTION [$(bingo)]._Mass 
  (
    @molecule nvarchar(max),
    @type nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Mass
GO
ADD SIGNATURE TO [$(bingo)]._Mass BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Mass 
  (
    @molecule nvarchar(max),
    @type nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)]._Mass (@molecule, @type, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Mass BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Mass to $(bingo)_reader
GO

--
-- Molfile
--
CREATE FUNCTION [$(bingo)]._Molfile 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Molfile
GO
ADD SIGNATURE TO [$(bingo)]._Molfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Molfile 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._Molfile (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Molfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Molfile to $(bingo)_reader
GO

--
-- Name
--
CREATE FUNCTION [$(bingo)]._Name 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Name
GO
ADD SIGNATURE TO [$(bingo)]._Name BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Name 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._Name (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Name BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Name to $(bingo)_reader
GO

--
-- OnSessionClose
--
CREATE PROCEDURE [$(bingo)].OnSessionClose 
    @spid_str nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].OnSessionClose
GO
ADD SIGNATURE TO [$(bingo)].OnSessionClose BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

--
-- ProfilingGetCount
--
CREATE FUNCTION [$(bingo)]._ProfilingGetCount 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS bigint
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetCount
GO
ADD SIGNATURE TO [$(bingo)]._ProfilingGetCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].ProfilingGetCount 
  (
    @counter_name nvarchar(max),
    @whole_session bit
  )
  RETURNS bigint
AS
BEGIN
  RETURN [$(bingo)]._ProfilingGetCount (@counter_name, @whole_session, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].ProfilingGetCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ProfilingGetCount to $(bingo)_operator
GO

--
-- ProfilingGetTime
--
CREATE FUNCTION [$(bingo)]._ProfilingGetTime 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetTime
GO
ADD SIGNATURE TO [$(bingo)]._ProfilingGetTime BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].ProfilingGetTime 
  (
    @counter_name nvarchar(max),
    @whole_session bit
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)]._ProfilingGetTime (@counter_name, @whole_session, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].ProfilingGetTime BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ProfilingGetTime to $(bingo)_operator
GO

--
-- ProfilingGetValue
--
CREATE FUNCTION [$(bingo)]._ProfilingGetValue 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS bigint
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetValue
GO
ADD SIGNATURE TO [$(bingo)]._ProfilingGetValue BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].ProfilingGetValue 
  (
    @counter_name nvarchar(max),
    @whole_session bit
  )
  RETURNS bigint
AS
BEGIN
  RETURN [$(bingo)]._ProfilingGetValue (@counter_name, @whole_session, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].ProfilingGetValue BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ProfilingGetValue to $(bingo)_operator
GO

--
-- ReadFileAsBinary
--
CREATE FUNCTION [$(bingo)].ReadFileAsBinary 
  (
    @filename nvarchar(max)
  )
  RETURNS varbinary(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils].ReadFileAsBinary
GO
ADD SIGNATURE TO [$(bingo)].ReadFileAsBinary BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ReadFileAsBinary to $(bingo)_operator
GO

--
-- ReadFileAsText
--
CREATE FUNCTION [$(bingo)].ReadFileAsText 
  (
    @filename nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils].ReadFileAsText
GO
ADD SIGNATURE TO [$(bingo)].ReadFileAsText BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ReadFileAsText to $(bingo)_operator
GO

--
-- ResetStatistics
--
CREATE PROCEDURE [$(bingo)]._ResetStatistics 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ResetStatistics
GO
ADD SIGNATURE TO [$(bingo)]._ResetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ResetStatistics 
AS
BEGIN
  EXEC [$(bingo)]._ResetStatistics '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].ResetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].ResetStatistics to $(bingo)_operator
GO

--
-- RSmiles
--
CREATE FUNCTION [$(bingo)]._RSmiles 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSmiles
GO
ADD SIGNATURE TO [$(bingo)]._RSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSmiles 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._RSmiles (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].RSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].RSmiles to $(bingo)_reader
GO

--
-- RSub
--
CREATE FUNCTION [$(bingo)]._RSub 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSub
GO
ADD SIGNATURE TO [$(bingo)]._RSub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSub 
  (
    @target nvarchar(max),
    @query nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._RSub (@target, @query, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].RSub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].RSub to $(bingo)_reader
GO

--
-- RSubHi
--
CREATE FUNCTION [$(bingo)]._RSubHi 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSubHi
GO
ADD SIGNATURE TO [$(bingo)]._RSubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSubHi 
  (
    @target nvarchar(max),
    @query nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._RSubHi (@target, @query, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].RSubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].RSubHi to $(bingo)_reader
GO

--
-- Rxnfile
--
CREATE FUNCTION [$(bingo)]._Rxnfile 
  (
    @reaction nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Rxnfile
GO
ADD SIGNATURE TO [$(bingo)]._Rxnfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Rxnfile 
  (
    @reaction nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._Rxnfile (@reaction, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Rxnfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Rxnfile to $(bingo)_reader
GO

--
-- SearchExact
--
CREATE FUNCTION [$(bingo)]._SearchExact 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchExact
GO
ADD SIGNATURE TO [$(bingo)]._SearchExact BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchExact 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchExact (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchExact to $(bingo)_reader
GO

--
-- SearchGross
--
CREATE FUNCTION [$(bingo)]._SearchGross 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchGross
GO
ADD SIGNATURE TO [$(bingo)]._SearchGross BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchGross 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchGross (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchGross to $(bingo)_reader
GO

--
-- SearchMolecularWeight
--
CREATE FUNCTION [$(bingo)]._SearchMolecularWeight 
  (
    @table nvarchar(max),
    @min_bound float,
    @max_bound float,
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int, weight real)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchMolecularWeight
GO
ADD SIGNATURE TO [$(bingo)]._SearchMolecularWeight BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchMolecularWeight 
  (
    @table nvarchar(max),
    @min_bound float,
    @max_bound float,
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchMolecularWeight (@table, @min_bound, @max_bound, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchMolecularWeight to $(bingo)_reader
GO

--
-- SearchRSub
--
CREATE FUNCTION [$(bingo)]._SearchRSub 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchRSub
GO
ADD SIGNATURE TO [$(bingo)]._SearchRSub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchRSub 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchRSub (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchRSub to $(bingo)_reader
GO

--
-- SearchRSubHi
--
CREATE FUNCTION [$(bingo)]._SearchRSubHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int, highlighting nvarchar(max))
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchRSubHi
GO
ADD SIGNATURE TO [$(bingo)]._SearchRSubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchRSubHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchRSubHi (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchRSubHi to $(bingo)_reader
GO

--
-- SearchSim
--
CREATE FUNCTION [$(bingo)]._SearchSim 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @metric nvarchar(max),
    @bingo_schema nvarchar(max),
    @min_bound float,
    @max_bound float
  )
  RETURNS TABLE (id int, similarity real)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchSim
GO
ADD SIGNATURE TO [$(bingo)]._SearchSim BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchSim 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @metric nvarchar(max),
    @min_bound float,
    @max_bound float
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchSim (@table, @query, @metric, '$(bingo)', @min_bound, @max_bound))
GO

grant select on [$(bingo)].SearchSim to $(bingo)_reader
GO

--
-- SearchSMARTS
--
CREATE FUNCTION [$(bingo)]._SearchSMARTS 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchSMARTS
GO
ADD SIGNATURE TO [$(bingo)]._SearchSMARTS BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchSMARTS 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchSMARTS (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchSMARTS to $(bingo)_reader
GO

--
-- SearchSMARTSHi
--
CREATE FUNCTION [$(bingo)]._SearchSMARTSHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int, highlighting nvarchar(max))
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchSMARTSHi
GO
ADD SIGNATURE TO [$(bingo)]._SearchSMARTSHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchSMARTSHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchSMARTSHi (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchSMARTSHi to $(bingo)_reader
GO

--
-- SearchSub
--
CREATE FUNCTION [$(bingo)]._SearchSub 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchSub
GO
ADD SIGNATURE TO [$(bingo)]._SearchSub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchSub 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchSub (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchSub to $(bingo)_reader
GO

--
-- SearchSubHi
--
CREATE FUNCTION [$(bingo)]._SearchSubHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int, highlighting nvarchar(max))
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SearchSubHi
GO
ADD SIGNATURE TO [$(bingo)]._SearchSubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SearchSubHi 
  (
    @table nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)]._SearchSubHi (@table, @query, @options, '$(bingo)'))
GO

grant select on [$(bingo)].SearchSubHi to $(bingo)_reader
GO

--
-- SetKeepCache
--
CREATE PROCEDURE [$(bingo)]._SetKeepCache 
    @table nvarchar(max),
    @value bit,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SetKeepCache
GO
ADD SIGNATURE TO [$(bingo)]._SetKeepCache BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].SetKeepCache 
    @table nvarchar(max),
    @value bit
AS
BEGIN
  EXEC [$(bingo)]._SetKeepCache @table, @value, '$(bingo)'
END
GO
ADD SIGNATURE TO [$(bingo)].SetKeepCache BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].SetKeepCache to $(bingo)_operator
GO

--
-- Sim
--
CREATE FUNCTION [$(bingo)]._Sim 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @metrics nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Sim
GO
ADD SIGNATURE TO [$(bingo)]._Sim BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Sim 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @metrics nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)]._Sim (@target, @query, @metrics, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Sim BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Sim to $(bingo)_reader
GO

--
-- SMARTS
--
CREATE FUNCTION [$(bingo)]._SMARTS 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SMARTS
GO
ADD SIGNATURE TO [$(bingo)]._SMARTS BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SMARTS 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._SMARTS (@target, @query, @options, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].SMARTS BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].SMARTS to $(bingo)_reader
GO

--
-- SMARTSHi
--
CREATE FUNCTION [$(bingo)]._SMARTSHi 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SMARTSHi
GO
ADD SIGNATURE TO [$(bingo)]._SMARTSHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SMARTSHi 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._SMARTSHi (@target, @query, @parameters, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].SMARTSHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].SMARTSHi to $(bingo)_reader
GO

--
-- Smiles
--
CREATE FUNCTION [$(bingo)]._Smiles 
  (
    @molecule nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Smiles
GO
ADD SIGNATURE TO [$(bingo)]._Smiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Smiles 
  (
    @molecule nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._Smiles (@molecule, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Smiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Smiles to $(bingo)_reader
GO

--
-- Sub
--
CREATE FUNCTION [$(bingo)]._Sub 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Sub
GO
ADD SIGNATURE TO [$(bingo)]._Sub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Sub 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)]._Sub (@target, @query, @options, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].Sub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].Sub to $(bingo)_reader
GO

--
-- SubHi
--
CREATE FUNCTION [$(bingo)]._SubHi 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SubHi
GO
ADD SIGNATURE TO [$(bingo)]._SubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SubHi 
  (
    @target nvarchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)]._SubHi (@target, @query, @parameters, '$(bingo)')
END
GO
ADD SIGNATURE TO [$(bingo)].SubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

grant execute on [$(bingo)].SubHi to $(bingo)_reader
GO

