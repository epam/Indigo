--
-- This file was generated automatically --
--

use $(database)
go

DROP PROCEDURE [$(bingo)]._CheckMemoryAllocate 
GO

DROP PROCEDURE [$(bingo)].z__CheckMemoryAllocate 
GO

DROP PROCEDURE [$(bingo)]._DropAllIndices 
GO

DROP PROCEDURE [$(bingo)].z__DropAllIndices 
GO

DROP PROCEDURE [$(bingo)]._DropIndexByID 
GO

DROP PROCEDURE [$(bingo)].z__DropIndexByID 
GO

DROP PROCEDURE [$(bingo)]._FlushInAllSessions 
GO

DROP PROCEDURE [$(bingo)].z__FlushInAllSessions 
GO

DROP PROCEDURE [$(bingo)]._ForceGC 
GO

DROP PROCEDURE [$(bingo)].z__ForceGC 
GO

DROP PROCEDURE [$(bingo)]._OnDeleteRecordTrigger 
GO

DROP PROCEDURE [$(bingo)].z__OnDeleteRecordTrigger 
GO

DROP PROCEDURE [$(bingo)]._OnInsertRecordTrigger 
GO

DROP PROCEDURE [$(bingo)].z__OnInsertRecordTrigger 
GO

DROP PROCEDURE [$(bingo)]._WriteLog 
GO

DROP PROCEDURE [$(bingo)].z__WriteLog 
GO

DROP FUNCTION [$(bingo)].AAM 
GO

DROP FUNCTION [$(bingo)].AAMB 
GO

DROP FUNCTION [$(bingo)].z_AAM 
GO

DROP FUNCTION [$(bingo)].CanSmiles 
GO

DROP FUNCTION [$(bingo)].CanSmilesB 
GO

DROP FUNCTION [$(bingo)].z_CanSmiles 
GO

DROP FUNCTION [$(bingo)].CheckMolecule 
GO

DROP FUNCTION [$(bingo)].CheckMoleculeB 
GO

DROP FUNCTION [$(bingo)].z_CheckMolecule 
GO

DROP FUNCTION [$(bingo)].CheckReaction 
GO

DROP FUNCTION [$(bingo)].CheckReactionB 
GO

DROP FUNCTION [$(bingo)].z_CheckReaction 
GO

DROP PROCEDURE [$(bingo)].CreateMoleculeIndex 
GO

DROP PROCEDURE [$(bingo)].z_CreateMoleculeIndex 
GO

DROP PROCEDURE [$(bingo)].CreateReactionIndex 
GO

DROP PROCEDURE [$(bingo)].z_CreateReactionIndex 
GO

DROP PROCEDURE [$(bingo)].DropIndex 
GO

DROP PROCEDURE [$(bingo)].z_DropIndex 
GO

DROP PROCEDURE [$(bingo)].DropInvalidIndices 
GO

DROP PROCEDURE [$(bingo)].z_DropInvalidIndices 
GO

DROP FUNCTION [$(bingo)].Exact 
GO

DROP FUNCTION [$(bingo)].ExactB 
GO

DROP FUNCTION [$(bingo)].z_Exact 
GO

DROP PROCEDURE [$(bingo)].FlushOperations 
GO

DROP PROCEDURE [$(bingo)].z_FlushOperations 
GO

DROP FUNCTION [$(bingo)].GetAtomCount 
GO

DROP FUNCTION [$(bingo)].GetAtomCountB 
GO

DROP FUNCTION [$(bingo)].z_GetAtomCount 
GO

DROP FUNCTION [$(bingo)].GetBondCount 
GO

DROP FUNCTION [$(bingo)].GetBondCountB 
GO

DROP FUNCTION [$(bingo)].z_GetBondCount 
GO

DROP FUNCTION [$(bingo)].GetStatistics 
GO

DROP FUNCTION [$(bingo)].z_GetStatistics 
GO

DROP FUNCTION [$(bingo)].GetVersion 
GO

DROP FUNCTION [$(bingo)].z_GetVersion 
GO

DROP FUNCTION [$(bingo)].Gross 
GO

DROP FUNCTION [$(bingo)].GrossB 
GO

DROP FUNCTION [$(bingo)].z_Gross 
GO

DROP PROCEDURE [$(bingo)].ImportRDF 
GO

DROP PROCEDURE [$(bingo)].z_ImportRDF 
GO

DROP PROCEDURE [$(bingo)].ImportSDF 
GO

DROP PROCEDURE [$(bingo)].z_ImportSDF 
GO

DROP PROCEDURE [$(bingo)].ImportSMILES 
GO

DROP PROCEDURE [$(bingo)].z_ImportSMILES 
GO

DROP FUNCTION [$(bingo)].Mass 
GO

DROP FUNCTION [$(bingo)].MassB 
GO

DROP FUNCTION [$(bingo)].z_Mass 
GO

DROP FUNCTION [$(bingo)].Molfile 
GO

DROP FUNCTION [$(bingo)].MolfileB 
GO

DROP FUNCTION [$(bingo)].z_Molfile 
GO

