use sandbox_db2;

CREATE USER test WITHOUT LOGIN;
CREATE USER test_op WITHOUT LOGIN;

EXEC sp_addrolemember bingo_reader, test
EXEC sp_addrolemember bingo_operator, test_op

REVERT
EXEC sp_droprolemember bingo_reader, test
EXEC sp_droprolemember bingo_operator, test_op

grant select on molecules to public;
grant insert on molecules to test_op;

exec sp_who @@spid;

grant select on mols to test;
grant select on mols to test_op;

select [bingo].GetVersion();
exec [bingo]._UnloadLibrary;

EXECUTE AS USER = 'test_op'

select * from [bingo].newtable;

select [bingo].Test2(5);

GO
SET SHOWPLAN_ALL OFF;
SET SHOWPLAN_ALL ON;

select top 100 MAX(LEN(Bingo.GetVersion())) from testtbl OPTION (MAXDOP 3);

select top 20 * from testtbl where SUBSTRING(bingo.GetVersion(), 1, 4)='1.3-';

drop table sugars;
create table sugars (id int unique, molfile nvarchar(max), MDLNUMBER varchar(50), MOLNAME varchar(500));
select * from sugars;
delete from sugars

exec [bingo].ImportSDF 'sugars', 'molfile', 
	'c:\Tmp\sugars.sdf', 
	'molregno id int; MDLNUMBER MDLNUMBER; MOLNAME MOLNAME';

-- Index creation	
use mols;

drop synonym bingo;
create synonym bingo for [sandbox_db2].[bingo]

exec bingo.CreateMoleculeIndex 'mol10k', 'id', 'molfile';

select len(value) from bingo.Config_Bin

select count(*) from acd2d;

select top 100 id, molfile over from acd2d;

SELECT CURRENT_USER;

ALTER DATABASE sandbox_db2 SET DB_CHAINING ON

select OBJECT_ID('mol10')
use mols
select * from mol10;

select 

exec bingo.DropMoleculeIndex 'mols..mol10';

select single_pages_kb + multi_pages_kb + 
	virtual_memory_committed_kb from sys.dm_os_memory_clerks 
	where type = 'MEMORYCLERK_SQLCLR'

select bingo.CanonicalSmiles(bingo.ReadFileAsText('c:\Tmp\mols\sql_test_mol.mol'));

select * from acd2d where id in 
	(select * from bingo.SearchSub(bingo.ReadFileAsText('c:\\Tmp\\bingo\\query.mol'), 'mol10', 'TOP 500'));
	
select * from mol10 where id in (select * from [sandbox_db2].bingo.SearchSub('CCCCC', 'mols..mol10', 'TOP 500'));

select bingo.CanonicalSmiles('C1CCC2C(C1)C1CCCCC1C1CCC3CC1C1CC(CCC21)C1CCC2C(C1)C1CCCCC1C1CCCCC1C1CCCC3C21');

--SELECT bingo.CanonicalSmiles(BulkColumn) from OPENROWSET(BULK N'c:\Tmp\qqq.mol', SINGLE_CLOB) as anystring

use mols;
use sandbox_db2;

select * from Mol10;

create synonym bingo for [sandbox_db2].[bingo]

create table acd2d (id int unique, molfile nvarchar(max));

drop table mol1000;
select top 100000 * into mol100k from [mols].[dbo].[acd2d];
select top 100 * into mol100 from [mols].[dbo].[acd2d];

select * into sugars from [mols].[dbo].[sugars];

select * from sugars;

create index id on sugars(id);

create index id on acd2d(id);

select * from mol100;

create table acd2d (id int unique, molfile nvarchar(max));
exec [sandbox_db2].[bingo].ImportSDF 'acd2d', 'molfile', 
	'c:\Tmp\acd2d_symyx.sdf', 
	'cdbregno id int';

use mols;

exec bingo.CreateMoleculeIndex 'Acd2d', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol100k', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol10', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol100', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol1k', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol10k', 'id', 'molfile';

select * from bingo.CONTEXT

exec bingo.DropIndex 'mol10k'

exec bingo.CreateReactionIndex 'rhea', 'id', 'molfile';

use mols;
select * from 
	bingo.SearchSub('acd2d', 
	'C(Br)1=C(Br)C(Br)=C(Br)C(Br)=C1Br', '');

select * from 
	bingo.SearchSim('acd2d', 
	'C(Br)1=C(Br)C(Br)=C(Br)C(Br)=C1Br', 'Tanimoto', 0.70, 1.1);

select * into test1 from acd2d where id=55;
exec bingo.CreateMoleculeIndex 'test1', 'id', 'molfile';

select * from bingo.shadow_21575115 where id=55;

select bingo.smiles(molfile) from acd2d where id=55;

exec bingo.DropIndex 'mol10'	
select * from bingo.context;	
	
select * from 
	bingo.SearchRSub('rhea', 
	'CCCCC>>', '');

	
exec bingo._CheckMemoryAllocate 140, 0

SELECT @@VERSION

use mols
select id, bingo.MolecularWeight(molfile) from mol10k where id in 
	(select * from bingo.SearchMolecularWeight('mol10k', 310, 320, ''))

select id, bingo.Gross(molfile), bingo.MolecularWeight(molfile), molfile from mol10k 
	where 
		id in (select * from bingo.SearchGross('mol10k', '>= O10 C H S', ''))
	
select * from bingo.SearchGross('mol10k', '<= C4 H4 O', '')

select * from bingo.SearchRSub('rhea', 'CCCCCC>>CCCCCC', '')

select top 100 bingo.Gross(molfile) from mol100k;

select bingo.MolecularWeight('C1=CC2=C3C=NC4=C(N=CP5=C4C=NC4=P5C5=C(N=C4)C4=C(P=C5)C5=C(C=N4)C4=C(C=C5)P=CC=C4)C3=C3C4=C(N=CC=N4)N=NC3=C2C=C1')

select * from bingo.SearchSub('mol10k', 'CC1=CC=CC=C1', '')
select * from bingo.SearchSim('mol10k', bingo.ReadFileAsText('c:\\Tmp\\bingo\\query.mol'), 'Tanimoto', 0.9, null)

select bingo.GetVersion();

use mols;
exec bingo._DropAllIndices