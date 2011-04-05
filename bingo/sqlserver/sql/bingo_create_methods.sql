--
-- This file was generated automatically --
--

use $(database)
go

--
-- _CheckMemoryAllocate
--
CREATE PROCEDURE [$(bingo)].z__CheckMemoryAllocate 
    @dotnet_size_mb int,
    @block_size_mb int,
    @core_size_mb int,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._CheckMemoryAllocate
GO
ADD SIGNATURE TO [$(bingo)].z__CheckMemoryAllocate BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._CheckMemoryAllocate 
    @dotnet_size_mb int,
    @block_size_mb int,
    @core_size_mb int
AS
BEGIN
  EXEC [$(bingo)].z__CheckMemoryAllocate @dotnet_size_mb, @block_size_mb, @core_size_mb, '$(bingo)'
END
GO
grant execute on [$(bingo)]._CheckMemoryAllocate to $(bingo)_operator
GO

--
-- _DropAllIndices
--
CREATE PROCEDURE [$(bingo)].z__DropAllIndices 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._DropAllIndices
GO
ADD SIGNATURE TO [$(bingo)].z__DropAllIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._DropAllIndices 
AS
BEGIN
  EXEC [$(bingo)].z__DropAllIndices '$(bingo)'
END
GO
--
-- _DropIndexByID
--
CREATE PROCEDURE [$(bingo)].z__DropIndexByID 
    @table_id int,
    @database_id int,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._DropIndexByID
GO
ADD SIGNATURE TO [$(bingo)].z__DropIndexByID BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._DropIndexByID 
    @table_id int,
    @database_id int
AS
BEGIN
  EXEC [$(bingo)].z__DropIndexByID @table_id, @database_id, '$(bingo)'
END
GO
grant execute on [$(bingo)]._DropIndexByID to $(bingo)_operator
GO

--
-- _FlushInAllSessions
--
CREATE PROCEDURE [$(bingo)].z__FlushInAllSessions 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._FlushInAllSessions
GO
ADD SIGNATURE TO [$(bingo)].z__FlushInAllSessions BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._FlushInAllSessions 
AS
BEGIN
  EXEC [$(bingo)].z__FlushInAllSessions '$(bingo)'
END
GO
--
-- _ForceGC
--
CREATE PROCEDURE [$(bingo)].z__ForceGC 
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils]._ForceGC
GO
ADD SIGNATURE TO [$(bingo)].z__ForceGC BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._ForceGC 
AS
BEGIN
  EXEC [$(bingo)].z__ForceGC 
END
GO
--
-- _OnDeleteRecordTrigger
--
CREATE PROCEDURE [$(bingo)].z__OnDeleteRecordTrigger 
    @table_id int,
    @database_id int,
    @tmp_table_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._OnDeleteRecordTrigger
GO
ADD SIGNATURE TO [$(bingo)].z__OnDeleteRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._OnDeleteRecordTrigger 
    @table_id int,
    @database_id int,
    @tmp_table_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z__OnDeleteRecordTrigger @table_id, @database_id, @tmp_table_name, '$(bingo)'
END
GO
grant execute on [$(bingo)]._OnDeleteRecordTrigger to $(bingo)_operator
GO

--
-- _OnInsertRecordTrigger
--
CREATE PROCEDURE [$(bingo)].z__OnInsertRecordTrigger 
    @table_id int,
    @database_id int,
    @tmp_table_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo]._OnInsertRecordTrigger
GO
ADD SIGNATURE TO [$(bingo)].z__OnInsertRecordTrigger BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._OnInsertRecordTrigger 
    @table_id int,
    @database_id int,
    @tmp_table_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z__OnInsertRecordTrigger @table_id, @database_id, @tmp_table_name, '$(bingo)'
END
GO
grant execute on [$(bingo)]._OnInsertRecordTrigger to $(bingo)_operator
GO

--
-- _WriteLog
--
CREATE PROCEDURE [$(bingo)].z__WriteLog 
    @message nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoLog]._WriteLog
GO
ADD SIGNATURE TO [$(bingo)].z__WriteLog BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)]._WriteLog 
    @message nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z__WriteLog @message
