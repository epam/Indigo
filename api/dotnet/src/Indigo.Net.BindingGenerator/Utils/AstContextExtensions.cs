using System;
using CppSharp.AST;

namespace Indigo.Net.BindingGenerator.Utils
{
    public static class AstContextExtensions
    {
        public static void SetFunctionParameterType(
            this ASTContext context, string functionName, int parameterIndex, QualifiedType qualifiedType)
        {
            if (parameterIndex <= 0)
                throw new ArgumentException(null, nameof(parameterIndex));

            foreach (var function in context.FindFunction(functionName))
            {
                if (function.Parameters.Count < parameterIndex)
                    throw new ArgumentException(null, nameof(parameterIndex));

                var parameter = function.Parameters[parameterIndex - 1];
                parameter.QualifiedType = qualifiedType;
            }
        }
        
        public static void SetFunctionParameterArrayType(
            this ASTContext context, string functionName, int parameterIndex, PrimitiveType? primitiveType = null)
        {
            if (parameterIndex <= 0)
                throw new ArgumentException(null, nameof(parameterIndex));

            foreach (var function in context.FindFunction(functionName))
            {
                if (function.Parameters.Count < parameterIndex)
                    throw new ArgumentException(null, nameof(parameterIndex));

                var parameter = function.Parameters[parameterIndex - 1];
                if (primitiveType != null)
                {
                    parameter.QualifiedType = new QualifiedType(new ArrayType
                    {
                        QualifiedType = new QualifiedType(new BuiltinType(primitiveType.Value)),
                        SizeType = ArrayType.ArraySize.Incomplete
                    });
                }
                else
                {
                    if (parameter.QualifiedType.Type is PointerType { Pointee: BuiltinType pointerBuiltinType })
                    {
                        parameter.QualifiedType = new QualifiedType(new ArrayType
                        {
                            QualifiedType = new QualifiedType(pointerBuiltinType),
                            SizeType = ArrayType.ArraySize.Incomplete
                        });
                        continue;
                    }

                    throw new ArgumentException("Parameter's type is not a pointer", nameof(parameterIndex));
                }
            }
        }
    }
}

