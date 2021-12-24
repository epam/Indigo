using System.Linq;
using CppSharp;
using CppSharp.Generators;
using CppSharp.Passes;

namespace Indigo.Net.BindingGenerator.GeneratorOutputPasses
{
    internal class ReplaceStringMarshallerPass : GeneratorOutputPass
    {
        public override void VisitGeneratorOutput(GeneratorOutput output)
        {
            FixAttributes(output);
            FixRetrieval(output);
        }

        private static void FixAttributes(GeneratorOutput output)
        {
            const string marshallerOld = "CppSharp.Runtime.UTF8Marshaller";
            const string marshallerNew = "com.epam.indigo.UTF8Marshaller";
            
            var internalClassMethods = output.Outputs.SelectMany(i => i.FindBlocks(BlockKind.InternalsClassMethod));
            var typedefs = output.Outputs.SelectMany(i => i.FindBlocks(BlockKind.Typedef));

            var targetBlocks = internalClassMethods.Concat(typedefs);
            foreach (var block in targetBlocks)
            {
                block.Text.StringBuilder.Replace(marshallerOld, marshallerNew);
            }
        }

        private static void FixRetrieval(GeneratorOutput output)
        {
            const string getStringOld = "CppSharp.Runtime.MarshalUtil.GetString(global::System.Text.Encoding.UTF8, ";
            const string getStringNew = "com.epam.indigo.UTF8Marshaller.GetString(";

            var methodBodies = output.Outputs.SelectMany(i => i.FindBlocks(BlockKind.Function));
            foreach (var methodBody in methodBodies)
            {
                methodBody.Text.StringBuilder.Replace(getStringOld, getStringNew);
            }
        }
    }
}