END
GO
--
-- AAM
--
CREATE FUNCTION [$(bingo)].z_AAM 
  (
    @reaction varbinary(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].AAM
GO
ADD SIGNATURE TO [$(bingo)].z_AAM BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].AAM 
  (
    @reaction varchar(max),
    @options nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_AAM (cast(@reaction as VARBINARY(max)), @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].AAM to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].AAMB 
  (
    @reaction varbinary(max),
    @options nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_AAM (@reaction, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].AAMB to $(bingo)_reader
GO

--
-- CanSmiles
--
CREATE FUNCTION [$(bingo)].z_CanSmiles 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CanSmiles
GO
ADD SIGNATURE TO [$(bingo)].z_CanSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CanSmiles 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CanSmiles (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].CanSmiles to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].CanSmilesB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CanSmiles (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].CanSmilesB to $(bingo)_reader
GO

--
-- CheckMolecule
--
CREATE FUNCTION [$(bingo)].z_CheckMolecule 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CheckMolecule
GO
ADD SIGNATURE TO [$(bingo)].z_CheckMolecule BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CheckMolecule 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CheckMolecule (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].CheckMolecule to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].CheckMoleculeB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CheckMolecule (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].CheckMoleculeB to $(bingo)_reader
GO

--
-- CheckMoleculeTable
--
CREATE FUNCTION [$(bingo)].z_CheckMoleculeTable 
  (
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS TABLE (id int, msg nvarchar(max))
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CheckMoleculeTable
GO
ADD SIGNATURE TO [$(bingo)].z_CheckMoleculeTable BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CheckMoleculeTable 
  (
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max)
  )
  RETURNS TABLE
AS
  RETURN (SELECT * FROM [$(bingo)].z_CheckMoleculeTable (@table, @id_column, @data_column, '$(bingo)'))

GO
grant select on [$(bingo)].CheckMoleculeTable to $(bingo)_reader
GO

--
-- CheckReaction
--
CREATE FUNCTION [$(bingo)].z_CheckReaction 
  (
    @reaction varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CheckReaction
GO
ADD SIGNATURE TO [$(bingo)].z_CheckReaction BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CheckReaction 
  (
    @reaction varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CheckReaction (cast(@reaction as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].CheckReaction to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].CheckReactionB 
  (
    @reaction varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_CheckReaction (@reaction, '$(bingo)')
END
GO
grant execute on [$(bingo)].CheckReactionB to $(bingo)_reader
GO

--
-- CompactMolecule
--
CREATE FUNCTION [$(bingo)].z_CompactMolecule 
  (
    @molecule varbinary(max),
    @save_xyz bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS varbinary(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CompactMolecule
GO
ADD SIGNATURE TO [$(bingo)].z_CompactMolecule BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CompactMolecule 
  (
    @molecule varchar(max),
    @save_xyz bit
  )
  RETURNS varbinary(max)
AS
BEGIN
  RETURN [$(bingo)].z_CompactMolecule (cast(@molecule as VARBINARY(max)), @save_xyz, '$(bingo)')
END
GO
grant execute on [$(bingo)].CompactMolecule to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].CompactMoleculeB 
  (
    @molecule varbinary(max),
    @save_xyz bit
  )
  RETURNS varbinary(max)
AS
BEGIN
  RETURN [$(bingo)].z_CompactMolecule (@molecule, @save_xyz, '$(bingo)')
END
GO
grant execute on [$(bingo)].CompactMoleculeB to $(bingo)_reader
GO

--
-- CompactReaction
--
CREATE FUNCTION [$(bingo)].z_CompactReaction 
  (
    @reaction varbinary(max),
    @save_xyz bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS varbinary(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CompactReaction
GO
ADD SIGNATURE TO [$(bingo)].z_CompactReaction BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].CompactReaction 
  (
    @reaction varchar(max),
    @save_xyz bit
  )
  RETURNS varbinary(max)
AS
BEGIN
  RETURN [$(bingo)].z_CompactReaction (cast(@reaction as VARBINARY(max)), @save_xyz, '$(bingo)')
END
GO
grant execute on [$(bingo)].CompactReaction to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].CompactReactionB 
  (
    @reaction varbinary(max),
    @save_xyz bit
  )
  RETURNS varbinary(max)
AS
BEGIN
  RETURN [$(bingo)].z_CompactReaction (@reaction, @save_xyz, '$(bingo)')
END
GO
grant execute on [$(bingo)].CompactReactionB to $(bingo)_reader
GO

--
-- CreateMoleculeIndex
--
CREATE PROCEDURE [$(bingo)].z_CreateMoleculeIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CreateMoleculeIndex
GO
ADD SIGNATURE TO [$(bingo)].z_CreateMoleculeIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].CreateMoleculeIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_CreateMoleculeIndex @table, @id_column, @data_column, '$(bingo)'
END
GO
grant execute on [$(bingo)].CreateMoleculeIndex to $(bingo)_operator
GO

--
-- CreateReactionIndex
--
CREATE PROCEDURE [$(bingo)].z_CreateReactionIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].CreateReactionIndex
GO
ADD SIGNATURE TO [$(bingo)].z_CreateReactionIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].CreateReactionIndex 
    @table nvarchar(max),
    @id_column nvarchar(max),
    @data_column nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_CreateReactionIndex @table, @id_column, @data_column, '$(bingo)'
END
GO
grant execute on [$(bingo)].CreateReactionIndex to $(bingo)_operator
GO

--
-- DropIndex
--
CREATE PROCEDURE [$(bingo)].z_DropIndex 
    @table nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].DropIndex
