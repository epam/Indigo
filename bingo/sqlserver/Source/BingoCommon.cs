namespace indigo
{
   public class FetchedData
   {
      public FetchedData (int id) 
      {
         this.id = id;
      }

      public int id { get; set; }
      public string str { get; set; }
      public double value;
   }
}