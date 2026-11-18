# MoleculeAlleneStereo

> **Read when:** working with allene (cumulated diene) stereochemistry, axial chirality, substructure matching with allene stereo constraints, or molecular transformations affecting C=C=C systems.
> **Skip when:** working only with tetrahedral stereochemistry (see `MoleculeStereocenters`), cis-trans double bonds (see `MoleculeCisTrans`), or structures without cumulated double bonds.

Manages **allene (axial) stereochemistry** on cumulated double bond systems. An allene is a fragment R₁R₂C=C=CR₃R₄ where the central carbon has two consecutive double bonds. The sp-hybridized central carbon creates perpendicular planes, giving the molecule axial chirality like a propeller. This is *side data* keyed by the central atom index (see invariant A5).

## Responsibility

- Detect allene centers: `[C,Si]=[C,Si]=[C,Si]` fragments that can exhibit axial stereochemistry (`possibleCenter`)
- Compute configuration from 3D coordinates and bond directions (`buildFromBonds`)
- Map configuration through atom mappings for substructure matching (`checkSub`, `buildOnSubmolecule`)
- Update configuration when molecular topology changes (`removeAtoms`, `removeBonds`, `registerUnfoldedHydrogen`)
- Serialize/restore bond direction markers for stereochemistry preservation (`markBonds`)

Does **not** handle:
- Tetrahedral atom stereochemistry — see `MoleculeStereocenters`
- Cis-trans (E/Z) stereochemistry on isolated double bonds — see `MoleculeCisTrans`
- Allenes with more than two consecutive double bonds (higher cumulenes)

## Public Interface

Core in `core/indigo-core/molecule/molecule_allene_stereo.h`:

**Lifecycle:**
- `buildFromBonds(BaseMolecule&, bool ignore_errors, int* sensible_bonds_out)` — detect and compute from 3D coordinates and bond directions
- `buildOnSubmolecule(BaseMolecule&, BaseMolecule& super, int* mapping)` — transfer from supermolecule
- `clear()` — remove all allene stereo data

**Query:**
- `possibleCenter(BaseMolecule&, int idx, int& left, int& right, int subst[4], bool pure_h[4])` → static check if atom could be allene center
- `isCenter(int atom_idx)` → true if this atom is a recognized allene center
- `getByAtomIdx(int atom_idx, int& left, int& right, int subst[4], int& parity)` — retrieve configuration
- `get(int i, ...)` — retrieve configuration by iterator position
- `size()`, `begin()`, `end()`, `next(int i)` — iterator interface

**Mutation:**
- `add(int atom_idx, int left, int right, int subst[4], int parity)` — manually add allene center
- `invert(int atom_idx)` — flip stereochemistry (parity 1↔2)
- `reset(int atom_idx)` — remove this allene center
- `markBonds(BaseMolecule&)` — set bond directions (wedges) on substituent bonds to encode stereochemistry

**Topology updates:**
- `removeAtoms(BaseMolecule&, const Array<int>& indices)` — update when atoms removed
- `removeBonds(BaseMolecule&, const Array<int>& indices)` — update when bonds removed
- `registerUnfoldedHydrogen(int atom_idx, int added_hydrogen)` — explicit H added

**Matching:**
- `checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping)` — validate all query allene centers match target configuration

## Storage Model

One `_Atom` entry per allene center in `_centers` RedBlackMap:

```cpp
struct _Atom {
    int left;        // index of "left" terminal atom
    int right;       // index of "right" terminal atom
    int subst[4];    // substituents: [0,1] on left, [2,3] on right
                     // [1] and [3] may be -1 (implicit H)
                     // [0] and [2] are never -1
    int parity;      // 1 = CCW rotation (looking left→right)
                     // 2 = CW rotation
                     // 3 = either/unknown (rejected by algorithm)
}
```

**Substituent normalization** (lines 252-262 in `molecule_allene_stereo.cpp`):
- Pure hydrogen (explicit H or implicit) moves to positions [1] or [3]
- Non-hydrogen substituent in position [0] or [2]
- Swapping [0]↔[1] or [2]↔[3] inverts parity (parity = 3 - parity)

## Eligibility Rules

