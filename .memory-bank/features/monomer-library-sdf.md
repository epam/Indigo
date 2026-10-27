# Monomer library from SDF

> **Read when:** a ticket is about a monomer or preset that an SDF library update accepted, rejected
> or changed; or you are adding a property to the SDF library format.
> **Skip when:** the library is given as KET — that input does not go through these properties.

## Problem

Ketcher and API callers extend the monomer library with an SDF file: each record is one library item,
a MOL V3000 with its template(s) followed by a data section of properties (#3161). The properties say
what the item is and how it is named in HELM and IDT, so a wrong one is reported rather than guessed
at.

## Interface

`indigoLoadMonomerLibrary` and its file and string forms (`loadMonomerLibraryFromFile` /
`loadMonomerLibraryFromString` in the wrappers). The input format is detected: KET is read as a
library document, anything else as SDF. Ketcher's `updateMonomersLibrary(…, { format: 'sdf' })` sends
the whole SDF in one call.

| Property | Applies to | Meaning |
| --- | --- | --- |
| `type` | every item | `monomerTemplate` (the default when absent) or `monomerGroupTemplate` (a preset) |
| `groupName`, `groupClass` | presets | mandatory; ignored on a `monomerTemplate` |
| `idtAliases` | every item | `base=…` and/or `ep5=…`, `ep3=…`, `internal=…`, separated by `;` or `,` |
| `modificationTypes` | monomers | `;`-separated list |
| `aliasHELM`, `aliasBILN` | monomers | the alias in that notation |

## Expected behaviour

- WHEN `type` is anything but the two values above, empty included, THEN the load fails with
  `Type: "<type>" is unknown. "<name>" monomer hasn't been added to the library.`
- WHEN a preset has no `groupName`, or an empty one, THEN the load fails with
  `Load of preset has failed, preset definition contains invalid groupName value. The groupName couldn't be empty`
  (#3666).
- WHEN a preset has no `groupClass`, or an empty one, THEN the load fails, naming the preset.
- WHEN `modificationTypes` is present but empty THEN the load fails with
  `Load of "<name>" monomer has failed, monomer definition contains invalid modificationTypes value. The modificationTypes couldn't be empty`
  (#3671).
- WHEN an `idtAliases` entry has no value — `base=`, or `ep3` with no `=` — THEN the load fails with
  `… contains invalid idtAliases value. The value of "<key>" couldn't be empty`, naming the item (#3672).
- WHEN any record fails THEN the whole call fails, prefixed `Error loading monomer library:`, and no
  library is returned: the valid records of the same file are not loaded either. Ketcher shows the
  message as `Monomer item could not be loaded because of an error: <message>`.

## Guarantees

- A library Indigo saves to SDF passes these checks when it is loaded back: the saver writes only the
  two known types and both group fields of a preset, and writes `idtAliases` and `modificationTypes`
  only when they have a value. Ketcher's default library makes the round trip whole.
- `<name>` is the template's name for a monomer and the `groupName` for a preset.

## Limitations

- #3161 asks for an invalid item to be skipped and the rest loaded. One call returns one library or
  one error, so the whole file fails instead.
- `aliasHELM` is kept on items that the requirement says may not have one — RNA nucleotides, CHEMs
  and presets — and no warning is given (#3677).
- An `idtAliases` key other than the four above is ignored without an error, and so is the value of
  `groupClass`: only its presence is checked, not that it is `RNA`.
- The format and length rules of #3161 for the aliases are not checked.
