import json
import math
import os
import sys
import tempfile
from collections import defaultdict


def compare_positions(ket_a, ket_b, eps=0.05):
    """Compare two KET JSONs, allowing epsilon tolerance on position coordinates."""
    a = json.loads(ket_a)
    b = json.loads(ket_b)
    nodes_a = a.get("root", {}).get("nodes", [])
    nodes_b = b.get("root", {}).get("nodes", [])
    if len(nodes_a) != len(nodes_b):
        return "Node count mismatch: {} vs {}".format(
            len(nodes_a), len(nodes_b)
        )
    for na, nb in zip(nodes_a, nodes_b):
        if na.get("type") != nb.get("type"):
            return "Node type mismatch: {} vs {}".format(
                na.get("type"), nb.get("type")
            )
        pa = na.get("position")
        pb = nb.get("position")
        if pa and pb:
            dx = abs(pa["x"] - pb["x"])
            dy = abs(pa["y"] - pb["y"])
            if dx > eps or dy > eps:
                return (
                    "Position mismatch for id={}: "
                    "({:.4f}, {:.4f}) vs "
                    "({:.4f}, {:.4f}), "
                    "delta=({:.4f}, {:.4f})"
                ).format(
                    na.get("id"), pa["x"], pa["y"], pb["x"], pb["y"], dx, dy
                )
    return ""


sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)

from common.util import compare_diff
from env_indigo import *  # noqa

indigo = Indigo()
indigo.setOption("json-saving-pretty", True)
indigo.setOption("ignore-stereochemistry-errors", True)
indigo.setOption("json-use-native-precision", True)
indigo.setOption("json-set-native-precision", 4)

print("*** Sequence cycles layout ***")

root = joinPathPy("molecules/", __file__)
ref_path = joinPathPy("ref/", __file__)


files = [
    "pep_sel",
    "rna_sel",
    "n-agon1",
    "n-agon2",
    "rna_bicyclic",
    "rna_bicyclic_sel_2",
    "small_mol",
    "shifting_structs_1",
    "shifting_structs_2",
    "shifting_structs_1_sel",
    "shifting_structs_2_sel",
    "overlapping",
    "ring_fuse",
    "left_top_monomer",
    "6bases",
    # Issue #3606: inner base bond length for n-agon rings
    "rna_ring_6",
    "rna_ring_11",
    "rna_ring_12",
    "rna_ring_24",
]

files.sort()
for filename in files:
    try:
        mol = indigo.loadMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )
    except:
        mol = indigo.loadQueryMoleculeFromFile(
            os.path.join(root, filename + ".ket")
        )

    mol.layout()
    ket = mol.json()
    compare_diff(ref_path, filename + ".ket", ket, diff_fn=compare_positions)


# ======================================================================
# Sequential multi-cycle layout test (cycles 1, 3, 5, 7 in multi.ket)
#
# Applies layout to cycles one-by-one and verifies after each step:
#   1. The cycle becomes a regular polygon (edge = 1.5)
#   2. The left-top monomer preserves its position
#   3. Previously laid-out cycles remain regular
# ======================================================================

BOND_LENGTH = 1.5  # LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH
GEOM_TOL = 0.05


def _get_monomer_positions(data):
    pos = {}
    for key, val in data.items():
        if (
            key.startswith("monomer")
            and isinstance(val, dict)
            and "position" in val
        ):
            pos[key] = (val["position"]["x"], val["position"]["y"])
    return pos


def _get_adjacency(data):
    adj = defaultdict(set)
    for c in data["root"].get("connections", []):
        ep1 = c["endpoint1"]["monomerId"]
        ep2 = c["endpoint2"]["monomerId"]
        adj[ep1].add(ep2)
        adj[ep2].add(ep1)
    return adj


