import csv
import sys
from typing import Optional, Any
import pydot
from sklearn.metrics import r2_score

sys.path.append("../../common")
from env_indigo import dataPath, Indigo, IndigoObject, IndigoException


def build_decision_tree(filename: str) -> dict[int, dict]:
    decision_tree = {}
    graph = pydot.Dot(graph_type='graph')
    nodes = {}
    edges = []
    with open(filename) as f:
        for line in csv.DictReader(f):
            parent_id = int(line["#parent"])
            node_id = int(line['#node'])
            # print(node_id)
            node = {
                "id": node_id,
                "smarts": line["SMARTS"],
                "pka_value": float(line["pKa_cal"]),
                "pka_range": float(line["pKa_range"]),
                "yes_child_id": None,
                "no_child_id": None
            }
            decision_tree[node_id] = node
            node = pydot.Node(node_id, label=line["SMARTS"])
            nodes[node_id] = node
            is_terminal = line['children'] == '0'
            if parent_id > 0:
                if int(line["Y/N"]):
                    decision_tree[parent_id]["yes_child_id"] = node_id
                    if is_terminal:
                        node.obj_dict["attributes"]["label"] = round(float(line["pKa_cal"]), 2)
                    nodes[parent_id].obj_dict["attributes"]["label"] = line["SMARTS"]
                    edges.append(pydot.Edge(nodes[parent_id], node, label="yes"))
                else:
                    if is_terminal:
                        node.obj_dict["attributes"]["label"] = round(float(line["pKa_cal"]), 2)
                    nodes[parent_id].obj_dict["attributes"]["label"] = line["SMARTS"]
                    decision_tree[parent_id]["no_child_id"] = node_id
                    edges.append(pydot.Edge(nodes[parent_id], node, label="no"))
    for node in nodes.values():
        graph.add_node(node)
    for edge in edges:
        graph.add_edge(edge)
    graph.write("graph.svg", format="svg")
    return decision_tree


decision_tree = build_decision_tree(dataPath("molecules/pka/pka_smarts.csv"))
indigo = Indigo()
indigo.setOption("ignore-bad-valence", True)


def pka_tree(matcher: IndigoObject, node: dict[str, Any]) -> Optional[tuple[float, float]]:
    child = decision_tree.get(node.get("yes_child_id"))
    if child:
        if bool(__debug__):
            print("\t",child["smarts"], end=' ')
        query = indigo.loadSmarts(child["smarts"])
        if matcher.match(query):
            if bool(__debug__):
                print("matched")
            return pka_tree(matcher, child)
        else:
            if bool(__debug__):
                print("non_matched")
            return pka_tree(matcher, decision_tree.get(node.get("no_child_id")))
    else:
        if bool(__debug__):
            print("\t terminal", node["pka_value"])
        return round(node["pka_value"], 2), node["pka_range"]


def calc_pka(smiles: str) -> tuple[float, float]:
    mol = indigo.loadMolecule(smiles)
    mol.aromatize()
    if bool(__debug__):
        print(mol.canonicalSmiles())
    matcher = indigo.substructureMatcher(mol)
    return pka_tree(matcher, decision_tree[1])[0]

print(abs(calc_pka("Oc1cc(cc([N+](C)(C)C)c1)C") - 8.56) < 1e-1)
print(abs(calc_pka("[NH2+]1CC1c1ccc(cc1)C") - 7.73) < 1e-1)
print(abs(calc_pka("BrNC(=O)C(Cl)Cl") - 4.15) < 1e-1) 

