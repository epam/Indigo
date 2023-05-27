/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __base_molecule__
#define __base_molecule__

#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"
#include "graph/graph.h"
#include "math/algebra.h"
#include "molecule/metadata_storage.h"
#include "molecule/molecule_allene_stereo.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/molecule_ionize.h"
#include "molecule/molecule_rgroups.h"
#include "molecule/molecule_sgroups.h"
#include "molecule/molecule_standardize.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molecule_tgroups.h"
#include <map>
#include <set>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    enum
    {
        CHARGE_UNKNOWN = -100
    };

    enum
    {
        ATOM_AROMATIC = 1,
        ATOM_ALIPHATIC = 2
    };

    enum
    {
        BOND_ZERO = 0,
        BOND_SINGLE = 1,
        BOND_DOUBLE = 2,
        BOND_TRIPLE = 3,
        BOND_AROMATIC = 4,
        _BOND_SINGLE_OR_DOUBLE = 5,
        _BOND_SINGLE_OR_AROMATIC = 6,
        _BOND_DOUBLE_OR_AROMATIC = 7,
        _BOND_ANY = 8,
        _BOND_COORDINATION = 9,
        _BOND_HYDROGEN = 10
    };

    enum
    {
        BOND_UP = 1,
        BOND_DOWN = 2,
        BOND_EITHER = 3
    };

    enum layout_orientation_value
    {
        UNCPECIFIED,
        HORIZONTAL,
        VERTICAL
    };

    // Flags that disables copying information in making submolecule,
    // merging with molecule and cloning procedures
    enum
    {
        SKIP_ALL = 0x7F,
        SKIP_CIS_TRANS = 0x01,
        SKIP_STEREOCENTERS = 0x02,
        SKIP_XYZ = 0x04,
        SKIP_RGROUP_FRAGMENTS = 0x08,
        SKIP_ATTACHMENT_POINTS = 0x10,
        SKIP_TGROUPS = 0x20,
        SKIP_TEMPLATE_ATTACHMENT_POINTS = 0x40,
    };

    class Molecule;
    class QueryMolecule;
    class MetaDataStorage;

    class DLLEXPORT BaseMolecule : public Graph
    {
    public:
        friend class MoleculeCIPCalculator;
        typedef std::map<int, int> Mapping;

        BaseMolecule();
        ~BaseMolecule() override;
        MetaDataStorage& meta()
        {
            return _meta;
        }
        // Casting methods. Invalid casting throws exceptions.
        virtual Molecule& asMolecule();
        virtual QueryMolecule& asQueryMolecule();
        virtual bool isQueryMolecule();

        void clear() override;
        virtual void changed() override;

        // 'neu' means 'new' in German
        virtual BaseMolecule* neu() = 0;

        virtual int getAtomNumber(int idx) = 0;      // > 0 -- ELEM_***, 0 -- pseudo-atom, -1 -- not sure
        virtual int getAtomCharge(int idx) = 0;      // charge or CHARGE_UNKNOWN if not sure
        virtual int getAtomIsotope(int idx) = 0;     // > 0 -- isotope, -1 -- not sure
        virtual int getAtomRadical(int idx) = 0;     // > 0 -- RADICAL_***, -1 -- not sure
        virtual int getAtomAromaticity(int idx) = 0; // ATOM_AROMATIC, ATOM_ALIPHATIC, or -1 -- not sure
        virtual int getExplicitValence(int idx) = 0; // explicit valence or -1 if not set
        virtual int getAtomValence(int idx) = 0;     // >= 0 -- valence, -1 is not set explicitly
        virtual int getAtomSubstCount(int idx) = 0;
        virtual int getAtomRingBondsCount(int idx) = 0; // >= 0 -- ring bonds count, -1 -- not sure
        virtual int getAtomConnectivity(int idx) = 0;

        int getAtomRadical_NoThrow(int idx, int fallback);
        int getAtomValence_NoThrow(int idx, int fallback);

        virtual int getAtomMaxH(int idx) = 0;
        virtual int getAtomMinH(int idx) = 0;
        virtual int getAtomTotalH(int idx) = 0;

        int possibleAtomTotalH(int idx, int hcount);

        virtual bool isPseudoAtom(int idx) = 0;
        virtual const char* getPseudoAtom(int idx) = 0;

        virtual bool isTemplateAtom(int idx) = 0;
        virtual const char* getTemplateAtom(int idx) = 0;
        virtual const int getTemplateAtomSeqid(int idx) = 0;
        virtual const char* getTemplateAtomClass(int idx) = 0;
        virtual const int getTemplateAtomDisplayOption(int idx) = 0;

        int countRSites();
        int countSGroups();

        static void collapse(BaseMolecule& bm, int id, Mapping& mapAtom, Mapping& mapBondInv);
        static void collapse(BaseMolecule& bm, int id);
        static void collapse(BaseMolecule& bm);

        int transformSCSRtoFullCTAB();
        int transformFullCTABtoSCSR(ObjArray<TGroup>& templates);
        int transformHELMtoSGroups(Array<char>& helm_class, Array<char>& name, Array<char>& code, Array<char>& natreplace, StringPool& r_names);

        virtual bool isRSite(int atom_idx) = 0;
        virtual dword getRSiteBits(int atom_idx) = 0;
        virtual void allowRGroupOnRSite(int atom_idx, int rg_idx) = 0;

        void getAllowedRGroups(int atom_idx, Array<int>& rgroup_list);
        int getSingleAllowedRGroup(int atom_idx);
        int getRSiteAttachmentPointByOrder(int idx, int order) const;
        void setRSiteAttachmentOrder(int atom_idx, int att_atom_idx, int order);

        void setTemplateAtomAttachmentOrder(int atom_idx, int att_atom_idx, const char* att_id);

        int getTemplateAtomAttachmentPoint(int atom_idx, int order);
        void getTemplateAtomAttachmentPointId(int atom_idx, int order, Array<char>& apid);
        int getTemplateAtomAttachmentPointsCount(int atom_idx);
        int getTemplateAtomAttachmentPointById(int atom_idx, Array<char>& att_id);

        void addAttachmentPoint(int order, int atom_index);

        int getAttachmentPoint(int order, int index) const;
        void removeAttachmentPointsFromAtom(int atom_index);
        int attachmentPointCount() const;
        void removeAttachmentPoints();

        void getAttachmentIndicesForAtom(int atom_idx, Array<int>& res);

        virtual bool isSaturatedAtom(int idx) = 0;

        virtual int getBondOrder(int idx) const = 0; // > 0 -- BOND_***, -1 -- not sure
        virtual int getBondTopology(int idx) = 0;    // > 0 -- TOPOLOGY_***, -1 -- not sure

        // true if the atom number belongs to the given list, false otherwise
        virtual bool atomNumberBelongs(int idx, const int* numbers, int count) = 0;

        // true if the atom can have that number, false otherwise
        virtual bool possibleAtomNumber(int idx, int number) = 0;

        // true if the atom can have that number and that charge, false otherwise
        virtual bool possibleAtomNumberAndCharge(int idx, int number, int charge) = 0;

        // true if the atom can have that number and that charge, false otherwise
        virtual bool possibleAtomNumberAndIsotope(int idx, int number, int isotope) = 0;

        // true if the atom can have that isotope index, false otherwise
        virtual bool possibleAtomIsotope(int idx, int isotope) = 0;

        // true if the atom can have that isotope index, false otherwise
        virtual bool possibleAtomCharge(int idx, int charge) = 0;

        // human-readable atom and bond desciptions for diagnostic purposes
        virtual void getAtomDescription(int idx, Array<char>& description) = 0;
        virtual void getBondDescription(int idx, Array<char>& description) = 0;

        // true if the bond can be that order, false otherwise
        virtual bool possibleBondOrder(int idx, int order) = 0;

        // true if bond stereoconfiguration is important
        virtual bool bondStereoCare(int idx) = 0;

        // Returns true if some bonds were changed
        virtual bool aromatize(const AromaticityOptions& options) = 0;
        // Returns true if all bonds were dearomatized
        virtual bool dearomatize(const AromaticityOptions& options) = 0;

        enum
        {
            CHANGED_ATOM_NUMBER = 0x01,
            CHANGED_CONNECTIVITY = 0x02,
            CHANGED_ALL = 0xFF,
        };
        virtual void invalidateAtom(int index, int mask);
        void addCIP();
        void clearCIP();
        CIPDesc getAtomCIP(int atom_idx);
        CIPDesc getBondCIP(int bond_idx);
        void setAtomCIP(int atom_idx, CIPDesc cip);
        void setBondCIP(int bond_idx, CIPDesc cip);

        Vec3f& getAtomXyz(int idx);
        void setAtomXyz(int idx, float x, float y, float z);
        void setAtomXyz(int idx, const Vec3f& v);

        void clearXyz();

        MoleculeStereocenters stereocenters;
        MoleculeCisTrans cis_trans;
        MoleculeAlleneStereo allene_stereo;

        bool have_xyz = false;
        bool have_cip = false;

        bool isChiral();

        Array<int>& getAAMArray()
        {
            return reaction_atom_mapping;
        }

        Array<int>& getReactingCenterArray()
        {
            return reaction_bond_reacting_center;
        }

        Array<int>& getInversionArray()
        {
            return reaction_atom_inversion;
        }

        Array<int>& getExactChangeArray()
        {
            return reaction_atom_exact_change;
        }

        struct TemplateAttPoint
        {
            int ap_occur_idx;
            int ap_aidx;
            Array<char> ap_id;
        };
        ObjPool<TemplateAttPoint> template_attachment_points;

        MoleculeSGroups sgroups;

        MoleculeTGroups tgroups;

        bool use_scsr_sgroups_only = false;
        bool remove_scsr_lgrp = false;
        bool use_scsr_name = false;
        bool expand_mod_templates = false;
        bool ignore_chem_templates = false;

        MoleculeRGroups rgroups;

        StringPool custom_collections;

        Array<char> name;

        Array<int> reaction_atom_mapping;
        Array<int> reaction_atom_inversion;
        Array<int> reaction_atom_exact_change;
        Array<int> reaction_bond_reacting_center;

        static bool hasCoord(BaseMolecule& mol);
        static bool hasZCoord(BaseMolecule& mol);

        void mergeWithSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, Array<int>* mapping_out, int skip_flags = 0);

        int mergeAtoms(int atom1, int atom2);

        void flipBond(int atom_parent, int atom_from, int atom_to);

        void makeSubmolecule(BaseMolecule& mol, const Array<int>& vertices, Array<int>* mapping_out, int skip_flags = 0);
        void makeSubmolecule(BaseMolecule& other, const Filter& filter, Array<int>* mapping_out, Array<int>* inv_mapping, int skip_flags = 0);
        void makeEdgeSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>& edges, Array<int>* v_mapping, int skip_flags = 0);

        void clone(BaseMolecule& other, Array<int>* mapping = nullptr, Array<int>* inv_mapping = nullptr, int skip_flags = 0);

        // This is a bad hack for those who are too lazy to handle the mappings.
        // NEVER USE IT.
        void clone_KeepIndices(BaseMolecule& other, int skip_flags = 0);

        void mergeWithMolecule(BaseMolecule& other, Array<int>* mapping, int skip_flags = 0);

        void removeAtoms(const Array<int>& indices);
        void removeAtoms(const Filter& filter);
        void removeAtom(int idx);
        void removeBonds(const Array<int>& indices);
        void removeBond(int idx);

        void removeSGroup(int idx);
        void removeSGroupWithBasis(int idx);

        void unhighlightAll();
        void highlightAtom(int idx);
        void highlightBond(int idx);
        void highlightAtoms(const Filter& filter);
        void highlightBonds(const Filter& filter);
        void unhighlightAtom(int idx);
        void unhighlightBond(int idx);
        int countHighlightedAtoms();
        int countHighlightedBonds();
        bool hasHighlighting();
        bool isAtomHighlighted(int idx);
        bool isBondHighlighted(int idx);
        void highlightSubmolecule(BaseMolecule& sub, const int* mapping, bool entire);

        void unselectAll();
        void selectAtom(int idx);
        void selectBond(int idx);
        void selectAtoms(const Filter& filter);
        void selectBonds(const Filter& filter);
        void getAtomSelection(std::set<int>& selection);
        void unselectAtom(int idx);
        void unselectBond(int idx);
        int countSelectedAtoms();
        int countSelectedBonds();
        bool hasSelection();
        bool isAtomSelected(int idx);
        bool isBondSelected(int idx);
        void selectSubmolecule(BaseMolecule& sub, const int* mapping, bool entire);

        static int getVacantPiOrbitals(int group, int charge, int radical, int conn, int* lonepairs_out);

        // Returns edit revision for this molecule.
        // Each time molecule is changed revision number is increased.
        // If revision number is the same then molecule hasn't been changed.
        int getEditRevision();
        // Manually update edit revision. This is required when molecule is changed
        // directly without calling molecule methods (for example mol.cis_trans.clear() and etc.)
        void updateEditRevision();

        void clearBondDirections();
        int getBondDirection(int idx) const;
        void setBondDirection(int idx, int dir);

        int getBondDirection2(int center_idx, int nei_idx);

        void mergeSGroupsWithSubmolecule(BaseMolecule& mol, Array<int>& mapping);

        void mergeSGroupsWithSubmolecule(BaseMolecule& mol, Array<int>& mapping, Array<int>& edge_mapping);
        void clearSGroups();

        void getSGroupAtomsCenterPoint(SGroup& sgroup, Vec2f& res);
        void getAtomsCenterPoint(Array<int>& atoms, Vec2f& res);

        void getAtomSymbol(int v, Array<char>& output);

        int atomCode(int idx);
        int bondCode(int idx);

        int addTemplate(TGroup& tgroup);

        int getChiralFlag();
        void setChiralFlag(int flag);

        // multifragments management methods

        void setStereoFlagPosition(int frag_index, const Vec3f& pos);
        bool getStereoFlagPosition(int frag_index, Vec3f& pos);
        int countStereoFlags();

        // proxy methods for stereocenters
        const int* getPyramidStereocenters(int idx) const;
        void markBondsStereocenters();
        void markBondStereocenters(int atom_idx);

        void addStereocenters(int atom_idx, int type, int group, const int pyramid[4]);
        void addStereocenters(int atom_idx, int type, int group, bool inverse_pyramid);
        void addStereocentersIgnoreBad(int atom_idx, int type, int group, bool inverse_pyramid);

        void removeAtomsStereocenters(const Array<int>& indices);
        void removeBondsStereocenters(const Array<int>& indices);

        void buildFromBondsStereocenters(const StereocentersOptions& options, int* sensible_bonds_out);
        void buildFrom3dCoordinatesStereocenters(const StereocentersOptions& options);
        bool isPossibleStereocenter(int atom_idx, bool* possible_implicit_h = 0, bool* possible_lone_pair = 0);
        void buildOnSubmoleculeStereocenters(const BaseMolecule& super, int* mapping);

        // proxy methods for cis_trans
        void getSubstituents_All(int bond_idx, int subst[4]);
        void restoreSubstituents(int bond_idx);
        void buildCisTrans(int* exclude_bonds);
        bool registerBondAndSubstituentsCisTrans(int idx);
        void registerUnfoldedHydrogenCisTrans(int atom_idx, int added_hydrogen);
        void buildFromSmilesCisTrans(int* dirs);
        void buildOnSubmoleculeCisTrans(BaseMolecule& super, int* mapping);
        void validateCisTrans();
        bool convertableToImplicitHydrogenCisTrans(int idx);

        // proxy methods for allene_stereo
        void markBondsAlleneStereo();
        void buildOnSubmoleculeAlleneStereo(BaseMolecule& super, int* mapping);
        void removeAtomsAlleneStereo(const Array<int>& indices);
        void removeBondsAlleneStereo(const Array<int>& indices);
        void buildFromBondsAlleneStereo(bool ignore_errors, int* sensible_bonds_out);

        // calc bounding box
        void getBoundingBox(Rect2f& bbox) const;
        void getBoundingBox(Vec2f& a, Vec2f& b) const;

        DECL_ERROR;

    protected:
        void _mergeWithSubmolecule_Sub(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, Array<int>& mapping, Array<int>& edge_mapping,
                                       int skip_flags);

        void _flipSGroupBond(SGroup& sgroup, int src_bond_idx, int new_bond_idx);
        void _flipSuperatomBond(Superatom& sa, int src_bond_idx, int new_bond_idx);
        void _flipTemplateAtomAttachmentPoint(int idx, int atom_from, Array<char>& ap_id, int atom_to);

        virtual void _mergeWithSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping,
                                           int skip_flags) = 0;

        virtual void _postMergeWithSubmolecule(BaseMolecule& mol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping,
                                               int skip_flags);

        virtual void _flipBond(int atom_parent, int atom_from, int atom_to);

        virtual void _removeAtoms(const Array<int>& indices, const int* mapping);
        virtual void _removeBonds(const Array<int>& indices);

        int _addBaseAtom();
        int _addBaseBond(int beg, int end);

        void _removeAtomsFromSGroup(SGroup& sgroup, Array<int>& mapping);
        void _removeAtomsFromMultipleGroup(MultipleGroup& mg, Array<int>& mapping);
        void _removeAtomsFromSuperatom(Superatom& sa, Array<int>& mapping);
        void _removeBondsFromSGroup(SGroup& sgroup, Array<int>& mapping);
        void _removeBondsFromSuperatom(Superatom& sa, Array<int>& mapping);
        bool _mergeSGroupWithSubmolecule(SGroup& sgroup, SGroup& super, BaseMolecule& supermol, Array<int>& mapping, Array<int>& edge_mapping);

        void _checkSgroupHierarchy(int pidx, int oidx);

        int _transformTGroupToSGroup(int idx, int t_idx);
        int _transformSGroupToTGroup(int idx, int& t_idx);
        void _fillTemplateSeqIds();
        bool _isCTerminus(Superatom& su, int idx);
        bool _isNTerminus(Superatom& su, int idx);
        int _createSGroupFromFragment(Array<int>& sg_atoms, const TGroup& tg, Array<int>& mapping);
        bool isAtomBelongsSGroup(int idx);

        Array<int> _hl_atoms;
        Array<int> _hl_bonds;
        Array<int> _sl_atoms;
        Array<int> _sl_bonds;

        Array<int> _bond_directions;

        Array<Vec3f> _xyz;
        RedBlackMap<int, Vec3f> _stereo_flag_positions;
        // CIP maps should be changed to std::unordered_map
        RedBlackMap<int, CIPDesc> _cip_atoms;
        RedBlackMap<int, CIPDesc> _cip_bonds;

        ObjArray<Array<int>> _rsite_attachment_points;
        bool _rGroupFragment;

        ObjArray<Array<int>> _attachment_index;

        int _chiral_flag = -1;

        // When molecule gets edited then edit revision is increased.
        // If edit revision is the same then molecule wasn't edited
        int _edit_revision;

        MetaDataStorage _meta;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
