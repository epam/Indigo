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

// Characterization tests for indigo::MetaDataStorage (PtrPool<MetaObject>).
//
// Rationale (task #3766): MetaDataStorage::_meta_data.remove() (metadata_storage.cpp:141,144)
// is invoked only from resetReactionData() and has 0 direct test coverage in the
// whole known run. It is the pool slot-reuse path most at risk in the migration
// because MetaDataStorage keeps *side index arrays* (_plus_indexes, _text_object_indexes,
// ...) that store pool indices: after removing reaction objects the surviving
// non-reaction objects MUST keep their original pool indices so those side arrays
// stay valid. These tests pin exactly that invariant plus reuse and full reset.

#include <gtest/gtest.h>

#include <molecule/meta_commons.h>
#include <molecule/metadata_storage.h>

using namespace indigo;

namespace
{
    // Minimal concrete MetaObject: routes categorization purely by class id, so
    // tests can exercise addMetaObject/resetReactionData without constructing
    // full graphics/text payloads.
    struct TestMeta : public MetaObject
    {
        explicit TestMeta(uint32_t cid) : MetaObject(cid)
        {
        }
        MetaObject* clone() const override
        {
            return new TestMeta(_class_id);
        }
        void getBoundingBox(Rect2f&) const override
        {
        }
        void offset(const Vec2f&) override
        {
        }
    };
} // namespace

TEST(MetaDataStorageContract, AddCategorizesByClassIdAndCounts)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID));
    meta.addMetaObject(new TestMeta(ReactionArrowObject::CID));
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));

    EXPECT_EQ(3, meta.metaData().size());
    EXPECT_EQ(1, meta.getMetaCount(ReactionPlusObject::CID));
    EXPECT_EQ(1, meta.getMetaCount(ReactionArrowObject::CID));
    EXPECT_EQ(1, meta.getMetaCount(SimpleTextObject::CID));
}

// Core golden-master: resetReactionData() removes reaction objects from the
// pool but the surviving non-reaction object must stay at its original pool
// index so the side index arrays remain valid.
TEST(MetaDataStorageContract, ResetReactionDataRemovesReactionObjectsKeepsSurvivorIndex)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID));  // index 0
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));    // index 1
    meta.addMetaObject(new TestMeta(ReactionArrowObject::CID)); // index 2
    ASSERT_EQ(3, meta.metaData().size());

    meta.resetReactionData();

    EXPECT_EQ(1, meta.metaData().size());
    EXPECT_EQ(0, meta.getMetaCount(ReactionPlusObject::CID));
    EXPECT_EQ(0, meta.getMetaCount(ReactionArrowObject::CID));
    EXPECT_EQ(1, meta.getMetaCount(SimpleTextObject::CID));
    // survivor still reachable at its original pool index via side array
    EXPECT_EQ(SimpleTextObject::CID, meta.getMetaObject(SimpleTextObject::CID, 0)._class_id);
}

TEST(MetaDataStorageContract, AddAfterResetReactionDataReusesFreedSlot)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID));  // 0
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));    // 1
    meta.addMetaObject(new TestMeta(ReactionArrowObject::CID)); // 2
    meta.resetReactionData(); // frees slots 0 and 2
    ASSERT_EQ(1, meta.metaData().size());

    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID)); // reuses a freed slot
    EXPECT_EQ(2, meta.metaData().size());
    EXPECT_EQ(1, meta.getMetaCount(ReactionPlusObject::CID));
}

TEST(MetaDataStorageContract, ResetMetaDataClearsEverythingAndIsReusable)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID));
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));
    meta.resetMetaData();

    EXPECT_EQ(0, meta.metaData().size());
    EXPECT_EQ(0, meta.getMetaCount(ReactionPlusObject::CID));
    EXPECT_EQ(0, meta.getMetaCount(SimpleTextObject::CID));

    // pool restarts from 0 after a full reset
    int idx = meta.addMetaObject(new TestMeta(SimpleTextObject::CID));
    EXPECT_EQ(0, idx);
    EXPECT_EQ(1, meta.metaData().size());
}
