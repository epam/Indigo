# MoleculeStereocenters

> **Read when:** working with tetrahedral stereochemistry, chiral atoms, stereocenter detection from 3D coordinates or bond wedges, substructure matching with chirality constraints, or enhanced stereochemistry (AND/OR groups).
> **Skip when:** working only with double bond stereochemistry (see `MoleculeCisTrans`), allene stereochemistry (see `MoleculeAlleneStereo`), or structures without chiral centers.

Manages **tetrahedral (atom) stereochemistry** and **atropisomeric (axial) stereochemistry** in ring systems. A stereocenter is an atom (typically sp³ carbon) with four different substituents arranged tetrahedrally, giving rise to chirality. This is *side data* keyed by atom index (see invariant A5).

## Responsibility

- Decide which atoms can be stereocenters, by the chiral-center definition of issue #3853 that `MoleculeCIPCalc` applies (`isPossibleStereocenter`)
- Compute configuration from 3D coordinates (`buildFrom3dCoordinates`) or bond directions (`buildFromBonds`)
- Support enhanced stereochemistry: absolute (ABS), AND groups, OR groups, and ANY (either)
- Detect atropisomeric centers in ring systems with restricted rotation (`isPossibleAtropocenter`)
- Map configuration through atom mappings for substructure matching (`checkSub`, `isPyramidMappingRigid`)
- Serialize/restore bond direction markers (wedges) for stereochemistry preservation (`markBonds`)
- Update configuration when molecular topology changes (`removeAtoms`, `removeBonds`, `flipBond`, `registerUnfoldedHydrogen`)

Does **not** handle:
- Cis-trans (E/Z) stereochemistry on double bonds — see `MoleculeCisTrans`
- Allene stereochemistry — see `MoleculeAlleneStereo`
- Planar chirality or other non-tetrahedral/non-atropisomeric stereochemistry

## Public Interface

Core in `core/indigo-core/molecule/molecule_stereocenters.h`:

**Lifecycle:**
- `buildFromBonds(BaseMolecule&, const StereocentersOptions&, int* sensible_bonds_out)` — detect from bond directions (wedges)
- `buildFrom3dCoordinates(BaseMolecule&, const StereocentersOptions&)` — compute from 3D geometry
- `buildOnSubmolecule(BaseMolecule&, const BaseMolecule& super, int* mapping)` — transfer from supermolecule
- `clear()` — remove all stereocenter data

**Query:**
- `isPossibleStereocenter(BaseMolecule&, int atom_idx, bool* possible_implicit_h, bool* possible_lone_pair)` — check eligibility
- `isPossibleStereocenter(BaseMolecule&, int atom_idx, int pyramid[4], bool& is_either, bool bidirectional_mode, bool bidirectional_either_mode, const Array<bool>& bond_ignore)` — the same check, then the configuration the stereo bonds give; pyramid left all `-1` when they give none
- `isPossibleAtropocenter(BaseMolecule&, int atom_idx, int& possible_atropo_bond)` — check for atropisomerism
- `exists(int atom_idx)` — true if this atom is a stereocenter
- `getType(int idx)` → `ATOM_ANY`, `ATOM_AND`, `ATOM_OR`, or `ATOM_ABS`
- `getGroup(int idx)` — stereo group number for AND/OR types
- `getPyramid(int idx)` → `int[4]` where `[0,1,2]` go CCW when looking from `[3]`
- `isTetrahydral(int idx)` — true if tetrahedral (vs atropisomeric only)
- `isAtropisomeric(int idx)` — true if has atropisomeric component
- `size()`, `begin()`, `end()`, `next(int i)` — iterator interface

