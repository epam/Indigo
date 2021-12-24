using System.Linq;
using System.Text.RegularExpressions;
using CppSharp;
using CppSharp.Generators;
using CppSharp.Passes;
using Indigo.Net.BindingGenerator.Utils;

namespace Indigo.Net.BindingGenerator.GeneratorOutputPasses
{
    public class LowerCamelCaseFunctionNamesPass : GeneratorOutputPass
    {
        public override void VisitGeneratorOutput(GeneratorOutput output)
        {
            var regex = new Regex(@"(\w+)\(");

            var functions = output.Outputs.SelectMany(x => x.FindBlocks(BlockKind.Function));
            foreach (var function in functions)
            {
                var str = function.Text.StringBuilder.ToString();
                var matches = regex.Matches(str);
                if (matches.Count > 0)
                {
                    var match = matches[0].Groups[1].Value;
                    var camelCaseName = StringConverter.ConvertToCamelCase(match);
                    var replacedStr = regex.Replace(str, $"{camelCaseName}(", 1);

                    function.Text.StringBuilder.Clear();
                    function.Text.StringBuilder.Append(replacedStr);
                }
            }
        }
    }
}
