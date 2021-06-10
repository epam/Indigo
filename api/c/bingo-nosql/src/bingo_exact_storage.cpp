#include "bingo_exact_storage.h"

#include "base_cpp/crc32.h"
#include "bingo_mmf_storage.h"
#include "molecule/elements.h"

#include "graph/subgraph_hash.h"

using namespace bingo;

ExactStorage::ExactStorage()
{
}

BingoAddr ExactStorage::create(BingoPtr<ExactStorage>& exact_ptr)
{
    exact_ptr.allocate();
    new (exact_ptr.ptr()) ExactStorage();

    return (BingoAddr)exact_ptr;
}

void ExactStorage::load(BingoPtr<ExactStorage>& exact_ptr, BingoAddr offset)
{
    exact_ptr = BingoPtr<ExactStorage>(offset);
}

void ExactStorage::add(dword hash, int id)
{
    _molecule_hashes.add(hash, id);
}

void ExactStorage::findCandidates(dword query_hash, ArrayInt& candidates, int part_id, int part_count)
{
    profTimerStart(tsingle, "exact_filter");

    dword first_hash = 0;
    dword last_hash = (dword)(-1);

    if (part_id != -1 && part_count != -1)
    {
        first_hash = (part_id - 1) * last_hash / part_count;
        last_hash = part_id * last_hash / part_count;
    }

    if (query_hash < first_hash || query_hash > last_hash)
        return;

    Array<size_t> indices;
    _molecule_hashes.getAll(query_hash, indices);

    for (int i = 0; i < indices.size(); i++)
        candidates.push(indices[i]);
}

dword ExactStorage::calculateMolHash(Molecule& mol)
{
    QS_DEF(Molecule, mol_without_h);
    QS_DEF(ArrayInt, vertices);
    int i;

    vertices.clear();

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.getAtomNumber(i) != ELEM_H)
            vertices.push(i);

    mol_without_h.makeSubmolecule(mol, vertices, 0);

    QS_DEF(ArrayInt, vertex_codes);
    vertex_codes.clear_resize(mol_without_h.vertexEnd());

    SubgraphHash hh(mol_without_h);

    for (int v = mol_without_h.vertexBegin(); v != mol_without_h.vertexEnd(); v = mol_without_h.vertexNext(v))
        vertex_codes[v] = mol_without_h.atomCode(v);
    hh.vertex_codes = &vertex_codes;
    hh.max_iterations = (mol_without_h.edgeCount() + 1) / 2;

    return hh.getHash();
}

dword ExactStorage::calculateRxnHash(Reaction& rxn)
{
    QS_DEF(Molecule, mol_without_h);
    QS_DEF(ArrayInt, vertices);
    int i, j;
    dword hash = 0;

    for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
    {
        Molecule& mol = rxn.getMolecule(j);

        vertices.clear();

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            if (mol.getAtomNumber(i) != ELEM_H)
                vertices.push(i);

        mol_without_h.makeSubmolecule(mol, vertices, 0);
        SubgraphHash hh(mol_without_h);
        hash += hh.getHash();
    }

    return hash;
}