def _find_small_cycles(data):
    positions = _get_monomer_positions(data)
    adj = _get_adjacency(data)
    monomers = sorted(
        positions.keys(), key=lambda m: int(m.replace("monomer", ""))
    )
    all_cycles = set()
    for start in monomers:
        for neighbor in adj[start]:
            visited = {start: None}
            queue = [start]
            found = False
            while queue and not found:
                next_queue = []
                for cur in queue:
                    for nb in adj[cur]:
                        if nb == neighbor and cur != start:
                            path = []
                            p = cur
                            while p is not None:
                                path.append(p)
                                p = visited[p]
                            path.append(neighbor)
                            if len(path) <= 12:
                                all_cycles.add(frozenset(path))
                            found = True
                            break
                        if nb not in visited and nb != neighbor:
                            visited[nb] = cur
                            next_queue.append(nb)
                    if found:
                        break
                queue = next_queue
    cycles = []
    seen = []
    for cs in sorted(
        all_cycles, key=lambda cs: sum(positions[m][0] for m in cs) / len(cs)
    ):
        if cs not in seen:
            seen.append(cs)
            cycles.append(list(cs))
    return cycles


def _select_monomers(data, monomer_ids):
    for mid in monomer_ids:
        if mid in data:
            data[mid]["selected"] = True


def _clear_selection(data):
    for key, val in data.items():
        if key.startswith("monomer") and isinstance(val, dict):
            val.pop("selected", None)


def _do_layout(ket_data):
    ind = Indigo()
    ind.setOption("json-saving-pretty", True)
    ind.setOption("json-use-native-precision", True)
    ind.setOption("json-set-native-precision", 4)
    with tempfile.NamedTemporaryFile(
        mode="w", suffix=".ket", delete=False
    ) as f:
        json.dump(ket_data, f, indent=4)
        tmp_in = f.name
    try:
        mol = ind.loadMoleculeFromFile(tmp_in)
        mol.layout()
        return json.loads(mol.json())
    finally:
        os.unlink(tmp_in)


def _dist(p1, p2):
    return math.hypot(p1[0] - p2[0], p1[1] - p2[1])


def _find_left_top(cycle_monomers, positions):
    return min(cycle_monomers, key=lambda m: positions[m][0] - positions[m][1])


def _is_regular_polygon(
    cycle_monomers, positions, expected_edge=BOND_LENGTH, tol=GEOM_TOL
):
    n = len(cycle_monomers)
    if n < 3:
        return False, "too few vertices"
    cx = sum(positions[m][0] for m in cycle_monomers) / n
    cy = sum(positions[m][1] for m in cycle_monomers) / n
    R = expected_edge / (2 * math.sin(math.pi / n))
    for m in cycle_monomers:
        d = _dist(positions[m], (cx, cy))
        if abs(d - R) > tol:
            return False, "vertex {} dist={:.4f}, R={:.4f}".format(m, d, R)
    angles = sorted(
        (math.atan2(positions[m][1] - cy, positions[m][0] - cx), m)
        for m in cycle_monomers
    )
    ordered = [m for _, m in angles]
    for i in range(n):
        edge_len = _dist(
            positions[ordered[i]], positions[ordered[(i + 1) % n]]
        )
        if abs(edge_len - expected_edge) > tol:
            return False, "edge {}-{} len={:.4f}".format(
                ordered[i], ordered[(i + 1) % n], edge_len
            )
    return True, "ok"


print("\n*** Sequential multi-cycle layout (cycles 1,3,5,7) ***")
multi_errors = []

with open(os.path.join(root, "multi.ket")) as f:
    current_data = json.load(f)

cycle_indices = [1, 3, 5, 7]
cycle_history = {}

for idx in cycle_indices:
    cycles = _find_small_cycles(current_data)
    selected = cycles[idx - 1]
    cycle_history[idx] = selected

    positions_before = _get_monomer_positions(current_data)
    lt_id = _find_left_top(selected, positions_before)
    lt_pos_before = positions_before[lt_id]

    _clear_selection(current_data)
    _select_monomers(current_data, selected)
    current_data = _do_layout(current_data)

    positions_after = _get_monomer_positions(current_data)

    # Check regular polygon
    ok, msg = _is_regular_polygon(selected, positions_after)
    if not ok:
        multi_errors.append("Cycle {} not regular: {}".format(idx, msg))

    # Check orientation: center must be to the right of left-top vertex.
    # Note: with rotation optimization, cy != lt_y is expected — the decagon
    # is rotated to best fit the bridge bonds, not forced to horizontal.
    n = len(selected)
    cx = sum(positions_after[m][0] for m in selected) / n
    lt_pos = positions_after[lt_id]
    if cx <= lt_pos[0]:
        multi_errors.append(
            "Cycle {} center not right of left-top: cx={:.4f} lt_x={:.4f}".format(
                idx, cx, lt_pos[0]
            )
        )

    # Check previously laid-out cycles still regular
    for prev_idx in cycle_indices:
        if prev_idx >= idx:
            break
        ok, msg = _is_regular_polygon(cycle_history[prev_idx], positions_after)
        if not ok:
            multi_errors.append(
                "Cycle {} broken after step {}: {}".format(prev_idx, idx, msg)
            )