if not bool(__debug__):
    with open(dataPath("molecules/pka/pka_answers.csv")) as f:
        y_true = []
        y_preds = []
        y_preds_crippen = []
        y_preds_cx = []
        y_preds_sparc = []
        y_preds_acd = []
        y_preds_adme_boxes = []
        for line in csv.DictReader(f):
            try:
                smiles = line["SMILES"]
                expected_pka = float(line["pKa_accepted"])
                actual_pka = calc_pka(line["SMILES"])
                y_true.append(expected_pka)
                y_preds.append(actual_pka)
                y_preds_crippen.append(float(line["SMARTS pKa"] or '0.0'))
                y_preds_cx.append(float(line["MARVIN"] or '0.0'))
                y_preds_sparc.append(float(line["SPARC"] or '0.0'))
                y_preds_acd.append(float(line["ACD"] or '0.0'))
                y_preds_adme_boxes.append(float(line["ADME Boxes"] or '0.0'))
                if abs(y_preds_crippen[-1] - actual_pka) >= 1.0:
                    print(smiles, actual_pka, y_preds_crippen[-1])
            except IndigoException as e:
                print(line)
                print(e)
        print('epam', r2_score(y_true, y_preds))
        print('crippen', r2_score(y_true, y_preds_crippen))
        print('marvin', r2_score(y_true, y_preds_cx))
        print('sparc', r2_score(y_true, y_preds_sparc))
        print('acd', r2_score(y_true, y_preds_acd))
        print('adme_boxes', r2_score(y_true, y_preds_adme_boxes))
    
    with open(dataPath("molecules/pka/pka_answers.csv")) as f:
        y_true = []
        y_preds = []
        y_preds_crippen = []
        y_preds_cx = []
        y_preds_sparc = []
        y_preds_acd = []
        y_preds_adme_boxes = []
        for line in csv.DictReader(f):
            try:
                smiles = line["SMILES"]
                expected_pka = float(line["pKa_accepted"])
                actual_pka = calc_pka(line["SMILES"])
                y_true.append(expected_pka)
                y_preds.append(actual_pka)
                y_preds_crippen.append(float(line["SMARTS pKa"] or '0.0'))
                y_preds_cx.append(float(line["MARVIN"] or '0.0'))
                y_preds_sparc.append(float(line["SPARC"] or '0.0'))
                y_preds_acd.append(float(line["ACD"] or '0.0'))
                y_preds_adme_boxes.append(float(line["ADME Boxes"] or '0.0'))
                if abs(y_preds_crippen[-1] - actual_pka) >= 1.0:
                    print(smiles, actual_pka, y_preds_crippen[-1])
            except IndigoException as e:
                print(line)
                print(e)
        print('epam', r2_score(y_true, y_preds))
        print('crippen', r2_score(y_true, y_preds_crippen))
        print('marvin', r2_score(y_true, y_preds_cx))
        print('sparc', r2_score(y_true, y_preds_sparc))
        print('acd', r2_score(y_true, y_preds_acd))
        print('adme_boxes', r2_score(y_true, y_preds_adme_boxes))

    with open(dataPath("molecules/pka/pKaInWater.csv")) as f:
            with open(dataPath("molecules/pka/pkaInWater_acidic.csv"), 'wt') as foa:
                with open(dataPath("molecules/pka/pkaInWater_basic.csv"), 'wt') as fob:
                    foa.write("Smiles,pka\n")
                    fob.write("Smiles,pka\n")
                    y_true_acidic = []
                    y_preds_acidic = []
                    y_true_basic = []
                    y_preds_basic = []
                    for line in csv.DictReader(f):
                        try:
                            smiles = line["Smiles"]
                            expected_pka = float(line["pKa"])
                            actual_pka = calc_pka(smiles)
                            if not line["basicOrAcidic"] == "acidic":
                                fob.write(f"{smiles},{expected_pka}\n")
                                y_true_acidic.append(expected_pka)
                                y_preds_acidic.append(actual_pka)
                            else:
                                foa.write(f"{smiles},{expected_pka}\n")
                                y_true_basic.append(expected_pka)
                                y_preds_basic.append(actual_pka)
                        except IndigoException as e:
                            print(e)
                    print("acidic", r2_score(y_true_acidic, y_preds_acidic))
                    print("basic", r2_score(y_true_basic, y_preds_basic))