GO
ADD SIGNATURE TO [$(bingo)].z_DropIndex BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].DropIndex 
    @table nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_DropIndex @table, '$(bingo)'
END
GO
grant execute on [$(bingo)].DropIndex to $(bingo)_operator
GO

--
-- DropInvalidIndices
--
CREATE PROCEDURE [$(bingo)].z_DropInvalidIndices 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].DropInvalidIndices
GO
ADD SIGNATURE TO [$(bingo)].z_DropInvalidIndices BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].DropInvalidIndices 
AS
BEGIN
  EXEC [$(bingo)].z_DropInvalidIndices '$(bingo)'
END
GO
grant execute on [$(bingo)].DropInvalidIndices to $(bingo)_operator
GO

--
-- Exact
--
CREATE FUNCTION [$(bingo)].z_Exact 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Exact
GO
ADD SIGNATURE TO [$(bingo)].z_Exact BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Exact 
  (
    @target varchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_Exact (cast(@target as VARBINARY(max)), @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].Exact to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].ExactB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_Exact (@target, @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].ExactB to $(bingo)_reader
GO

--
-- ExportSDF
--
CREATE PROCEDURE [$(bingo)].z_ExportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ExportSDF
GO
ADD SIGNATURE TO [$(bingo)].z_ExportSDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ExportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_ExportSDF @table_name, @mol_column_name, @file_name, @additional_parameters
END
GO
grant execute on [$(bingo)].ExportSDF to $(bingo)_operator
GO

--
-- FlushOperations
--
CREATE PROCEDURE [$(bingo)].z_FlushOperations 
    @table_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].FlushOperations
GO
ADD SIGNATURE TO [$(bingo)].z_FlushOperations BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].FlushOperations 
    @table_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_FlushOperations @table_name, '$(bingo)'
END
GO
grant execute on [$(bingo)].FlushOperations to $(bingo)_operator
GO