if multi_errors:
    print("multi.ket:FAILED")
    for e in multi_errors:
        print("  " + e)
else:
    final_ket = json.dumps(current_data, indent=2)
    compare_diff(
        ref_path,
        "multi_seq_1357.ket",
        final_ket,
        diff_fn=compare_positions,
    )


# ======================================================================
# Multi-cycle simultaneous selection tests
#
# Selects the union of monomers from N cycles at once and runs a single
# layout call. The first (primary) cycle goes through assignFirstCycle and
# must be a perfect regular polygon. Secondary (fused) cycles may have
# geometric distortions due to shared edges — only checked loosely.
# Fixed (unselected) monomers must not move.
# ======================================================================


def _run_multi_cycle_selection_test(label, cycle_groups, ref_filename):
    """Select union of given cycle groups simultaneously, layout once, verify."""
    with open(os.path.join(root, "multi.ket")) as f:
        data = json.load(f)

    cycles = _find_small_cycles(data)
    selected_monomers = set()
    selected_cycles = []
    for idx in cycle_groups:
        cyc = cycles[idx - 1]
        selected_cycles.append((idx, cyc))
        selected_monomers.update(cyc)

    positions_before = _get_monomer_positions(data)

    _clear_selection(data)
    _select_monomers(data, list(selected_monomers))
    data = _do_layout(data)

    positions_after = _get_monomer_positions(data)

    errors = []

    # At least one selected cycle must be a perfect regular polygon —
    # the engine picks one as primary (via assignFirstCycle) and lays it
    # out with exact geometry. Which one depends on Morgan code sorting.
    any_regular = False
    for idx, cyc in selected_cycles:
        ok, _ = _is_regular_polygon(cyc, positions_after)
        if ok:
            any_regular = True
            break
    if not any_regular:
        msgs = []
        for idx, cyc in selected_cycles:
            _, msg = _is_regular_polygon(cyc, positions_after)
            msgs.append("Cycle {}: {}".format(idx, msg))
        errors.append("No cycle is a regular polygon: " + "; ".join(msgs))

    # Fixed (unselected) monomers must not move
    for m, pos_before in positions_before.items():
        if m in selected_monomers:
            continue
        pos_after = positions_after.get(m)
        if pos_after is None:
            continue
        dx = abs(pos_after[0] - pos_before[0])
        dy = abs(pos_after[1] - pos_before[1])
        if dx > GEOM_TOL or dy > GEOM_TOL:
            errors.append(
                "Fixed monomer {} moved: before=({:.3f},{:.3f}) after=({:.3f},{:.3f})".format(
                    m, pos_before[0], pos_before[1], pos_after[0], pos_after[1]
                )
            )

    if errors:
        print("{}:FAILED".format(label))
        for e in errors:
            print("  " + e)
    else:
        final_ket = json.dumps(data, indent=2)
        compare_diff(
            ref_path,
            ref_filename,
            final_ket,
            diff_fn=compare_positions,
        )


print("\n*** Multi-cycle simultaneous layout (cycles 1+2+3) ***")
_run_multi_cycle_selection_test(
    "multi_123.ket", [1, 2, 3], "multi_sel_123.ket"
)

print("\n*** Multi-cycle simultaneous layout (cycles 2+3+4) ***")
_run_multi_cycle_selection_test(
    "multi_234.ket", [2, 3, 4], "multi_sel_234.ket"
)


# ======================================================================
# Partial-selection cycle layout: cycle_part_sel.ket
#
# A 10-vertex selected ring connected to a fixed backbone via two
# pendant phosphates (P5, P12).  The layout must:
#   1. Make the selected ring a regular 10-gon (all edges = 1.5)
#   2. Place pendant P atoms OUTSIDE the ring (outward direction)
#   3. Keep the fixed backbone atoms stationary
# ======================================================================

print("\n*** cycle_part_sel: ring + pendant phosphates + fixed backbone ***")

