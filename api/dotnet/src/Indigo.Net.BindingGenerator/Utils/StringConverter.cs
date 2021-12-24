using System.Linq;

namespace Indigo.Net.BindingGenerator.Utils
{
    public static class StringConverter
    {
        public static string ConvertToPascalCase(string str)
        {
            var parts = str.Split('-', '_');
            var pascalParts = parts.Select(x => char.ToUpper(x[0]) + x[1..]);
            return string.Concat(pascalParts);
        }

        public static string ConvertToCamelCase(string str)
        {
            return char.ToLower(str[0]) + str[1..];
        }
    }
}

