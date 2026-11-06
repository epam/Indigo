> **Read when:** you need what a molfile carries for a data S-group — the field, its display record, its value —
> what Indigo refuses on reading or on writing, and when `auto` writes V3000 although V2000 was not excluded.
> **Skip when:** the question is the KET side ([ket-sgroups.md](ket-sgroups.md)), the brackets
> ([sgroup-brackets.md](sgroup-brackets.md)), or how the loader is built.

# Data S-groups in molfiles

## Problem

A data S-group is a field attached to atoms: a name, a type (formatted, numeric or text), units or a format, a query
flag and operator, a display record (position, attached or detached, absolute or relative, units shown, characters
shown, a tag and its position) and the value. V2000 keeps it in fixed columns (`M  SDT`, `M  SDD`, `M  SED`), V3000
in the keys of one line. Indigo reads both and writes either. A value the format does not allow must not become a
file no reader, Indigo included, can load; a value the columns of V2000 cannot hold must not shift the columns.

## Interface

Loading a molfile or an SDF entry; `molfile()` / `saveMolfile` with the option `molfile-saving-mode` (`auto`, `2000`,
`3000`), through any wrapper, the WASM module or the REST service.

## Expected behaviour

- WHEN a field type is not `F`, `N` or `T`, a query type is not `IQ`, an operator is not one of `=`, `<>`, `<`, `<=`,
  `>`, `>=`, `BETWEEN`, `CONTAINS`, `LIKE` (any case), the number of displayed characters is outside 0–999, a DASP
  position is outside 1–9 or a display coordinate is not a number THEN the file fails to load with the line or the
  key, the group and the value: `M SDT of Sgroup 1: 'X' is not a valid field type`, `QUERYOP of Sgroup 2:
  'QBETWEEN' is not a valid query operator`. A blank type is text, a blank operator is none, DASP `0` or blank is "not
  set", a type with a blank around it is the letter.
- WHEN a NUMERIC field holds data rather than a query THEN every non-empty line of its value must be a number;
  otherwise `M SED of Sgroup 1: 'abc' is not a valid numeric value`, `FIELDDATA of Sgroup 1: …` in V3000. An empty
  line is no value. FORMATTED values are not checked.
- WHEN a display coordinate of `M  SDD` or `FIELDDISP` has more decimals than single precision keeps to four
  (1234.5678) THEN the file fails to load: `cannot be kept to four decimals`.
- WHEN a structure holds what a molfile does not allow — a type, query type or operator outside the lists, displayed
  characters outside 0–999, a display coordinate outside −9999.9999 … 99999.9999, a NUMERIC value that is not a
  number — THEN writing it, as V2000 or as V3000, fails with `data S-group 1: 'value' reason`, the reason being the
  reader's.
- WHEN the columns of V2000 cannot hold the structure — more than 999 S-groups, an atom or bracket coordinate outside
  ten columns, a field name over 30 characters, units over 20, a data line over 200 — THEN `auto` writes V3000, and
  a requested V2000 fails naming the first thing that did not fit: `V2000 cannot hold the field name '…' of data
  S-group 1, longer than 30 characters`.
- WHEN a V2000 data line is longer than 200 characters THEN the reader keeps 200 and drops the rest.
- WHEN V3000 `FIELDINFO` starts with `F`, `N` or `T` followed by a blank, or is that letter alone, THEN the letter is
  the field type and the rest the units; any other value is the units alone. Indigo writes the letter, a blank and
  the units, or the units alone when no type is set.
- WHEN a V3000 value holds a blank, a quote, a double quote, `=`, `(` or `)` THEN it is written in double quotes with
  a double quote inside doubled, and read back the same way; `FIELDDATA` is always quoted, one key per line of the
  value.
- WHEN a V3000 coordinate has more integer digits than six significant digits leave four decimals for THEN it is
  written with four decimals; no coordinate is written with an exponent.

## Guarantees

- A data S-group survives V2000 ⇄ V3000: name, type, units, query flag and operator, display record, value, tag.
- What Indigo writes, Indigo reads: the writer refuses exactly what the reader rejects, V2000 and V3000 alike.
- The display position of a molfile is counted from the first atom of the group; KET counts from the centre and the
  conversion is done on the way ([ket-sgroups.md](ket-sgroups.md)).
- A V2000 file Indigo writes keeps its columns: nothing longer than its field is written into it.

## Limitations

- `FIELDINFO` is a free-format string in the specification: units that are themselves `F`, `N` or `T`, or such a
  letter and a blank, read as a type.
- A V2000 data line over 200 characters, as Ketcher writes them, loses its tail on reading.
- A V2000 line with a value one column off — a type in column 43, a query type one column to the right — reads as
  the trimmed value: a shifted `IQ` is `I` and is rejected, a shifted type letter is still the letter; the type is
  written back in its own column.
- A V3000 atom coordinate keeps the precision of single precision: 1234.5678 comes out as 1234.5677.
