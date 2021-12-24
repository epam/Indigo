using System.IO;
using System.Linq;
using CppSharp;
using CppSharp.AST;
using CppSharp.Generators;
using Indigo.Net.BindingGenerator.GeneratorOutputPasses;
using Indigo.Net.BindingGenerator.Utils;

namespace Indigo.Net.BindingGenerator
{
    public class Library : ILibrary
    {
        public void Preprocess(Driver driver, ASTContext ctx)
        {
            ctx.SetFunctionParameterUsage("indigoGetOptionInt", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetOptionBool", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetOptionFloat", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetReactingCenter", 3, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetCharge", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetExplicitValence", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetRadicalElectrons", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoGetRadical", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoCountHydrogens", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoSymmetryClasses", 2, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoSerialize", 3, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("indigoToBuffer", 3, ParameterUsage.Out);
            ctx.SetFunctionParameterUsage("bingoEstimateRemainingTime", 2, ParameterUsage.Out);

            // -----------

            ctx.SetFunctionParameterArrayType("indigoReadBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadFingerprintFromDescriptors", 1);
            ctx.SetFunctionParameterArrayType("indigoLoadStructureFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadQueryReactionFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoRenderGridToFile", 2);
            ctx.SetFunctionParameterArrayType("indigoRenderGrid", 2);
            ctx.SetFunctionParameterArrayType("indigoAddDataSGroup", 3);
            ctx.SetFunctionParameterArrayType("indigoAddDataSGroup", 5);
            ctx.SetFunctionParameterArrayType("indigoAddSuperatom", 3);
            ctx.SetFunctionParameterArrayType("indigoCreateSubmolecule", 3);
            ctx.SetFunctionParameterArrayType("indigoGetSubmolecule", 3);
            ctx.SetFunctionParameterArrayType("indigoCreateEdgeSubmolecule", 3);
            ctx.SetFunctionParameterArrayType("indigoCreateEdgeSubmolecule", 5);
            ctx.SetFunctionParameterArrayType("indigoRemoveAtoms", 3);
            ctx.SetFunctionParameterArrayType("indigoRemoveBonds", 3);
            ctx.SetFunctionParameterArrayType("indigoAlignAtoms", 3);
            ctx.SetFunctionParameterArrayType("indigoAlignAtoms", 4);
            ctx.SetFunctionParameterArrayType("indigoUnserialize", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadReactionFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadMoleculeFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadSmartsFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadReactionSmartsFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadQueryMoleculeFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadFingerprintFromBuffer", 1, PrimitiveType.UChar);
            ctx.SetFunctionParameterArrayType("indigoLoadBuffer", 1, PrimitiveType.UChar);
            
            // -----------

            // byte**
            ctx.SetFunctionParameterType("indigoToBuffer", 2, new QualifiedType(new PointerType
            {
                Modifier = PointerType.TypeModifier.Pointer,
                QualifiedPointee = new QualifiedType(new PointerType
                {
                    Modifier = PointerType.TypeModifier.Pointer,
                    QualifiedPointee = new QualifiedType(new BuiltinType(PrimitiveType.UChar))
                })
            }));
        }

        public void Postprocess(Driver driver, ASTContext ctx)
        {
        }

        public void Setup(Driver driver)
        {
            driver.Options.GeneratorKind = GeneratorKind.CSharp;
            driver.Options.Verbose = true;
            driver.Options.GenerationOutputMode = GenerationOutputMode.FilePerUnit;

            var module = driver.Options.AddModule("Indigo");

            var cApiDirPath = GetDotnetFolderRelativePath("../c");
            module.IncludeDirs.Add(Path.Combine(cApiDirPath, "indigo"));
            module.IncludeDirs.Add(Path.Combine(cApiDirPath, "indigo-inchi"));
            module.IncludeDirs.Add(Path.Combine(cApiDirPath, "indigo-renderer"));
            module.IncludeDirs.Add(Path.Combine(cApiDirPath, "bingo-nosql"));

            module.Headers.Add("indigo.h");
            module.Headers.Add("indigo-inchi.h");
            module.Headers.Add("indigo-renderer.h");
            module.Headers.Add("bingo-nosql.h");

            var libsDir = GetDotnetFolderRelativePath("../../dist/lib");
            // CLang supports all kind of libs, so it doesn't matter which one to use
            var libToUse = Directory.GetDirectories(libsDir).First();
            module.LibraryDirs.Add(libToUse);
            module.Libraries.Add("indigo");
            module.Libraries.Add("indigo-inchi");
            module.Libraries.Add("indigo-renderer");
            module.Libraries.Add("bingo-nosql");

            module.OutputNamespace = "com.epam.indigo";

            driver.Options.GenerateName = unit => NameGenerator.GenerateClassName(unit.FileNameWithoutExtension);
            
            driver.Options.OutputDir = GetDotnetFolderRelativePath("src/Indigo.Net/Bindings");
        }

        public void SetupPasses(Driver driver)
        {
            driver.Context.GeneratorOutputPasses.AddPass(new HeaderNamesAsPascalNamedClassesPass());
            driver.Context.GeneratorOutputPasses.AddPass(new ReplaceStringMarshallerPass());
            driver.Context.GeneratorOutputPasses.AddPass(new LowerCamelCaseFunctionNamesPass());
        }

        private static string GetDotnetFolderRelativePath(string relativePath)
        {
#if DEBUG
            return Path.Combine(@"..\..\..\..\..", relativePath);
#else
            // used in CMake
            return relativePath;
#endif
        }
    }
}

