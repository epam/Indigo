CREATE TRIGGER {0}
	ON {1}
	FOR UPDATE
AS
	IF UPDATE({2}) 
	BEGIN 
		RAISERROR ('{2} column cannot be changed after Bingo index has been created.', 16, 1);
		ROLLBACK TRANSACTION
	END
	
	IF NOT UPDATE({3}) 
	BEGIN 
		RETURN 
	END
	
	-- At first delete records and then insert them

	-- Create global cursor for deleted records
	-- Simultaneous inserts from different sessions are not supported
	declare [bingotmpcur_upd_{5}_{6}] cursor GLOBAL FORWARD_ONLY READ_ONLY for select {2} from inserted
	open [bingotmpcur_upd_{5}_{6}]

	-- Cursor will be deallocated in the _OnInsertRecordTrigger automatically
	exec {4}._OnDeleteRecordTrigger {5}, {6}, '[bingotmpcur_upd_{5}_{6}]'

	declare [bingotmpcur_upd_{5}_{6}] cursor GLOBAL FORWARD_ONLY READ_ONLY for select {2}, {3} from inserted
	open [bingotmpcur_upd_{5}_{6}]

	-- Cursor will be deallocated in the _OnInsertRecordTrigger automatically
	exec {4}._OnInsertRecordTrigger {5}, {6}, '[bingotmpcur_upd_{5}_{6}]'


