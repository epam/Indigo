<Query Kind="SQL">
  <Connection>
    <ID>49ac537d-474c-4d83-b438-afdef5140c0f</ID>
    <Persist>true</Persist>
    <Server>CHR-0270</Server>
    <Database>mols</Database>
    <ShowServer>true</ShowServer>
  </Connection>
  <Reference>d:\usr\projects\indigo_sql_server\bingo\sqlserver\linqpad_extesions\linqpad_extesions\bin\Debug\linqpad_extesions.dll</Reference>
</Query>

--exec bingo.CreateMoleculeIndex 'mol10', 'id', 'molfile';

select * from bingo.SearchSub(bingo.ReadFileAsText('c:\\Tmp\\bingo\\query.mol'), 'mol10', '')

select * from mol10 where id in 
	(select * from bingo.SearchSub(bingo.ReadFileAsText('c:\\Tmp\\bingo\\query.mol'), 'mol10', 'TOP 500'));
	
exec bingo.CreateMoleculeIndex 'mol10', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol100', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol1k', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol10k', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'mol100k', 'id', 'molfile';
exec bingo.CreateMoleculeIndex 'Acd2d', 'id', 'molfile';

exec bingo.CreateMoleculeIndex 'Acd2d_bug', 'id', 'molfile';

exec bingo.DropMoleculeIndex 'mol10';
exec bingo.DropMoleculeIndex 'mol100';
exec bingo.DropMoleculeIndex 'mol1k';
exec bingo.DropMoleculeIndex 'mol10k';
exec bingo.DropMoleculeIndex 'mol100k';
exec bingo.DropMoleculeIndex 'Acd2d';

select count(*) from bingo.SearchSub('CCCCP(CC)CCC', 'acd2d', 'TOP 1000')

select count(*) from bingo.SearchExact('mol1k', bingo.ReadFileAsText('c:\\Tmp\\bingo\\query.mol'), '')

--insert into acd2d_bug select * from acd2d where id=73922;