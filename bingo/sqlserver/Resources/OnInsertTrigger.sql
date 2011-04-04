CREATE TRIGGER {0}
	ON {1}
	FOR INSERT
AS
	IF OBJECT_ID('tempdb..##bingotmptbl_{5}_{6}') IS NOT NULL
	BEGIN
		DROP TABLE ##bingotmptbl_{5}_{6}
	END
	select {2}, {3} into ##bingotmptbl_{5}_{6} from inserted 

	exec {4}._OnInsertRecordTrigger {5}, {6}, '##bingotmptbl_{5}_{6}'
	DROP TABLE ##bingotmptbl_{5}_{6}
