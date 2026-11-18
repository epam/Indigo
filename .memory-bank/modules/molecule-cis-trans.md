# MoleculeCisTrans

> **Read when:** working with cis-trans (E/Z) stereochemistry, double bond configuration, substructure matching with stereo constraints, or molecular transformations that affect double bond geometry.
> **Skip when:** working only with atom stereochemistry (see `MoleculeStereocenters`), single bonds, or structures without double bonds.

Manages **cis-trans (geometric) stereochemistry** on double bonds. This is *side data* keyed by bond index — the bond itself stores only connectivity and order; configuration lives here (see invariant A5).

## Responsibility

- Determine which double bonds can exhibit cis-trans stereochemistry (`isGeomStereoBond`)
- Compute configuration from 3D coordinates (`build`)
- Parse configuration from SMILES directional notation (`/`, `\`) in `buildFromSmiles`
- Map configuration through atom mappings for substructure matching (`applyMapping`, `getMappingParitySign`)
- Update configuration when molecular topology changes (`flipBond`, `registerUnfoldedHydrogen`)
- Validate and normalize substituent ordering (`sortSubstituents`)

Does **not** handle:
- Atom stereochemistry (tetrahedral centers) — see `MoleculeStereocenters`
- Allene or cumulated double bond stereochemistry
- Ring strain analysis beyond detecting trans configuration in rings

## Public Interface

Core in `core/indigo-core/molecule/molecule_cis_trans.h`:

**Lifecycle:**
- `build(BaseMolecule&, int* exclude_bonds)` — compute from 3D coordinates
- `buildFromSmiles(BaseMolecule&, int* dirs)` — parse from directional bonds
- `buildOnSubmolecule(BaseMolecule&, BaseMolecule& super, int* mapping)` — transfer from supermolecule

**Query:**
- `getParity(int bond_idx)` → `0` (none), `CIS` (1), or `TRANS` (2)
- `getSubstituents(int bond_idx)` → `int[4]` where `[0,1]` attach to beg, `[2,3]` to end; `-1` = implicit
- `isGeomStereoBond(BaseMolecule&, int bond_idx, int* substituents, bool have_xyz)` — eligibility check
- `isIgnored(int bond_idx)` — explicitly excluded from stereo processing

**Mutation:**
- `setParity(int bond_idx, int parity)`
- `add(int bond_idx, int substituents[4], int parity)`
- `ignore(int bond_idx)` — mark bond as non-stereo
- `flipBond(BaseMolecule&, int atom_parent, int atom_from, int atom_to)` — update when topology changes

**Mapping (for substructure matching):**
- `applyMapping(int parity, const int* substituents, const int* mapping, bool sort)` — compute parity after atom renumbering
- `getMappingParitySign(BaseMolecule& query, BaseMolecule& target, int bond_idx, const int* mapping)` → `-2` (invalid), `0` (no constraint), `±1` (match/mismatch)
- `checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping)` — validate all stereo-care bonds match

## Storage Model

One `_Bond` entry per bond index in `_bonds` array:

```cpp
struct _Bond {
    int parity;           // 0 (none), 1 (CIS), or 2 (TRANS)
    int ignored;          // 1 = explicitly excluded
    int substituents[4];  // [beg_sub0, beg_sub1, end_sub2, end_sub3]
                         // -1 = implicit H or lone pair
}
```

**Substituent ordering convention** (`sortSubstituents`):
- Within each pair `[0,1]` and `[2,3]`: smaller index first (except explicit H or `-1` moves right)
- Swapping within a pair flips parity
- Invalid if both substituents on one end are implicit, **except** nitrogen with lone pair allows it

## Eligibility Rules

A bond exhibits cis-trans stereochemistry when (`core/indigo-core/molecule/src/molecule_cis_trans.cpp#isGeomStereoBond`):

1. **Double bond** between C, N, Si, or Ge
2. Each atom has **degree 2 or 3** (the double bond + 1-2 single bonds)
3. Not an **R-site** (variable attachment point)
4. Substituents don't form a **triangle** (same atom on both sides)
5. With 3D coordinates: substituents not **collinear** with double bond axis

Ring double bonds ≤7 atoms can be cis-trans, but trans is strained (`isRingTransBond` detects this).

## Geometric Calculation

`build` computes configuration from 3D coordinates using vector geometry (`core/indigo-core/molecule/src/molecule_cis_trans.cpp#sameside`):