--
-- GetAtomCount
--
CREATE FUNCTION [$(bingo)].z_GetAtomCount 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetAtomCount
GO
ADD SIGNATURE TO [$(bingo)].z_GetAtomCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetAtomCount 
  (
    @molecule varchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_GetAtomCount (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].GetAtomCount to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].GetAtomCountB 
  (
    @molecule varbinary(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_GetAtomCount (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].GetAtomCountB to $(bingo)_reader
GO

--
-- GetBondCount
--
CREATE FUNCTION [$(bingo)].z_GetBondCount 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetBondCount
GO
ADD SIGNATURE TO [$(bingo)].z_GetBondCount BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetBondCount 
  (
    @molecule varchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_GetBondCount (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].GetBondCount to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].GetBondCountB 
  (
    @molecule varbinary(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_GetBondCount (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].GetBondCountB to $(bingo)_reader
GO

--
-- GetStatistics
--
CREATE FUNCTION [$(bingo)].z_GetStatistics 
  (
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetStatistics
GO
ADD SIGNATURE TO [$(bingo)].z_GetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetStatistics 
  (
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_GetStatistics ('$(bingo)')
END
GO
grant execute on [$(bingo)].GetStatistics to $(bingo)_operator
GO

--
-- GetVersion
--
CREATE FUNCTION [$(bingo)].z_GetVersion 
  (
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].GetVersion
GO
ADD SIGNATURE TO [$(bingo)].z_GetVersion BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].GetVersion 
  (
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_GetVersion ('$(bingo)')
END
GO
grant execute on [$(bingo)].GetVersion to $(bingo)_reader
GO

--
-- Gross
--
CREATE FUNCTION [$(bingo)].z_Gross 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Gross
GO
ADD SIGNATURE TO [$(bingo)].z_Gross BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Gross 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Gross (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].Gross to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].GrossB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Gross (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].GrossB to $(bingo)_reader
GO

--
-- ImportRDF
--
CREATE PROCEDURE [$(bingo)].z_ImportRDF 
    @table_name nvarchar(max),
    @react_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportRDF
GO
ADD SIGNATURE TO [$(bingo)].z_ImportRDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportRDF 
    @table_name nvarchar(max),
    @react_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_ImportRDF @table_name, @react_column_name, @file_name, @additional_parameters, '$(bingo)'
END
GO
grant execute on [$(bingo)].ImportRDF to $(bingo)_operator
GO

--
-- ImportSDF
--
CREATE PROCEDURE [$(bingo)].z_ImportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportSDF
GO
ADD SIGNATURE TO [$(bingo)].z_ImportSDF BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportSDF 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @additional_parameters nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_ImportSDF @table_name, @mol_column_name, @file_name, @additional_parameters, '$(bingo)'
END
GO
grant execute on [$(bingo)].ImportSDF to $(bingo)_operator
GO

--
-- ImportSMILES
--
CREATE PROCEDURE [$(bingo)].z_ImportSMILES 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @id_column_name nvarchar(max),
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ImportSMILES
GO
ADD SIGNATURE TO [$(bingo)].z_ImportSMILES BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ImportSMILES 
    @table_name nvarchar(max),
    @mol_column_name nvarchar(max),
    @file_name nvarchar(max),
    @id_column_name nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_ImportSMILES @table_name, @mol_column_name, @file_name, @id_column_name, '$(bingo)'
END
GO
grant execute on [$(bingo)].ImportSMILES to $(bingo)_operator
GO

--
-- Mass
--
CREATE FUNCTION [$(bingo)].z_Mass 
  (
    @molecule varbinary(max),
    @type nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Mass
GO
ADD SIGNATURE TO [$(bingo)].z_Mass BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Mass 
  (
    @molecule varchar(max),
    @type nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)].z_Mass (cast(@molecule as VARBINARY(max)), @type, '$(bingo)')
END
GO
grant execute on [$(bingo)].Mass to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].MassB 
  (
    @molecule varbinary(max),
    @type nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)].z_Mass (@molecule, @type, '$(bingo)')
END
GO
grant execute on [$(bingo)].MassB to $(bingo)_reader
GO

--
-- Molfile
--
CREATE FUNCTION [$(bingo)].z_Molfile 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Molfile
GO
ADD SIGNATURE TO [$(bingo)].z_Molfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Molfile 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Molfile (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].Molfile to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].MolfileB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Molfile (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].MolfileB to $(bingo)_reader
GO

--
-- Name
--
CREATE FUNCTION [$(bingo)].z_Name 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Name
GO
ADD SIGNATURE TO [$(bingo)].z_Name BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Name 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Name (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].Name to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].NameB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Name (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].NameB to $(bingo)_reader
GO

--
-- OnSessionClose
--
CREATE PROCEDURE [$(bingo)].z_OnSessionClose 
    @spid_str nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].OnSessionClose
GO
ADD SIGNATURE TO [$(bingo)].z_OnSessionClose BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].OnSessionClose 
    @spid_str nvarchar(max)
AS
BEGIN
  EXEC [$(bingo)].z_OnSessionClose @spid_str
END
GO
--
-- ProfilingGetCount
--
CREATE FUNCTION [$(bingo)].z_ProfilingGetCount 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS bigint
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetCount
GO
ADD SIGNATURE TO [$(bingo)].z_ProfilingGetCount BY CERTIFICATE $(bingo)_certificate
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
  RETURN [$(bingo)].z_ProfilingGetCount (@counter_name, @whole_session, '$(bingo)')