**Mutation:**
- `add(BaseMolecule&, int atom_idx, int type, int group, bool inverse_pyramid)` — manually add stereocenter
- `add(BaseMolecule&, int atom_idx, int type, int group, const int pyramid[4])` — add with explicit pyramid
- `setType(int idx, int type)`, `setGroup(int idx, int group)` — modify properties
- `invertPyramid(int idx)` — flip configuration (swap [0]↔[1])
- `remove(int idx)` — remove this stereocenter
- `markBonds(BaseMolecule&)` — set bond directions (wedges) to encode stereochemistry
- `markBond(BaseMolecule&, int atom_idx)` — mark bonds for one stereocenter

**Topology updates:**
- `removeAtoms(BaseMolecule&, const Array<int>& indices)` — update when atoms removed
- `removeBonds(BaseMolecule&, const Array<int>& indices)` — update when bonds removed
- `registerUnfoldedHydrogen(int atom_idx, int added_hydrogen)` — explicit H added
- `flipBond(int atom_parent, int atom_from, int atom_to)` — update when bond flipped

**Matching:**
- `checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping, bool reset_h_isotopes, Filter*)` — validate all query stereocenters match target
- `isPyramidMappingRigid(const int mapping[4])` — check if mapping preserves configuration
- `getMdlParity(BaseMolecule&, int idx)` → `MDL_PARITY_ODD` (1) or `MDL_PARITY_EVEN` (2) per CTfile spec

**Groups:**
- `getAbsAtoms(Array<int>&)`, `getOrGroups(Array<int>&)`, `getAndGroups(Array<int>&)` — enumerate by type
- `haveAbs()`, `haveAllAbs()`, `haveAllAbsAny()`, `haveAllAndAny()` — check group presence

**Utility:**
- `isAutomorphism(BaseMolecule&, const Array<int>& mapping, const Filter*)` — check if mapping preserves stereochemistry

## Storage Model

One `_Atom` entry per stereocenter in `_stereocenters` RedBlackMap:

```cpp
struct _Atom {
    int type;              // ATOM_ANY (1), ATOM_AND (2), ATOM_OR (3), ATOM_ABS (4)
    int group;             // stereogroup index (1-based)
    bool is_atropisomeric; // has atropisomeric component
    bool is_tetrahydral;   // has tetrahedral component
    int pyramid[4];        // [X, Y, Z, W] where X,Y,Z go CCW looking from W
                          // -1 = implicit hydrogen or lone pair
}
```

Nothing records whether a `-1` stands for a hydrogen or a lone pair; the eligibility check reports it
(`possible_implicit_h` / `possible_lone_pair`) only while it runs.

One `_AtropoCenter` entry per atropisomeric center in `_atropocenters` RedBlackObjMap:

```cpp
struct _AtropoCenter {
    int atropo_bond;                      // the restricted rotation bond
    RedBlackMap<int, int> bond_directions; // bond_idx → direction for ring stereobonds
}
```

**Pyramid convention**: When viewing the stereocenter from position `[3]` (the "behind" atom), the three "front" atoms `[0]`, `[1]`, `[2]` are arranged **counterclockwise**. This is the canonical representation; swapping any two atoms inverts the configuration.

**Type hierarchy**: `ATOM_ANY` (1) < `ATOM_AND` (2) < `ATOM_OR` (3) < `ATOM_ABS` (4). Query type ≤ target type for successful matching.

## Eligibility Rules

