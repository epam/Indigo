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

#ifndef __molecule_layout_macrocycles_h__
#define __molecule_layout_macrocycles_h__

#include "layout/molecule_layout_graph.h"
#include "molecule/molecule.h"
#include <algorithm>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

using namespace std;
namespace indigo
{

    static const unsigned short SHORT_INFINITY = 60000;
    static const int WEIGHT_FACTOR = 12;
    static const int SIX = 6;

    inline int get_weight(int weight, int rotate)
    {
        if (abs(weight) <= WEIGHT_FACTOR)
            return 0;
        return max(0, weight * (rotate > 0 ? -1 : 1));
    }

    class DLLEXPORT MoleculeLayoutMacrocycles
    {
    public:
        CP_DECL;
        MoleculeLayoutMacrocycles(int size);

        void addVertexOutsideWeight(int v, int weight);
        void setVertexEdgeParallel(int v, bool parallel);
        void set_vertex_added_square(int v, float s);
        void setEdgeStereo(int e, int stereo);
        void setVertexDrawn(int v, bool drawn);
        void set_component_finish(int v, int f);
        void set_target_angle(int v, float angle);
        void set_angle_importance(int, float);

        int getVertexStereo(int v);

        const Vec2f& getPos(int v) const;

        void doLayout();

        // private:
    public:
        static bool canApply(BaseMolecule& mol);

        float layout(BaseMolecule& mol);

        void smoothing(int ind, int molSize, int* rotateAngle, int* edgeLenght, int* vertexNumber, Vec2f* p, bool profi);
        void smoothing2(int ind, int molSize, int* rotateAngle, int* edgeLenght, int* vertexNumber, Vec2f* p);
        void improvement2(int i, int vertex_count, int cycle_size, int* rotate_angle, int* edge_lenght, int* vertex_number, Vec2f* p, int base_vertex,
                          bool fix_angle, bool fix_next, float multiplyer);
        float badness(int ind, int molSize, int* rotateAngle, int* edgeLenght, int* vertexNumber, Vec2f* p, int diff);
        float depictionMacrocycleGreed(bool profi);
        float depictionCircle();

        DECL_ERROR;

    private:
        int length;
        static const int max_size;
        static const int init_x;
        static const int init_y;
        static const int init_rot;
        static const float CHANGE_FACTOR;

        int get_diff_grid(int x, int y, int rot, int value);
        int get_diff_circle(int x, int y, int rot, int value);

        struct Data
        {
            enum
            {
                max_size = 105
            };
            unsigned short minRotates[max_size][max_size][2][max_size][max_size];
        };

        TL_CP_DECL(Data, data);
        TL_CP_DECL(Array<int>, _vertex_weight);
        TL_CP_DECL(Array<int>, _vertex_stereo);
        TL_CP_DECL(Array<float>, _vertex_added_square);
        TL_CP_DECL(Array<int>, _edge_stereo);
        TL_CP_DECL(Array<bool>, _vertex_drawn);
        TL_CP_DECL(Array<Vec2f>, _positions);
        TL_CP_DECL(Array<int>, _component_finish);
        TL_CP_DECL(Array<float>, _target_angle);
        TL_CP_DECL(Array<float>, _angle_importance);
    };

    struct answer_point;

    class DLLEXPORT MoleculeLayoutMacrocyclesLattice
    {
    public:
        CP_DECL;

        MoleculeLayoutMacrocyclesLattice(int size);

        void doLayout();

        void addVertexOutsideWeight(int v, int weight);
        void setVertexEdgeParallel(int v, bool parallel);
        bool getVertexStereo(int v);
        void setEdgeStereo(int e, int stereo);
        void setVertexAddedSquare(int v, float s);
        void setVertexDrawn(int v, bool drawn);
        void setComponentFinish(int v, int f);
        void setTargetAngle(int v, float angle);
        void setAngleImportance(int, float);

        class DLLEXPORT CycleLayout
        {
            CP_DECL;

        public:
            int vertex_count;
            TL_CP_DECL(Array<Vec2f>, point);
            TL_CP_DECL(Array<int>, rotate);
            TL_CP_DECL(Array<int>, external_vertex_number);
            TL_CP_DECL(Array<int>, edge_length);
            // TL_CP_DECL(Array<int>, component_finish);

            CycleLayout();
            void initStatic();
            void init(answer_point* points);
            void init(int* up);
            float area();
            float perimeter();
            Vec2f getWantedVector(int vertex_number);

            void soft_move_vertex(int vertex_number, Vec2f move_vector);
            void stright_rotate_chein(int vertex_number, float angle);
            void stright_move_chein(int vertex_number, Vec2f vector);

            DECL_ERROR;
        };

        void initCycleLayout(CycleLayout& cl);
        int internalValue(CycleLayout& cl);
        float rating(CycleLayout& cl);
        int period(CycleLayout& cl);
        bool is_period(CycleLayout& cl, int k);
        void closingStep(CycleLayout& cl, int index, int base_vertex, bool fix_angle, bool fix_next, float multiplyer);
        void closing(CycleLayout& cl);
        void updateTouchingPoints(Array<local_pair_id>&, CycleLayout&);
        void smoothingStep(CycleLayout& cl, int vertex_number, float coef, Array<local_pair_id>&);
        void smoothing(CycleLayout& cl);
        const Vec2f& getPos(int v) const;
        float preliminary_layout(CycleLayout& cl);

        DECL_ERROR;

    private:
        int length;
        int rotate_length;
        static const float SMOOTHING_MULTIPLIER;
        static const float CHANGE_FACTOR;