END
GO
grant execute on [$(bingo)].ProfilingGetCount to $(bingo)_operator
GO

--
-- ProfilingGetTime
--
CREATE FUNCTION [$(bingo)].z_ProfilingGetTime 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetTime
GO
ADD SIGNATURE TO [$(bingo)].z_ProfilingGetTime BY CERTIFICATE $(bingo)_certificate
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
  RETURN [$(bingo)].z_ProfilingGetTime (@counter_name, @whole_session, '$(bingo)')
END
GO
grant execute on [$(bingo)].ProfilingGetTime to $(bingo)_operator
GO

--
-- ProfilingGetValue
--
CREATE FUNCTION [$(bingo)].z_ProfilingGetValue 
  (
    @counter_name nvarchar(max),
    @whole_session bit,
    @bingo_schema nvarchar(max)
  )
  RETURNS bigint
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ProfilingGetValue
GO
ADD SIGNATURE TO [$(bingo)].z_ProfilingGetValue BY CERTIFICATE $(bingo)_certificate
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
  RETURN [$(bingo)].z_ProfilingGetValue (@counter_name, @whole_session, '$(bingo)')
END
GO
grant execute on [$(bingo)].ProfilingGetValue to $(bingo)_operator
GO

--
-- ReadFileAsBinary
--
CREATE FUNCTION [$(bingo)].z_ReadFileAsBinary 
  (
    @filename nvarchar(max)
  )
  RETURNS varbinary(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils].ReadFileAsBinary
GO
ADD SIGNATURE TO [$(bingo)].z_ReadFileAsBinary BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].ReadFileAsBinary 
  (
    @filename nvarchar(max)
  )
  RETURNS varbinary(max)
AS
BEGIN
  RETURN [$(bingo)].z_ReadFileAsBinary (@filename)
END
GO
grant execute on [$(bingo)].ReadFileAsBinary to $(bingo)_operator
GO

--
-- ReadFileAsText
--
CREATE FUNCTION [$(bingo)].z_ReadFileAsText 
  (
    @filename nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.BingoSqlUtils].ReadFileAsText
GO
ADD SIGNATURE TO [$(bingo)].z_ReadFileAsText BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].ReadFileAsText 
  (
    @filename nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_ReadFileAsText (@filename)
END
GO
grant execute on [$(bingo)].ReadFileAsText to $(bingo)_operator
GO

--
-- ResetStatistics
--
CREATE PROCEDURE [$(bingo)].z_ResetStatistics 
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].ResetStatistics
GO
ADD SIGNATURE TO [$(bingo)].z_ResetStatistics BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].ResetStatistics 
AS
BEGIN
  EXEC [$(bingo)].z_ResetStatistics '$(bingo)'
END
GO
grant execute on [$(bingo)].ResetStatistics to $(bingo)_operator
GO

