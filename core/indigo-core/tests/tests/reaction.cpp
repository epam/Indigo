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

#include <gtest/gtest.h>

#include <base_cpp/output.h>
#include <reaction/reaction.h>
#include <reaction/rsmiles_saver.h>

#include "common.h"

using namespace std;
using namespace indigo;

class IndigoCoreReactionTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreReactionTest, pseudoatoms)
{
    Reaction reaction;
    loadReaction("*>>* |$Carbon;$|", reaction);
    ASSERT_STREQ("*>>* |$Carbon;A$|", saveReactionSmiles(reaction).c_str());
}

TEST_F(IndigoCoreReactionTest, aliases)
{
    Reaction reaction;
    loadReaction("C>>N |$Carbon;$|", reaction);
    ASSERT_STREQ("C>>N |$Carbon;$|", saveReactionSmiles(reaction).c_str());
    ASSERT_STREQ("$RXN\n\n -INDIGO- 0100000000\n\n  1  1\n$MOL\n\n  -INDIGO-01000000002D\n\n  1  0  0  0  0  0  0  0  0  0999 V2000\n    0.0000    0.0000    "
                 "0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\nA    1\nCarbon\nM  END\n$MOL\n\n  -INDIGO-01000000002D\n\n  1  0  0  0  0  0  0  0  0  0999 "
                 "V2000\n    0.0000    0.0000    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\nM  END\n",
                 saveRxnfle(reaction).c_str());
    ASSERT_STREQ("<?xml version=\"1.0\" ?>\n<cml>\n<reaction>\n<reactantList>\n<molecule>\n    <atomArray>\n        <atom id=\"a0\" elementType=\"C\" "
                 "mrvAlias=\"Carbon\"/>\n    </atomArray>\n</molecule>\n</reactantList>\n<productList>\n<molecule>\n    <atomArray>\n        <atom id=\"a0\" "
                 "elementType=\"N\"/>\n    </atomArray>\n</molecule>\n</productList>\n</reaction>\n</cml>\n",
                 saveReactionCML(reaction).c_str());
    // NOTE: uncomment when Ket format coordinates will be rounded up to 4 digits after dot
    /*
    ASSERT_STREQ(
        "{\"root\":{\"nodes\":[{\"$ref\":\"mol0\"},{\"$ref\":\"mol1\"},{\"type\":\"arrow\",\"data\":{\"mode\":\"open-angle\",\"pos\":[{\"x\":-2.0,\"y\":0.0,"
        "\"z\":0.0},{\"x\":2.0,\"y\":0.0,\"z\":0.0}]}}]},\"mol0\":{\"type\":\"molecule\",\"atoms\":[{\"label\":\"C\",\"alias\":\"Carbon\",\"location\":[0.0,0."
        "0,0.0]}],\"bonds\":[]},\"mol1\":{\"type\":\"molecule\",\"atoms\":[{\"label\":\"N\",\"location\":[2.0,0.0,0.0]}],\"bonds\":[]}}",
        saverReactionJson(reaction).c_str());
    */
}

