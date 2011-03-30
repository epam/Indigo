CREATE TRIGGER {0}
	ON {1}
	FOR INSERT
AS
	-- Create global cursor for inserted records
	-- Simultaneous inserts from different sessions are not supported
	declare [bingotmpcur_{5}_{6}] cursor GLOBAL FORWARD_ONLY READ_ONLY for select {2}, {3} from inserted
	open [bingotmpcur_{5}_{6}]

	-- Cursor will be deallocated in the _OnInsertRecordTrigger automatically
	exec {4}._OnInsertRecordTrigger {5}, {6}, '[bingotmpcur_{5}_{6}]'