--
-- RSmiles
--
CREATE FUNCTION [$(bingo)].z_RSmiles 
  (
    @reaction varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSmiles
GO
ADD SIGNATURE TO [$(bingo)].z_RSmiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSmiles 
  (
    @reaction varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_RSmiles (cast(@reaction as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].RSmiles to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].RSmilesB 
  (
    @reaction varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_RSmiles (@reaction, '$(bingo)')
END
GO
grant execute on [$(bingo)].RSmilesB to $(bingo)_reader
GO

--
-- RSub
--
CREATE FUNCTION [$(bingo)].z_RSub 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSub
GO
ADD SIGNATURE TO [$(bingo)].z_RSub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSub 
  (
    @target varchar(max),
    @query nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_RSub (cast(@target as VARBINARY(max)), @query, '$(bingo)')
END
GO
grant execute on [$(bingo)].RSub to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].RSubB 
  (
    @target varbinary(max),
    @query nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_RSub (@target, @query, '$(bingo)')
END
GO
grant execute on [$(bingo)].RSubB to $(bingo)_reader
GO

--
-- RSubHi
--
CREATE FUNCTION [$(bingo)].z_RSubHi 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].RSubHi
GO
ADD SIGNATURE TO [$(bingo)].z_RSubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].RSubHi 
  (
    @target varchar(max),
    @query nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_RSubHi (cast(@target as VARBINARY(max)), @query, '$(bingo)')
END
GO
grant execute on [$(bingo)].RSubHi to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].RSubHiB 
  (
    @target varbinary(max),
    @query nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_RSubHi (@target, @query, '$(bingo)')
END
GO
grant execute on [$(bingo)].RSubHiB to $(bingo)_reader
GO

--
-- Rxnfile
--
CREATE FUNCTION [$(bingo)].z_Rxnfile 
  (
    @reaction varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Rxnfile
GO
ADD SIGNATURE TO [$(bingo)].z_Rxnfile BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Rxnfile 
  (
    @reaction varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Rxnfile (cast(@reaction as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].Rxnfile to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].RxnfileB 
  (
    @reaction varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Rxnfile (@reaction, '$(bingo)')
END
GO
grant execute on [$(bingo)].RxnfileB to $(bingo)_reader
GO

--
-- SearchExact
--
CREATE FUNCTION [$(bingo)].z_SearchExact 
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
ADD SIGNATURE TO [$(bingo)].z_SearchExact BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchExact (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchExact to $(bingo)_reader
GO

--
-- SearchGross
--
CREATE FUNCTION [$(bingo)].z_SearchGross 
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
ADD SIGNATURE TO [$(bingo)].z_SearchGross BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchGross (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchGross to $(bingo)_reader
GO

--
-- SearchMolecularWeight
--
CREATE FUNCTION [$(bingo)].z_SearchMolecularWeight 
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
ADD SIGNATURE TO [$(bingo)].z_SearchMolecularWeight BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchMolecularWeight (@table, @min_bound, @max_bound, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchMolecularWeight to $(bingo)_reader
GO

--
-- SearchRSub
--
CREATE FUNCTION [$(bingo)].z_SearchRSub 
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
ADD SIGNATURE TO [$(bingo)].z_SearchRSub BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchRSub (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchRSub to $(bingo)_reader
GO

--
-- SearchRSubHi
--
CREATE FUNCTION [$(bingo)].z_SearchRSubHi 
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
ADD SIGNATURE TO [$(bingo)].z_SearchRSubHi BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchRSubHi (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchRSubHi to $(bingo)_reader
GO

--
-- SearchSim
--
CREATE FUNCTION [$(bingo)].z_SearchSim 
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
ADD SIGNATURE TO [$(bingo)].z_SearchSim BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchSim (@table, @query, @metric, '$(bingo)', @min_bound, @max_bound))

GO
grant select on [$(bingo)].SearchSim to $(bingo)_reader
GO

--
-- SearchSMARTS
--
CREATE FUNCTION [$(bingo)].z_SearchSMARTS 
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
ADD SIGNATURE TO [$(bingo)].z_SearchSMARTS BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchSMARTS (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchSMARTS to $(bingo)_reader
GO

--
-- SearchSMARTSHi
--
CREATE FUNCTION [$(bingo)].z_SearchSMARTSHi 
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
ADD SIGNATURE TO [$(bingo)].z_SearchSMARTSHi BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchSMARTSHi (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchSMARTSHi to $(bingo)_reader
GO

--
-- SearchSub
--
CREATE FUNCTION [$(bingo)].z_SearchSub 
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
ADD SIGNATURE TO [$(bingo)].z_SearchSub BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchSub (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchSub to $(bingo)_reader
GO

--
-- SearchSubHi
--
CREATE FUNCTION [$(bingo)].z_SearchSubHi 
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
ADD SIGNATURE TO [$(bingo)].z_SearchSubHi BY CERTIFICATE $(bingo)_certificate
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
  RETURN (SELECT * FROM [$(bingo)].z_SearchSubHi (@table, @query, @options, '$(bingo)'))

GO
grant select on [$(bingo)].SearchSubHi to $(bingo)_reader
GO

--
-- SetKeepCache
--
CREATE PROCEDURE [$(bingo)].z_SetKeepCache 
    @table nvarchar(max),
    @value bit,
    @bingo_schema nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SetKeepCache
GO
ADD SIGNATURE TO [$(bingo)].z_SetKeepCache BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE PROCEDURE [$(bingo)].SetKeepCache 
    @table nvarchar(max),
    @value bit
AS
BEGIN
  EXEC [$(bingo)].z_SetKeepCache @table, @value, '$(bingo)'
END
GO
grant execute on [$(bingo)].SetKeepCache to $(bingo)_operator
GO

--
-- Sim
--
CREATE FUNCTION [$(bingo)].z_Sim 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @metrics nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS real
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Sim
GO
ADD SIGNATURE TO [$(bingo)].z_Sim BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Sim 
  (
    @target varchar(max),
    @query nvarchar(max),
    @metrics nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)].z_Sim (cast(@target as VARBINARY(max)), @query, @metrics, '$(bingo)')
END
GO
grant execute on [$(bingo)].Sim to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SimB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @metrics nvarchar(max)
  )
  RETURNS real
AS
BEGIN
  RETURN [$(bingo)].z_Sim (@target, @query, @metrics, '$(bingo)')
END
GO
grant execute on [$(bingo)].SimB to $(bingo)_reader
GO

--
-- SMARTS
--
CREATE FUNCTION [$(bingo)].z_SMARTS 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SMARTS
GO
ADD SIGNATURE TO [$(bingo)].z_SMARTS BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SMARTS 
  (
    @target varchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_SMARTS (cast(@target as VARBINARY(max)), @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].SMARTS to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SMARTSB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_SMARTS (@target, @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].SMARTSB to $(bingo)_reader
GO

--
-- SMARTSHi
--
CREATE FUNCTION [$(bingo)].z_SMARTSHi 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @parameters nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SMARTSHi
GO
ADD SIGNATURE TO [$(bingo)].z_SMARTSHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SMARTSHi 
  (
    @target varchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_SMARTSHi (cast(@target as VARBINARY(max)), @query, @parameters, '$(bingo)')
END
GO
grant execute on [$(bingo)].SMARTSHi to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SMARTSHiB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_SMARTSHi (@target, @query, @parameters, '$(bingo)')
END
GO
grant execute on [$(bingo)].SMARTSHiB to $(bingo)_reader
GO

--
-- Smiles
--
CREATE FUNCTION [$(bingo)].z_Smiles 
  (
    @molecule varbinary(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Smiles
GO
ADD SIGNATURE TO [$(bingo)].z_Smiles BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Smiles 
  (
    @molecule varchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Smiles (cast(@molecule as VARBINARY(max)), '$(bingo)')
END
GO
grant execute on [$(bingo)].Smiles to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SmilesB 
  (
    @molecule varbinary(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_Smiles (@molecule, '$(bingo)')
END
GO
grant execute on [$(bingo)].SmilesB to $(bingo)_reader
GO

--
-- Sub
--
CREATE FUNCTION [$(bingo)].z_Sub 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS int
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].Sub
GO
ADD SIGNATURE TO [$(bingo)].z_Sub BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].Sub 
  (
    @target varchar(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_Sub (cast(@target as VARBINARY(max)), @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].Sub to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SubB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @options nvarchar(max)
  )
  RETURNS int
AS
BEGIN
  RETURN [$(bingo)].z_Sub (@target, @query, @options, '$(bingo)')
END
GO
grant execute on [$(bingo)].SubB to $(bingo)_reader
GO

--
-- SubHi
--
CREATE FUNCTION [$(bingo)].z_SubHi 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @parameters nvarchar(max),
    @bingo_schema nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
  EXTERNAL NAME [$(bingo)_assembly].[indigo.Bingo].SubHi
GO
ADD SIGNATURE TO [$(bingo)].z_SubHi BY CERTIFICATE $(bingo)_certificate
  WITH PASSWORD = '$(bingo_pass)'
GO

CREATE FUNCTION [$(bingo)].SubHi 
  (
    @target varchar(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_SubHi (cast(@target as VARBINARY(max)), @query, @parameters, '$(bingo)')
END
GO
grant execute on [$(bingo)].SubHi to $(bingo)_reader
GO

CREATE FUNCTION [$(bingo)].SubHiB 
  (
    @target varbinary(max),
    @query nvarchar(max),
    @parameters nvarchar(max)
  )
  RETURNS nvarchar(max)
AS
BEGIN
  RETURN [$(bingo)].z_SubHi (@target, @query, @parameters, '$(bingo)')
END
GO
grant execute on [$(bingo)].SubHiB to $(bingo)_reader
GO