An atom is an allene center when (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#possibleCenter`):

1. **Degree 2** — exactly two neighbors
2. **Element** — carbon or silicon (not N, O, etc.)
3. **Both bonds are double bonds** (`BOND_DOUBLE`)
4. **Terminal atoms** are C or Si with degree 2-3
5. **At least one non-hydrogen substituent** on each terminal atom (lines 122-126)

Additional geometric requirements in `_isAlleneCenter`:

6. **At least one bond direction** (up/down wedge) on substituents — no directions = no stereochemistry (line 162-163)
7. **Collinearity** — the two double bonds form ~180° angle: `dot(vec_left, vec_right) ≈ -1` within 0.04 tolerance (~16 degrees) (line 186)
8. **Opposite sides** — two substituents on same terminal must be on opposite sides of that bond axis (lines 190-194)
9. **Not square-planar** — configuration must be consistent with helical geometry (lines 215-216)

Tolerances: 0.04 for collinearity (line 186), 1e-3 for sameside dot products (line 48).

## Geometric Calculation

### The `sameside` Algorithm

**Purpose:** Determine if two substituent vectors `dir1` and `dir2` lie on the same side of a separator plane defined by `sep`.

**Method** (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#sameside`, lines 34-52):

1. Use **double cross product** to get vector in plane of `sep` and `dir1`:
   ```cpp
   norm = (sep × dir1) × sep
   ```
   This produces a vector perpendicular to `sep` in the plane containing both vectors.

2. Project both directions onto the normal:
   ```cpp
   prod1 = dot(dir1, norm)
   prod2 = dot(dir2, norm)
   ```

3. Return:
   - `1` if same sign (same side)
   - `-1` if opposite signs (opposite sides)
   - `0` if either is collinear with `sep` (cannot determine)

### Parity Calculation

**Input:** 3D coordinates, bond directions (up/down wedges) on substituent bonds

**Steps** (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#_isAlleneCenter`, lines 131-264):

1. **Extract geometry** (lines 144-160):
   - Read bond directions for all four substituents
   - Compute normalized vectors from terminal atoms to substituents
   - Throw error if any bond has zero length

2. **Validate bond directions** (lines 162-169):
   - At least one direction must be set
   - No two substituents on same end with identical direction (invalid notation)

3. **Check collinearity** (lines 174-187):
   - Compute vectors from center to left/right terminals
   - Must be antiparallel: `dot(vec_left, vec_right) + 1 < 0.04`

4. **Validate substituent geometry** (lines 189-194):
   - Two substituents on left must be on opposite sides: `sameside(...) == -1`
   - Two substituents on right must be on opposite sides: `sameside(...) == -1`

5. **Infer missing directions** (lines 200-213):
   - If one substituent has direction, infer the other: `dirs[0] = 3 - dirs[1]`
   - Use `sameside(subst_vecs[0], subst_vecs[2], vec_right)` to relate left and right

6. **Compute parity** (lines 215-222):
   - Check consistency with helical vs square-planar geometry
   - If `sameside == 1` and `dirs[0] == BOND_UP`, or `sameside == -1` and `dirs[0] == BOND_DOWN` → parity = 1
   - Otherwise → parity = 2

7. **Z-axis correction** (lines 245-250):
   - Compute `prod = vec_left × subst_vecs[0]`
   - If `prod.z > 0`, invert parity (accounts for drawing convention)

8. **Normalize hydrogens** (lines 252-262):
   - Move pure H from [0]→[1] or [2]→[3], inverting parity each time

## Substructure Matching

### `checkSub` — Validate Query Match

**Purpose:** Verify that all allene centers in query molecule match target configuration after atom mapping.

**Algorithm** (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#checkSub`, lines 315-361):

1. For each query allene center:
   - Target must also have allene center at mapped position (line 327-329)
   - Map all four query substituents to target (lines 332-335)

2. **Handle axis flip** (lines 337-341):
   - If `mapping[qs[0]]` maps to target's right side (`ts[2]` or `ts[3]`), swap left↔right
   - This reflects viewing the allene from opposite ends

3. **Handle substituent swap within each end** (lines 343-355):
   - If `mapping[qs[0]] == ts[0]` → no parity change
   - If `mapping[qs[0]] == ts[1]` → parity inverted
   - Same check for `qs[2]` → `ts[2]` or `ts[3]`

4. **Compare final parities** (lines 357-358):
   - Return `false` if mismatch, `true` if all centers match

### `buildOnSubmolecule` — Transfer to Submolecule

**Purpose:** Copy allene stereochemistry from supermolecule to submolecule via atom mapping.

**Algorithm** (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#buildOnSubmolecule`, lines 363-452):

1. For each allene center in super:
   - Map central, left, right atoms (lines 371-383)
   - Map all four substituents; skip if any key atom unmapped

2. **Validate non-hydrogen substituents** (lines 393-397):
   - Must have at least one non-H on each end

3. **Normalize hydrogen positions** (lines 399-423):
   - Move pure H from [0]→[1] or [2]→[3], inverting parity
   - Move unmapped substituent from [0]→[1] or [2]→[3], inverting parity

4. **Copy bond directions** (lines 427-450):
   - Transfer bond directions from super to submolecule for all substituent bonds

## `markBonds` — Serialize Stereochemistry

**Purpose:** Set bond directions (up/down wedges) on substituent bonds to encode allene stereochemistry for output formats.

**Algorithm** (`core/indigo-core/molecule/src/molecule_allene_stereo.cpp#markBonds`, lines 502-655):

1. **Recompute geometry** (lines 510-546):
   - Compute substituent vectors, check collinearity, validate opposite-side placement
   - If validation fails, skip this center (lines 541, 544, 546, 551)

2. **Select bonds to mark** (lines 553-595):
   - Priority 1: terminal atoms (degree 1) — lines 572-576
   - Priority 2: non-ring bonds — lines 577-581
   - Priority 3: any unmarked bonds — lines 582-583
   - If both left substituents marked, skip right (and vice versa) — lines 597-600

3. **Compute directions** (lines 602-640):
   - Start with parity 2 → `[DOWN, UP]`, parity 1 → `[UP, DOWN]` (lines 604-613)
   - Flip if `sameside == -1` (lines 615-619)
   - Flip if Z-axis test fails (lines 621-629)
   - Compute right directions from left (lines 631-640)

4. **Apply directions** (lines 642-653):
   - Orient edge so terminal atom is `beg` (line 648-649)
   - Set bond direction (line 651)

## Constraints & Traps

1. **Substituent normalization is mandatory** — hydrogen and unmapped atoms must be in positions [1] and [3], never [0] or [2]. Every swap inverts parity. See lines 252-262, 399-423.

2. **Parity inversion formula** — `new_parity = 3 - old_parity` (because parity 1↔2). Used throughout when swapping substituents.

3. **Collinearity tolerance is loose** — 0.04 dot product threshold ≈ 16 degrees (line 186). This is intentional: "it is hard to draw a straight line accurately" (comment line 185).

4. **Zero bond length throws** — unlike cis-trans which can return false, allene stereo throws `Error("zero bond length")` (lines 172, 182, 518, 526, 536).

5. **Bond directions are relative** — `BOND_UP` (1) and `BOND_DOWN` (2) are relative to edge orientation. `markBonds` must orient edges correctly (lines 648-649) before setting direction.

6. **"Either" parity (3) is rejected** — if any substituent has `BOND_EITHER`, `parity = 3` is set (line 197), but then the center is rejected (lines 242-243). No ambiguous allene centers are stored.

7. **Implicit vs explicit hydrogen** — `pure_h[i]` is true only for `ELEM_H` with isotope 0 (lines 104-105, 116-117). Deuterium or tritium is not "pure H" and cannot be moved to [1] or [3].

8. **Z-axis drawing convention** — the algorithm assumes positive Z is "out of plane toward viewer". The correction at lines 249-250 accounts for different 2D drawing conventions.

9. **Iterator invalidation** — `removeAtoms` and `removeBonds` iterate and modify `_centers` (lines 658-717). They collect removals first (lines 661-709), then remove (lines 712-716), avoiding iterator invalidation.

10. **Sensible bonds tracking** — `buildFromBonds` marks bonds that contribute to stereochemistry in `sensible_bonds_out` (lines 228-239). These bonds must be preserved by layout/canonicalization.

11. **Pure H check at both ends required** — if both substituents on one end are pure H (or implicit), the allene is symmetric on that end and has no stereochemistry (lines 122-126).

12. **Square-planar rejection** — if geometry suggests flat configuration rather than helical, return false (lines 215-216). This catches degenerate cases.

## Dependencies

**Depends on:**
- `BaseMolecule` — atom/bond properties, graph traversal, coordinate access
- `math/algebra.h` — `Vec3f` for 3D vector operations
- `molecule/elements.h` — `ELEM_C`, `ELEM_Si`, `ELEM_H`
- `base_cpp/red_black.h` — `RedBlackMap` for sparse storage

**Depended on by:**
- `BaseMolecule` — exposes `allene_stereo` member and proxy methods (lines 767-773 in `base_molecule.h`)
- Substructure matchers — call `checkSub` for stereo validation
- Loaders/savers — `buildFromBonds` on input, `markBonds` on output
- CMF format — serializes allene stereo with `CMF_STEREO_ALLENE_0` / `_ALLENE_1` codes
- Reaction enumerator — preserves allene stereo through transformations

## Related

- Tetrahedral stereocenters: `MoleculeStereocenters` (not documented yet)
- Cis-trans double bonds: `MoleculeCisTrans` ([molecule-cis-trans.md](./molecule-cis-trans.md))
- Invariant A5: why stereo is side data, not part of atom/bond
- SMILES stereochemistry encoding: similar principles but different notation (`@AL1`, `@AL2` in extended SMILES)
