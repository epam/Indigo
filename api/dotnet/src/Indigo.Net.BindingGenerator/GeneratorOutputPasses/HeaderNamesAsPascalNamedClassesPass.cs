using System.Text.RegularExpressions;
using CppSharp;
using CppSharp.Passes;
using Indigo.Net.BindingGenerator.Utils;

namespace Indigo.Net.BindingGenerator.GeneratorOutputPasses
{
    public class HeaderNamesAsPascalNamedClassesPass : GeneratorOutputPass
    {
        public override void VisitNamespace(Block block)
        {
            var regex = new Regex(@"class ([\w|-]+)");

            var unknownBlocks = block.FindBlocks(BlockKind.Unknown);
            foreach (var unknownBlock in unknownBlocks)
            {
                if (unknownBlock.Parent.Kind != BlockKind.Functions) continue;

                var str = unknownBlock.Text.StringBuilder.ToString();
                var matches = regex.Matches(str);
                if (matches.Count > 0)
                {
                    var className = matches[0].Groups[1].Value;
                    var newClassName = NameGenerator.GenerateClassName(className);
                    var replacedStr = regex.Replace(str, $"class {newClassName}");

                    unknownBlock.Text.StringBuilder.Clear();
                    unknownBlock.Text.StringBuilder.Append(replacedStr);
                }
            }
        }
    }
}
