Bingo uses only a subset of a methods from OCI library. Oracle oci32.lib and oci64.lib are wrappers for oci.dll dynamic library. To avoid usage of Oracle binary code, such libraries were generated manually from a list of used functions.

To generate library files run the following commands in the Visual Studio Command Prompt:

lib /def:oci.def /out:lib32/oci32.lib /machine:x86
lib /def:oci.def /out:lib64/oci64.lib /machine:x64