Since #3853 there is no table. `isPossibleStereocenter` runs the candidate test of
`MoleculeCIPCalc::_collectAtomCandidates` on the molecule itself instead of on a copy with unfolded
hydrogens (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#MoleculeStereocenters::isPossibleStereocenter`).
Everything a caller asks — the SMILES loader for `@`, the structure checker for undefined stereo,
standardize, the 3D builder — gets this one answer.

1. **Degree** 3 or 4, every bond single or double. A query bond of uncertain order counts as single
   if it may be, else as double.
2. **Element** through its isoelectronic equivalent, atomic number minus charge:
   `core/indigo-core/molecule/molecule_cip_calc.h#isAppropriateAtom` admits C, N, Si, P, S, As, Se, Te
   — so N+ and B− count as C, S+ as P — with a ring rule for N (all three bonds in rings, at most two
   to sp² atoms) and a rule for P/As (three single bonds none to H, or two single and one double).
3. **Double bonds** only on P, As, S, Se and Te equivalents.
4. **Four substituents** once the implicit hydrogens and lone pairs are counted. Lone pairs come from
   the free valence electrons, group − bonds − H − charge − radical, halved.
5. **Lone-pair rules**: P/As double bonds + lone pairs ≤ 1; S/Se/Te 0 or 2; N exactly its one lone
   pair; C and Si none.

Three cases are admitted here that `MoleculeCIPCalc` still rejects, so CIP labels did not move:
- N with its lone pair — the CIP code rejects every lone pair on N, which leaves its N ring rule
  unreachable;
- P/As with three single bonds and one double (req. 2.3; `isAppropriateAtom` checks two and one), as
  in phosphonates and phosphates;
- the lone pair of an expanded octet such as a sulfoxide S, which `getVacantPiOrbitals` reports as
  none because it finds no vacant orbital.

What this drops compared with the old table: N with a bond outside a ring (acyclic amines, N-alkyl
piperidines and aziridines, tropane-type N-methyl bridgeheads, ring NH), neutral N with four bonds one
of them double, C or Si with three neighbours and no H, neutral As with four single bonds, P bonded to H. What it
adds: Se and Te, As with three bonds, H-phosphinates.

`possible_implicit_h` is set when the fourth substituent is a hydrogen, `possible_lone_pair` when it
is a lone pair; the degree-3 wedge code relaxes two errors for a lone pair (see below).

**Queries.** A query atom of uncertain element is possible when one element it admits is. An
uncertain charge is read as neutral unless the query rules neutral out for that element, and only
then as ±1. The hydrogen count of a query atom is not known: a position its lone pairs leave free is
taken to be a hydrogen when an electron is left to bond it. See invariant A8a for why the charge rule
is as narrow as it is.

**Atropisomeric eligibility** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#isPossibleAtropocenter`):
- Atom is in a ring
- Has at least one bond direction (wedge) on a neighbor
- Has a single chain bond to another ring atom (potential axis of restricted rotation)
- Recursive search finds additional stereobonds within connected rings

## Geometric Algorithms

### `_xyzzy` — Angle Classification

**Purpose:** Determine which angular sector vector `u` lies in relative to the angle between `v1` and `v2`.

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#_xyzzy`):

1. Compute cross product and dot product between `v1` and `v2`:
   ```cpp
   sine1 = (v1 × v2).z
   cosine1 = dot(v1, v2)
   ```

2. Compute cross product and dot product between `v1` and `u`:
   ```cpp
   sine2 = (v1 × u).z
   cosine2 = dot(v1, u)
   ```

3. Return sector code:
   - If `sine1 ≈ 0` (v1 and v2 collinear):
     - `4` if `sine2 > 0` (positive straight angle)
     - `8` if `sine2 < 0` (negative straight angle)
   - If `sine1 * sine2 < 0` (opposite sides): `2` (bigger angle)
   - If `cosine2 < cosine1`: `2` (bigger angle)
   - Otherwise: `1` (smaller angle)

**Tolerance**: `1e-3` for sine comparisons

**Use case**: In degree-4 stereocenters, determines which bond is opposite to the main bond

### `_sign` — Pyramid Orientation

**Purpose:** Determine handedness of triangle formed by three vectors in 2D projection.

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#_sign`):

1. **Validate angles** between all three vector pairs:
   - Check `dot(vi, vj) < 0.997 * |vi| * |vj|` (equivalent to >4.5° angle)
   - Throw error if any angle too small

2. **Compute 2D cross product**:
   ```cpp
   res = (v1.x - v3.x) * (v2.y - v3.y) - (v1.y - v3.y) * (v2.x - v3.x)
   ```

3. Return sign:
   - `1` if `res > 1e-3` (CCW)
   - `-1` if `res < -1e-3` (CW)
   - Error if near zero (degenerate)

**Tolerances**: 0.997 dot product (~4.5°), 1e-3 for cross product

**Use case**: Determines if pyramid `[0,1,2]` is oriented correctly when looking from `[3]`

### `_onPlane` — Coplanarity Test

**Purpose:** Check if four points are coplanar and determine handedness.

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#_onPlane`):

1. **Compute normalized vectors** from `u` to each of `v1`, `v2`, `v3`

2. **Angle sum test** for coplanarity:
   - Compute pairwise angles: `a12`, `a23`, `a13`
   - If `|a12 + a23 + a13 - 2π| < 0.1` → coplanar, return `0`

3. **Handedness** via scalar triple product:
   - Compute `det = v1u · (v2u × v3u)`
   - Return `1` if positive, `-1` if negative

**Tolerance**: 0.1 radians (~5.7°) for angle sum

**Use case**: In `buildFrom3dCoordinates`, checks if degree-3 center with implicit H is planar

## Configuration Determination

### From 3D Coordinates

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#_buildOneFrom3dCoordinates`):

1. **Check eligibility** via `isPossibleStereocenter`

2. **Restore pyramid** structure

3. **Handle implicit hydrogen** (degree 3):
   - Check if three substituents + center are coplanar via `_onPlane`
   - If planar, no stereochemistry
   - Compute implicit H position as negative sum of three normalized vectors

4. **Determine orientation** via `_onPlane` of four substituents
   - If sign > 0 → inverse pyramid (`add(..., true)`)
   - If sign < 0 → normal pyramid (`add(..., false)`)
   - If sign = 0 → not a stereocenter

5. **Validate with automorphism search**:
   - Detect invalid stereocenters that are symmetric under automorphism
   - Remove such centers

### From Bond Directions (Wedges)

The work is split in two. `_buildOneCenter`
(`core/indigo-core/molecule/src/molecule_stereocenters.cpp#MoleculeStereocenters::_buildOneCenter`)
handles the atropisomer case, calls the pyramid overload of `isPossibleStereocenter`, sets `ATOM_ANY`
when it reports an either bond, marks the sensible bonds and stores the center. The overload holds the
geometry below and throws the wedge errors; it writes the pyramid only after every check has passed.

**Phase 1: Setup and eligibility**

1. The eligibility check above, which also yields `possible_lone_pair`
2. Collect neighbor edges, compute vectors, rank by atom index
3. Pure hydrogen (H with isotope 0) gets rank 10000, others get atom index

**Phase 2: Handle EITHER bonds**

- If any bond is `BOND_EITHER` → the pyramid is the neighbors in vertex order and `is_either` is set;
  `_buildOneCenter` makes the type `ATOM_ANY`
- Mark all directed bonds as sensible

**Phase 3: Degree 4 — Four substituents**

1. **Sort edges** by rank (bubble sort)

2. **Find main bond** — first bond with UP or DOWN direction

3. **Validate**:
   - No zero bond lengths (throw error)
   - At most 1 pure hydrogen (throw error)

4. **Find opposite bond** via `_xyzzy` tests:
   - Try each of three other bonds
   - Check if `_xyzzy(main, candidate, other1) + _xyzzy(main, candidate, other2) ∈ {3, 12}`
   - These sums indicate the two "other" bonds are on opposite sides of the main-candidate axis

5. **Validate bond directions**:
   - Opposite bonds must not have same explicit direction (UP/UP or DOWN/DOWN)
   - Non-opposite bonds must not have same direction as main

6. **Determine last atom direction**:
   - If main or opposite is in position [3] → keep main direction
   - Otherwise → invert main direction

7. **Compute pyramid** via `_sign`:
   - Check handedness of first three substituents
   - If `(last_dir==UP && sign>0) || (last_dir==DOWN && sign<0)` → `[0,1,2,3]`
   - Otherwise → `[0,2,1,3]` (swap [1] and [2])

**Phase 4: Degree 3 — Three substituents + implicit H**

1. **Sort edges** by rank (bubble sort)

2. **Read directions** and count UP/DOWN

3. **Analyze geometry** via `_xyzzy` for each neighbor:
   - `_xyzzy((i+1), (i+2), i)` determines if neighbor `i` is in special position
   - `xyzzy==1` → `main_nei = i` (all three in same half-plane)
   - `xyzzy==2` → `degenerate = false`

4. **Determine direction inversion**:
   - **If main_nei found**:
     - If main_nei has direction: others must differ, UP → dir=-1
     - If main_nei no direction: others must agree, DOWN → dir=-1
   - **If no main_nei but not degenerate**:
     - Cannot mix UP and DOWN (throw error)
     - For lone pair atoms: can have all 3 UP or all 3 DOWN
     - Otherwise: at least one of each, DOWN → dir=-1
   - **If degenerate**: throw error

5. **Additional validations**:
   - No zero bond lengths
   - If explicit H present, must be lone pair atom

6. **Compute pyramid** via `_sign`:
   - If `sign == dir` → `[0,2,1,-1]`
   - Otherwise → `[0,1,2,-1]`

**Phase 5: Finalization** (back in `_buildOneCenter`)

- A pyramid still all `-1` means the bonds define no configuration: not a stereocenter
- Mark sensible bonds
- Insert into `_stereocenters` map

**Bidirectional mode**: If enabled, reads bond directions in both orientations — UP from neighbor
means DOWN to neighbor (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#MoleculeStereocenters::_getDirection`)

## Pyramid Matching (Substructure)

### `isPyramidMappingRigid` — Parity Check

**Purpose:** Determine if atom mapping preserves stereocenter configuration.

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#isPyramidMappingRigid`):

1. Copy mapping to temporary array
2. **Bubble sort by mapped indices**
3. **Count swaps** during sort via `rigid` flag toggling
4. Return `rigid`:
   - `true` = even number of swaps = configuration preserved
   - `false` = odd number of swaps = configuration inverted

**Key insight**: A permutation is even/odd based on swap count. Since pyramid is defined by CCW ordering, an odd permutation inverts the handedness.

**Variant for degree 3**:
- Rotate mapped indices until minimum is at position [0]
- Each rotation is one swap
- Check if result is `[min, mid, max]` order

### `checkSub` — Validate Stereo Match

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#checkSub`):

1. **For each query stereocenter**:

2. **Skip non-constraining types**:
   - Skip if type < `ATOM_AND` (only `ATOM_ANY` is skipped)
   - Skip if not tetrahedral

3. **ATOM_ABS (absolute) type**:
   - Get pyramid mapping via `getPyramidMapping`
   - Check if `isPyramidMappingRigid` returns true
   - Return false if configuration inverted

4. **ATOM_OR or ATOM_AND groups**:
   - Collect all query stereocenters in same group
   - Verify all have corresponding target stereocenters
   - Check target types form valid hierarchy
   - **For AND groups**: all targets must be in same AND group
   - **For OR groups**: all targets must be in same OR group
   - **Mixed with ABS**: targets can be ABS
   - Parity check varies by group semantics

## CTfile MDL Parity

**Algorithm** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#getMdlParity`):

Per CTfile Appendix A specification:

1. **Number atoms** 1-4 in order of increasing atom index, with implicit H as highest

2. **View from position 4** looking toward plane of 1,2,3

3. **Check pyramid rigidity**:
   - If `isPyramidMappingRigid(pyramid)` → `MDL_PARITY_ODD` (1)
   - Otherwise → `MDL_PARITY_EVEN` (2)

4. **Special cases**:
   - No stereocenter or non-tetrahedral → `MDL_PARITY_NONE` (0)
   - `ATOM_ANY` type → `MDL_PARITY_EITHER` (3)

**Pure hydrogen normalization**:
- Replace pure H (isotope 0) with `vertexEnd()` value
- For query molecules, check if isotope constraint absent

## Enhanced Stereochemistry

**Types**:
- **ATOM_ANY (1)**: Either configuration (racemic, unknown)
- **ATOM_AND (2)**: AND group — all must have specified absolute configuration together
- **ATOM_OR (3)**: OR group — one enantiomer or the other (but not mixed)
- **ATOM_ABS (4)**: Absolute configuration

**Group semantics**:
- AND/OR centers share a `group` number (1-based)
- Multiple AND groups are independent constraints
- Multiple OR groups are independent enantiomeric choices
- ABS centers stand alone (group number ignored)

**Hierarchy**: Query type ≤ target type for matching. E.g., query `ATOM_OR` matches target `ATOM_ABS`.

## Atropisomerism

**Definition**: Stereochemistry arising from restricted rotation around a single bond in a ring system.

**Detection** (`core/indigo-core/molecule/src/molecule_stereocenters.cpp#isPossibleAtropocenter`):
1. Atom is in a ring
2. Has at least one bond direction on a neighbor
3. Has a chain bond to another ring atom (potential axis)
4. Recursive search (`findAtropoStereobonds`) finds ≥1 additional directed ring bonds

**Storage**:
- `_atropocenters` map stores `atropo_bond` (the axis) and `bond_directions` (contributing bonds)
- An atom can be both tetrahedral and atropisomeric

**Marking**:
- `markAtropisomericBond` called separately after `markBond`

## Constraints & Traps

1. **Pyramid normalization is critical** — implicit H must be in position [3], not [0-2]. Swapping [0]↔[1] inverts configuration. A pyramid with wrong ordering gives incorrect stereochemistry.

2. **Angle tolerance in `_sign`** — 0.997 dot product (~4.5°) to avoid degenerate triangles. Tighter tolerance would reject valid but poorly drawn structures.

3. **Bidirectional mode** — when enabled, UP from atom A to B means DOWN from B to A. This matches ChemDraw convention where bond direction is relative to drawing order.

4. **Pure hydrogen definition** — only `ELEM_H` with `isotope==0` is "pure". Deuterium/tritium are not pure H and do not move to position [3].

5. **Degree-3 "main_nei" logic** — `_xyzzy` return values `{1,2,4,8}` indicate angular sectors. The combination determines if all three substituents are in one half-plane, which changes how directions are interpreted.

6. **Haworth projection** — `buildFromBonds` first checks for Haworth sugar projections. These have special rules for stereochemistry that override normal wedge interpretation.

7. **Zero bond length throws** — unlike cis-trans/allene which may return false, stereocenter detection throws `Error("zero bond length")`. This is a hard error, not a "no stereocenter" result.

8. **Iterator invalidation** — `removeAtoms`/`removeBonds` iterate over `_stereocenters` while potentially modifying it. Must collect removals first, then remove (same pattern as allene).

9. **Pyramid swap semantics** — `invertPyramid` swaps [0]↔[1]. This is an inversion. Swapping [1]↔[2] is also an inversion. Swapping [0]↔[2] rotates the pyramid but does not invert (still CCW from same view).

10. **Type hierarchy in matching** — `checkSub` allows query type ≤ target type. This means query `ATOM_AND` matches target `ATOM_ABS`, but not vice versa. Query acts as constraint; target can be more specific.

11. **Group validation is stateful** — `checkSub` uses `stereo_group_and`, `stereo_group_or`, `have_abs` to enforce mutual exclusivity. A group cannot mix AND with OR, or either with ABS.

12. **Automorphism invalidation** — `buildFrom3dCoordinates` runs `MoleculeAutomorphismSearch` after detection. Symmetric stereocenters (e.g., meso compounds) are removed. This is not done in `buildFromBonds` because explicit wedges override symmetry.

13. **Lone pair stereochemistry** — nothing stored tells a lone pair from an implicit H at
    `pyramid[3] == -1`. The eligibility check reports it while it runs, and for a lone pair the
    degree-3 wedge code accepts all three bonds up or down and an explicit H among the neighbours.

14. **The definition exists twice.** `MoleculeCIPCalc::_collectAtomCandidates`
    (`core/indigo-core/molecule/src/molecule_cip_calc.cpp#MoleculeCIPCalc::_collectAtomCandidates`)
    decides which atoms get R/S labels; `isPossibleStereocenter` decides which atoms can be
    stereocenters at all. They share `isAppropriateAtom` and differ on purpose in the three cases
    listed under Eligibility Rules. A fix to the definition in one is checked against the other, and
    the three differences either stay deliberate or are removed from both.

15. **Narrowing the definition changes which files load.** With `ignore-stereochemistry-errors` off, a
    stereo mark on an atom that is not a possible stereocenter is an error, not a dropped mark:
    `core/indigo-core/molecule/src/smiles_loader_stereo.cpp#chirality not possible on atom`,
    `core/indigo-core/molecule/src/molfile_loader_postload.cpp#makes no sense` for an unused wedge,
    `core/indigo-core/molecule/src/molfile_loader_postload.cpp#stereo type specified for atom` for an
    enhanced-stereo label (the KET and CDXML loaders have the same check). Under #3853 that made
    N-alkyl aziridines of `data/molecules/basic/tetrahedral-all.cml` and one of the 100 000 SMILES
    of `sample_100000.smi` unloadable. With the option on, a label on such an atom is kept as an
    invalid stereocenter
    (`core/indigo-core/molecule/src/molfile_loader_postload.cpp#addStereocentersIgnoreBad`), so a
    lenient load can still print `[N@]` on an atom this class says cannot be one.

16. **Query eligibility never exceeds the molecule's** — invariant A8a. Reading a query atom's open
    charge as anything but neutral is how it breaks.

17. **Eligibility follows the hydrogen count.** For a `Molecule` the check reads `getImplicitH` and
    `getAtomRadical` of the molecule itself (`MoleculeCIPCalc` reads them on its unfolded copy), so an
    explicit count decides: `[C@](C)(O)N`, a bracket carbon with no H, is a radical and not a center.

## Dependencies

**Depends on:**
- `BaseMolecule` — atom/bond properties, graph traversal, coordinate access
- `math/algebra.h` — `Vec3f` for 3D vector operations
- `molecule/elements.h` — `ELEM_C`, `ELEM_N`, `ELEM_H`, etc.
- `molecule/molecule_stereocenter_options.h` — `StereocentersOptions` for configuration
- `molecule/haworth_projection_finder.h` — special case for sugar structures
- `molecule/molecule_automorphism_search.h` — symmetry detection for 3D builds
- `molecule/molecule_cip_calc.h` — `MoleculeCIPCalc::isAppropriateAtom`, the element rules of the chiral-center definition
- `base_cpp/red_black.h` — `RedBlackMap` for sparse storage

**Depended on by:**
- `BaseMolecule` — exposes `stereocenters` member and proxy methods
- Everything that asks whether an atom can be a center, through `isPossibleStereocenter`: the SMILES
  loader for `@`/`@@` and CXSMILES wiggly bonds, the molfile parity path, the structure checker's
  undefined-stereo report, standardize, the 3D builder
- Substructure matchers — call `checkSub` for stereo validation
- Loaders/savers — `buildFromBonds` on input, `markBonds`/`getMdlParity` on output
- Canonical SMILES — stereochemistry affects canonical order
- InChI generation — stereo layer comes from stereocenters
- Reaction enumerator — preserves stereochemistry through transformations

## Related

- Cis-trans double bonds: `MoleculeCisTrans`
- Allene stereochemistry: `MoleculeAlleneStereo`
- CIP labels: `MoleculeCIPCalc`, which shares the chiral-center definition (trap 14)
- Invariant A5: why stereo is side data, not part of atom/bond
- CTfile Appendix A: MDL parity specification
- Enhanced stereochemistry: V3000 STE/ABS/AND/OR groups
