<Query Kind="Statements">
  <Connection>
    <ID>49ac537d-474c-4d83-b438-afdef5140c0f</ID>
    <Persist>true</Persist>
    <Server>CHR-0270</Server>
    <Database>mols</Database>
    <ShowServer>true</ShowServer>
  </Connection>
  <Reference>D:\usr\projects\indigo_sql_server\bingo\sqlserver\linqpad_extesions\linqpad_extesions\linqpad_extesions.dll</Reference>
</Query>

var table = Acd2ds;
string table_mame = "acd2d";
string options = "";
string query_file = "c:\\Tmp\\bingo\\query.mol";
string target_file = "c:\\Tmp\\bingo\\results.sdf";

string query = System.IO.File.ReadAllText(query_file);

DateTime EventTime_begin = DateTime.Now;
var qqq = SearchSub(table_mame, query, options);
qqq.Dump();
(DateTime.Now - EventTime_begin).Dump();
	
var selected_mols = 
	from m in table 
	from id in SearchSub(table_mame, query, options)
	where m.Id==id.Id select m;
	
// selected_mols.Dump();

indigo.Utils.DumpSDF(selected_mols, target_file, "Molfile");

//indigo.Utils.DumpSDF(table, "c:\\Tmp\\bingo\\table.sdf", "Molfile");
/** /
indigo.Utils.DumpSDF(Acd2d_bugs, "c:\\Tmp\\bingo\\bug.sdf", "Molfile");
/* */