TEST_F(IndigoCoreReactionTest, aliases_complex)
{
    QueryReaction reaction;
    loadQueryReaction("[C:1]=[C:2][C:3].[C:4]=[C:5][C:6]>>[C:3][C:2]=[C:5][C:6] |$;;R1;;;R2;R1;;;R2$|", reaction);
    reaction.clearAAM();
    ASSERT_STREQ("[#6]=[#6]-[#6].[#6]=[#6]-[#6]>>[#6]-[#6]=[#6]-[#6]", saveReactionSmiles(reaction, true).c_str());
    ASSERT_STREQ("$RXN\n\n -INDIGO- 0100000000\n\n  2  1\n$MOL\n\n  -INDIGO-01000000002D\n\n  3  2  0  0  0  0  0  0  0  0999 V2000\n    0.0000    0.0000    "
                 "0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.0000    0.0000   "
                 " 0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n  1  2  2  0  0  0  0\n  2  3  1  0  0  0  0\nA    3\nR1\nM  END\n$MOL\n\n  "
                 "-INDIGO-01000000002D\n\n  3  2  0  0  0  0  0  0  0  0999 V2000\n    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    "
                 "0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n  "
                 "1  2  2  0  0  0  0\n  2  3  1  0  0  0  0\nA    3\nR2\nM  END\n$MOL\n\n  -INDIGO-01000000002D\n\n  4  3  0  0  0  0  0  0  0  0999 V2000\n  "
                 "  0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n "
                 "   0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  "
                 "0\n  1  2  1  0  0  0  0\n  2  3  2  0  0  0  0\n  3  4  1  0  0  0  0\nA    1\nR1\nA    4\nR2\nM  END\n",
                 saveRxnfle(reaction).c_str());
    // NOTE: uncomment when Ket format coordinates will be rounded up to 4 digits after dot
    /*
    ASSERT_STREQ(
        "{\"root\":{\"nodes\":[{\"$ref\":\"mol0\"},{\"$ref\":\"mol1\"},{\"$ref\":\"mol2\"},{\"type\":\"plus\",\"location\":[0.0,0.0,0.0]},{\"type\":\"arrow\","
        "\"data\":{\"mode\":\"open-angle\",\"pos\":[{\"x\":-1.0,\"y\":0.0,\"z\":0.0},{\"x\":1.0,\"y\":0.0,\"z\":0.0}]}}]},\"mol0\":{\"type\":\"molecule\","
        "\"atoms\":[{\"label\":\"C\",\"location\":[0.0,0.5,0.0]},{\"label\":\"C\",\"location\":[0.8660253882408142,0.0,0.0]},{\"label\":\"C\",\"alias\":\"R1\","
        "\"location\":[1.732050895690918,0.4999999701976776,0.0]}],\"bonds\":[{\"type\":2,\"atoms\":[0,1]},{\"type\":1,\"atoms\":[1,2]}]},\"mol1\":{\"type\":"
        "\"molecule\",\"atoms\":[{\"label\":\"C\",\"location\":[3.732050895690918,0.5,0.0]},{\"label\":\"C\",\"location\":[4.598076343536377,0.0,0.0]},{"
        "\"label\":\"C\",\"alias\":\"R2\",\"location\":[5.464101791381836,0.4999999701976776,0.0]}],\"bonds\":[{\"type\":2,\"atoms\":[0,1]},{\"type\":1,"
        "\"atoms\":[1,2]}]},\"mol2\":{\"type\":\"molecule\",\"atoms\":[{\"label\":\"C\",\"alias\":\"R1\",\"location\":[0.0,2.5,0.0]},{\"label\":\"C\","
        "\"location\":[0.8660253286361694,3.0,0.0]},{\"label\":\"C\",\"location\":[1.7320506572723389,2.5,0.0]},{\"label\":\"C\",\"alias\":\"R2\",\"location\":"
        "[2.598076105117798,3.0,0.0]}],\"bonds\":[{\"type\":1,\"atoms\":[0,1]},{\"type\":2,\"atoms\":[1,2]},{\"type\":1,\"atoms\":[2,3]}]}}",
        saverReactionJson(reaction).c_str());
    */
}

TEST_F(IndigoCoreReactionTest, smarts_reaction)
{
    QueryReaction qr;
    std::string smarts_in = "([#8:1].[#6:2])>>([#8:1].[#6:2])";
    loadQueryReaction(smarts_in.c_str(), qr);
    ASSERT_EQ(qr.reactantsCount(), 1);
    ASSERT_EQ(qr.productsCount(), 1);
    Array<char> out;
    ArrayOutput std_out(out);
    RSmilesSaver saver(std_out);
    saver.smarts_mode = true;
    saver.saveQueryReaction(qr);
    out.push(0);
    std::string smarts_out{out.ptr()};
    ASSERT_EQ(smarts_in, smarts_out);
}