DROP FUNCTION [$(bingo)].Name 
GO

DROP FUNCTION [$(bingo)].NameB 
GO

DROP FUNCTION [$(bingo)].z_Name 
GO

DROP PROCEDURE [$(bingo)].OnSessionClose 
GO

DROP PROCEDURE [$(bingo)].z_OnSessionClose 
GO

DROP FUNCTION [$(bingo)].ProfilingGetCount 
GO

DROP FUNCTION [$(bingo)].z_ProfilingGetCount 
GO

DROP FUNCTION [$(bingo)].ProfilingGetTime 
GO

DROP FUNCTION [$(bingo)].z_ProfilingGetTime 
GO

DROP FUNCTION [$(bingo)].ProfilingGetValue 
GO

DROP FUNCTION [$(bingo)].z_ProfilingGetValue 
GO

DROP FUNCTION [$(bingo)].ReadFileAsBinary 
GO

DROP FUNCTION [$(bingo)].z_ReadFileAsBinary 
GO

DROP FUNCTION [$(bingo)].ReadFileAsText 
GO

DROP FUNCTION [$(bingo)].z_ReadFileAsText 
GO

DROP PROCEDURE [$(bingo)].ResetStatistics 
GO

DROP PROCEDURE [$(bingo)].z_ResetStatistics 
GO

DROP FUNCTION [$(bingo)].RSmiles 
GO

DROP FUNCTION [$(bingo)].RSmilesB 
GO

DROP FUNCTION [$(bingo)].z_RSmiles 
GO

DROP FUNCTION [$(bingo)].RSub 
GO

DROP FUNCTION [$(bingo)].RSubB 
GO

DROP FUNCTION [$(bingo)].z_RSub 
GO

DROP FUNCTION [$(bingo)].RSubHi 
GO

DROP FUNCTION [$(bingo)].RSubHiB 
GO

DROP FUNCTION [$(bingo)].z_RSubHi 
GO

DROP FUNCTION [$(bingo)].Rxnfile 
GO

DROP FUNCTION [$(bingo)].RxnfileB 
GO

DROP FUNCTION [$(bingo)].z_Rxnfile 
GO

DROP FUNCTION [$(bingo)].SearchExact 
GO

DROP FUNCTION [$(bingo)].z_SearchExact 
GO

DROP FUNCTION [$(bingo)].SearchGross 
GO

DROP FUNCTION [$(bingo)].z_SearchGross 
GO

DROP FUNCTION [$(bingo)].SearchMolecularWeight 
GO

DROP FUNCTION [$(bingo)].z_SearchMolecularWeight 
GO

DROP FUNCTION [$(bingo)].SearchRSub 
GO

DROP FUNCTION [$(bingo)].z_SearchRSub 
GO

DROP FUNCTION [$(bingo)].SearchRSubHi 
GO

DROP FUNCTION [$(bingo)].z_SearchRSubHi 
GO

DROP FUNCTION [$(bingo)].SearchSim 
GO

DROP FUNCTION [$(bingo)].z_SearchSim 
GO

DROP FUNCTION [$(bingo)].SearchSMARTS 
GO

DROP FUNCTION [$(bingo)].z_SearchSMARTS 
GO

DROP FUNCTION [$(bingo)].SearchSMARTSHi 
GO

DROP FUNCTION [$(bingo)].z_SearchSMARTSHi 
GO

DROP FUNCTION [$(bingo)].SearchSub 
GO

DROP FUNCTION [$(bingo)].z_SearchSub 
GO

DROP FUNCTION [$(bingo)].SearchSubHi 
GO

DROP FUNCTION [$(bingo)].z_SearchSubHi 
GO

DROP PROCEDURE [$(bingo)].SetKeepCache 
GO

DROP PROCEDURE [$(bingo)].z_SetKeepCache 
GO

DROP FUNCTION [$(bingo)].Sim 
GO

DROP FUNCTION [$(bingo)].SimB 
GO

DROP FUNCTION [$(bingo)].z_Sim 
GO

DROP FUNCTION [$(bingo)].SMARTS 
GO

DROP FUNCTION [$(bingo)].SMARTSB 
GO

DROP FUNCTION [$(bingo)].z_SMARTS 
GO

DROP FUNCTION [$(bingo)].SMARTSHi 
GO

DROP FUNCTION [$(bingo)].SMARTSHiB 
GO

DROP FUNCTION [$(bingo)].z_SMARTSHi 
GO

DROP FUNCTION [$(bingo)].Smiles 
GO

DROP FUNCTION [$(bingo)].SmilesB 
GO

DROP FUNCTION [$(bingo)].z_Smiles 
GO

DROP FUNCTION [$(bingo)].Sub 
GO

DROP FUNCTION [$(bingo)].SubB 
GO

DROP FUNCTION [$(bingo)].z_Sub 
GO

DROP FUNCTION [$(bingo)].SubHi 
GO

DROP FUNCTION [$(bingo)].SubHiB 
GO

DROP FUNCTION [$(bingo)].z_SubHi 
GO