with open(os.path.join(root, "cycle_part_sel.ket")) as f:
    cp_data = json.load(f)

cp_out = _do_layout(cp_data)

cp_positions = _get_monomer_positions(cp_out)

cp_errors = []

# Ring0 vertices (atom indices 0,1,2,3,4,13,14,15,16,17 → monomer ids)
# Discover them: all monomers that are selected and NOT pendant phosphates.
# Selected monomers: those with "selected": true in the input.
selected_ids = {
    key
    for key, val in cp_data.items()
    if key.startswith("monomer")
    and isinstance(val, dict)
    and val.get("selected", False)
}
fixed_ids = {
    key
    for key, val in cp_data.items()
    if key.startswith("monomer")
    and isinstance(val, dict)
    and not val.get("selected", False)
}

# Find the ring cycle among selected monomers via graph cycle detection.
adj_cp = defaultdict(set)
for c in cp_data["root"].get("connections", []):
    ep1 = c["endpoint1"]["monomerId"]
    ep2 = c["endpoint2"]["monomerId"]
    if ep1 in selected_ids and ep2 in selected_ids:
        adj_cp[ep1].add(ep2)
        adj_cp[ep2].add(ep1)

# Pendant atoms: selected monomers with only 1 selected neighbour.
pendant_ids = {m for m in selected_ids if len(adj_cp[m]) <= 1}
ring_ids = selected_ids - pendant_ids

if len(pendant_ids) != 2:
    cp_errors.append(
        "Expected 2 pendant atoms, got {}".format(len(pendant_ids))
    )

if len(ring_ids) != 10:
    cp_errors.append("Expected 10 ring vertices, got {}".format(len(ring_ids)))
else:
    ok, msg = _is_regular_polygon(list(ring_ids), cp_positions)
    if not ok:
        cp_errors.append("Ring not regular polygon: " + msg)
    else:
        # Compute ring center.
        n = len(ring_ids)
        cx = sum(cp_positions[m][0] for m in ring_ids) / n
        cy = sum(cp_positions[m][1] for m in ring_ids) / n

        # Each pendant P must be OUTSIDE the ring (dot product with
        # outward radial direction at its ring neighbour > 0).
        for p in pendant_ids:
            # Find the ring-side neighbour of this pendant.
            ring_nb = None
            for c in cp_data["root"].get("connections", []):
                ep1 = c["endpoint1"]["monomerId"]
                ep2 = c["endpoint2"]["monomerId"]
                if ep1 == p and ep2 in ring_ids:
                    ring_nb = ep2
                    break
                if ep2 == p and ep1 in ring_ids:
                    ring_nb = ep1
                    break
            if ring_nb is None:
                cp_errors.append("Pendant {} has no ring neighbour".format(p))
                continue

            rn_pos = cp_positions[ring_nb]
            p_pos = cp_positions[p]

            # Outward direction at ring neighbour: from center through ring_nb.
            outward_x = rn_pos[0] - cx
            outward_y = rn_pos[1] - cy
            # Direction ring_nb → P.
            dp_x = p_pos[0] - rn_pos[0]
            dp_y = p_pos[1] - rn_pos[1]

            dot = outward_x * dp_x + outward_y * dp_y
            if dot <= 0:
                cp_errors.append(
                    "Pendant {} points inward (dot={:.4f})".format(p, dot)
                )

# Fixed backbone atoms must not move.
for m in fixed_ids:
    pos_val = cp_data[m].get("position")
    if pos_val is None:
        continue
    before = (pos_val["x"], pos_val["y"])
    after = cp_positions.get(m)
    if after is None:
        continue
    dx = abs(after[0] - before[0])
    dy = abs(after[1] - before[1])
    if dx > GEOM_TOL or dy > GEOM_TOL:
        cp_errors.append(
            "Fixed monomer {} moved: ({:.3f},{:.3f})->({:.3f},{:.3f})".format(
                m, before[0], before[1], after[0], after[1]
            )
        )

if cp_errors:
    print("cycle_part_sel.ket:FAILED")
    for e in cp_errors:
        print("  " + e)
else:
    cp_ket = json.dumps(cp_out, indent=2)
    compare_diff(
        ref_path,
        "cycle_part_sel.ket",
        cp_ket,
        diff_fn=compare_positions,
    )