        void calculate_rotate_length();
        void rotate_cycle(int shift);
        void _rotate_ar_i(Array<int>& ar, Array<int>& tmp, int shift);
        void _rotate_ar_d(Array<float>& ar, Array<float>& tmp, int shift);
        void _rotate_ar_v(Array<Vec2f>& ar, Array<Vec2f>& tmp, int shift);
        // float rating(Array<answer_point>);

        TL_CP_DECL(Array<int>, _vertex_weight);
        TL_CP_DECL(Array<int>, _vertex_stereo);
        TL_CP_DECL(Array<int>, _edge_stereo);
        TL_CP_DECL(Array<Vec2f>, _positions);
        TL_CP_DECL(Array<float>, _vertex_added_square);
        TL_CP_DECL(Array<bool>, _vertex_drawn);
        TL_CP_DECL(Array<int>, _component_finish);
        TL_CP_DECL(Array<float>, _target_angle);
        TL_CP_DECL(Array<float>, _angle_importance);
    };

    struct rectangle
    {
        int min_x;
        int max_x;
        int min_y;
        int max_y;

        bool empty;

        rectangle()
        {
            set_empty();
        }

        rectangle(int x1, int x2, int y1, int y2)
        {
            set(x1, x2, y1, y2);
        }

        void set(int x1, int x2, int y1, int y2)
        {
            min_x = x1;
            max_x = x2;
            min_y = y1;
            max_y = y2;

            empty = (min_x > max_x) || (min_y > max_y);
        }

        void intersec(int x1, int x2, int y1, int y2)
        {
            set(max(min_x, x1), min(max_x, x2), max(min_y, y1), min(max_y, y2));
        }

        void set(rectangle rec)
        {
            set(rec.min_x, rec.max_x, rec.min_y, rec.max_y);
        }

        rectangle intersec(rectangle rec)
        {
            intersec(rec.min_x, rec.max_x, rec.min_y, rec.max_y);
            return *this;
        }

        rectangle expand(int w)
        {
            if (empty)
                return rectangle(1, 0, 1, 0);
            else
                return rectangle(min_x - w, max_x + w, min_y - w, max_y + w);
        }

        rectangle expand()
        {
            return expand(1);
        }

        void set_empty()
        {
            empty = true;
        }

        rectangle shift(int x, int y)
        {
            return rectangle(min_x + x, max_x + x, min_y + y, max_y + y);
        }

        bool contains(int x, int y)
        {
            return !empty && x >= min_x && x <= max_x && y >= min_y && y <= max_y;
        }

        static rectangle square(int x, int y, int radius)
        {
            return rectangle(x - radius, x + radius, y - radius, y + radius);
        }
    };

    class DLLEXPORT TriangleLattice
    {
    public:
        TriangleLattice();

        TriangleLattice(rectangle rec, int rem, uint8_t* data_link);

        void init(rectangle rec, int rem, uint8_t* data_link);
        void init_void();

        unsigned short& getCell(int x, int y);
        bool isValid(int x, int y);
        int getFirstValidY(int x);
        bool isIncreaseForValidY(int);
        int getFirstValidX();
        bool isIncreaseForValidX(int);
        void switchNextY(int&);

        static int getAllocationSize(rectangle rec)
        {
            if (rec.empty)
                return 0;
            int sq = (((rec.max_x - rec.min_x + 1) * (rec.max_y - rec.min_y + 1) + 2) / 3) * sizeof(unsigned short) +
                     (rec.max_x - rec.min_x + 1) * sizeof(unsigned short*);
            return sq;
        };

        DECL_ERROR;

    private:
        int _difference_reminder;
        unsigned short** _starts;

        unsigned short _sink = SHORT_INFINITY;

        rectangle _BORDER;
    };

    class DLLEXPORT AnswerField
    {
    public:
        CP_DECL;
        AnswerField(int len, int target_x, int target_y, float target_rotation, int* vertex_weight_link, int* vertex_stereo_link, int* edge_stereo_link);

        void fill();
        unsigned short& get_field(int len, answer_point p);
        unsigned short& get_field(answer_point p);
        void _restore_path(answer_point* point, answer_point finish);
        TriangleLattice& getLattice(int l, int rot, int p);

        static int _cmp_answer_points(answer_point& p1, answer_point& p2, void* context);

        DECL_ERROR;

    private:
        const int ACCEPTABLE_ERROR = 10;

        int length;

        uint8_t* _hidden_data_field;
        ObjArray<Array<rectangle>> border_array;
        Array<rectangle*> border;

        TL_CP_DECL(Array<int>, _vertex_weight);
        TL_CP_DECL(Array<int>, _vertex_stereo);
        TL_CP_DECL(Array<int>, _edge_stereo);
        TL_CP_DECL(Array<int>, _rotation_parity);
        TL_CP_DECL(Array<int>, _coord_diff_reminder); // (x - y) % 3
        TL_CP_DECL(ObjArray<ObjArray<ObjArray<TriangleLattice>>>, _lattices);
        TL_CP_DECL(Array<uint8_t>, _hidden_data_field_array);

        TriangleLattice _sink_lattice;
    };

    struct answer_point
    {
        int rot;
        int p;
        int x;
        int y;

        answer_point()
        {
            set(0, 0, 0, 0);
        }

        answer_point(int _rot, int _p, int _x, int _y)
        {
            set(_rot, _p, _x, _y);
        }

        void set(int _rot, int _p, int _x, int _y)
        {
            rot = _rot;
            p = _p;
            x = _x;
            y = _y;
        }

        const int quality(AnswerField& fld) const
        {
            int diffCoord = (x * y >= 0) ? abs(x) + abs(y) : max(abs(x), abs(y));
            return diffCoord + 2 * abs(rot - SIX) + fld.get_field(*this);
        }
    };

} // namespace indigo
#ifdef _WIN32
#pragma warning(pop)
#endif

#endif