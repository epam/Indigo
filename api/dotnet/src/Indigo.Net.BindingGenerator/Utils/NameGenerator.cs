namespace Indigo.Net.BindingGenerator.Utils
{
    public static class NameGenerator
    {
        public static string GenerateClassName(string input)
        {
            var newInput = input switch
            {
                "bingo-nosql" => "bingo",
                "bingo_nosql" => "bingo",
                _ => input
            };
                
            return $"{StringConverter.ConvertToPascalCase(newInput)}Lib";
        }
    }
}
