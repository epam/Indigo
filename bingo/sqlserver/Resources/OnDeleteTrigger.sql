CREATE TRIGGER {0}
	ON {1}
	FOR DELETE
AS
	-- Create global cursor for deleted records
	-- Simultaneous inserts from different sessions are not supported
	declare [bingotmpcur_del_{5}_{6}] cursor GLOBAL FORWARD_ONLY READ_ONLY for select {2} from deleted
	open [bingotmpcur_del_{5}_{6}]

	-- Cursor will be deallocated in the _OnInsertRecordTrigger automatically
	exec {4}._OnDeleteRecordTrigger {5}, {6}, '[bingotmpcur_del_{5}_{6}]'
