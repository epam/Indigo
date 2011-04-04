CREATE TRIGGER {0}
	ON {1}
	FOR DELETE
AS
	IF OBJECT_ID('tempdb..##bingotmptbl_{5}_{6}') IS NOT NULL
	BEGIN
		DROP TABLE ##bingotmptbl_{5}_{6}
	END
	select {2} into ##bingotmptbl_{5}_{6} from deleted 

	exec {4}._OnDeleteRecordTrigger {5}, {6}, '##bingotmptbl_{5}_{6}'
	DROP TABLE ##bingotmptbl_{5}_{6}
