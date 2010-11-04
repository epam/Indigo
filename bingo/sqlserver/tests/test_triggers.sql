use mols;
go

select * from mol10k;

delete from mol1k where id < 0;

insert into mol1k values (-11, 'CCCCCC')
-- , (-6, 'CCCCCC'), (-7, 'CCCCCC'), (-8, 'CCCCCC')

update mol1k set molfile='NNNNNN' where id < 0

update mol10 set id=id-10 where id < 0

delete from mol10 where id = -11;

select * from [bingo].shadow_101575400

select * from bingo.SearchSub('mol1k', 'NNNNNN', '')

exec bingo.CreateMoleculeIndex 'mol1k', 'id', 'molfile';

exec bingo.DropIndex 'mol1k'

exec bingo.FlushOperations 'mol1k'

CREATE TRIGGER [DBO].[MOL10_Insert]
	ON [MOLS].[DBO].[MOL10]
	FOR INSERT
AS
	declare @ID int;
	declare @data nvarchar(max);
	declare Cur cursor LOCAL for select id, molfile from inserted;

	open Cur

	fetch next from Cur into @ID, @data

	while @@fetch_status = 0
	begin
		exec bingo._OnInsertRecord '[MOLS].[DBO].[MOL10]', @ID, @data
		
		fetch next from Cur into @ID, @data
	end
  close Cur
  deallocate Cur
go