1. For double bond beg→end with substituents sub0 (at beg) and sub2 (at end):
2. Create normal vector via double cross product: `norm = (diff × norm_beg) × diff` where `diff = beg - end`, `norm_beg = sub0 - beg`
3. Project both substituent vectors onto this normal: `prod_beg = dot(norm_beg, norm)`, `prod_end = dot(norm_end, norm)`
4. If signs match → **CIS**, opposite → **TRANS**

Returns `0` if vectors are collinear (cannot determine configuration).

## SMILES Directional Bonds

`buildFromSmiles` interprets `/` and `\` notation (`core/indigo-core/molecule/src/molecule_cis_trans.cpp#buildFromSmiles`):

- Direction `1` (`/`) or `2` (`\`) stored per bond in `dirs` array
- Direction is **relative to bond orientation** (beg→end), so flips for neighbor bonds
- Algorithm infers missing directions from explicit ones on same atom
- Validates no co-directed substituents on one atom (invalid SMILES)
- Final check: if `subst_dirs[0] == subst_dirs[2]` → CIS, else TRANS

## Mapping and Substructure Matching

**Core insight**: atom renumbering can change substituent ordering, which inverts parity.

`applyMapping` computes new parity (`core/indigo-core/molecule/src/molecule_cis_trans.cpp#applyMapping`):
- Check if ordering preserved for each pair `[0,1]` and `[2,3]`
- Both pairs swap → parity unchanged
- One pair swaps → parity inverted (CIS↔TRANS)
- Either pair has both unmapped → parity becomes `0`

`getMappingParitySign` validates matching (`core/indigo-core/molecule/src/molecule_cis_trans.cpp#getMappingParitySign`):
- Returns `-2` if query has stereo but target doesn't (invalid match)
- Maps query substituents to target and compares configurations
- Accounts for which substituent (0 or 1, 2 or 3) was chosen on each end
- Returns `1` (match), `-1` (mismatch), or `0` (no constraint)

## Constraints & Traps

1. **Substituent arrays are normalized** — always run `sortSubstituents` before storing or comparing. Direct comparison of raw substituent arrays fails.

2. **Explicit hydrogens matter for queries** — `[H]/C=C/C` (explicit) and `/C=C/C` (implicit) are **different** for query molecules but equivalent for target molecules. See `core/indigo-core/molecule/src/molecule_cis_trans.cpp#sortSubstituents` lines 150-163.

3. **Nitrogen lone pairs** — `[H]/N=C/C` allows implicit hydrogen on nitrogen (lone pair) even when not marked. Test via `_commonHasLonePair`.

4. **Mapping with -1 substituents** — when `substituents[i] == -1` or `mapping[substituents[i]] == -1`, that position contributes no constraint. See `core/indigo-core/molecule/src/molecule_cis_trans.cpp#_getPairParity`.

5. **Ring trans bonds are strained** — `isRingTransBond` detects trans configuration in rings by tracking which substituents are ring members. Trans in 7-member or smaller rings is geometrically unusual.

6. **Collinear geometry returns 0** — if substituents lie on the double bond axis (within threshold), configuration cannot be determined. `sameside` returns `0`, not an error.

7. **Parity inversion formula** — to flip CIS↔TRANS: `new_parity = 3 - old_parity`. This works because CIS=1, TRANS=2.

8. **Topology changes require updates** — when using `flipBond`, `registerUnfoldedHydrogen`, or `restoreSubstituents`, configuration must be explicitly maintained. These methods update substituent indices but caller must validate parity remains correct.

## Dependencies

**Depends on:**
- `BaseMolecule` — atom/bond properties, graph traversal
- `math/algebra.h` — `Vec3f` for 3D geometry
- `molecule/elements.h` — `ELEM_H`, `ELEM_C`, etc.

**Depended on by:**
- `Molecule`, `QueryMolecule` — expose `cis_trans` member
- Substructure matchers — call `checkSub`, `getMappingParitySign`
- Loaders/savers — build configuration on input, serialize on output
- Automorphism search — validate symmetry preserves configuration via `isAutomorphism`
- Reaction enumerator — preserve stereo through transformations (see `flipBond` usage in `core/indigo-core/reaction/src/reaction_enumerator_state.cpp`)

## Related

- Stereocenters (tetrahedral): `MoleculeStereocenters` (not documented yet)
- Allene stereochemistry: handled separately (not documented yet)
- Invariant A5: why stereo is side data, not part of atom/bond
