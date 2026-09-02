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

// Characterization tests for indigo::MetaDataStorage.
//
// Rationale (task #3766): resetReactionData() had 0 direct test coverage while
// being the only removal path. MetaDataStorage keeps per-kind index arrays
// (_plus_indexes, _text_object_indexes, ...) that point into the store, so
// removal is where they can go stale. Migrated from the legacy
// PtrPool<MetaObject> to a dense MetaObjectStore: a removal now compacts the
// store and the per-kind indexes are rebuilt from the survivors. These tests
// pin the observable behavior — counts, by-kind lookup, reset — across
// removals, additions and full resets.

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
    // survivor still reachable through the rebuilt per-kind index
    EXPECT_EQ(SimpleTextObject::CID, meta.getMetaObject(SimpleTextObject::CID, 0)._class_id);
}

TEST(MetaDataStorageContract, AddAfterResetReactionDataAppends)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID));  // 0
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));    // 1
    meta.addMetaObject(new TestMeta(ReactionArrowObject::CID)); // 2
    meta.resetReactionData();                                   // compacts 0 and 2 away
    ASSERT_EQ(1, meta.metaData().size());

    meta.addMetaObject(new TestMeta(ReactionPlusObject::CID)); // appends after compaction
    EXPECT_EQ(2, meta.metaData().size());
    EXPECT_EQ(1, meta.getMetaCount(ReactionPlusObject::CID));
}

// resetReactionData() compacts the store: survivors are kept in order and the
// per-kind indexes are rebuilt, so a second reset (and every by-kind lookup)
// works on the compacted contents.
TEST(MetaDataStorageContract, ResetReactionDataTwiceAfterRemovalIsConsistent)
{
    MetaDataStorage meta;
    meta.addMetaObject(new TestMeta(ReactionArrowObject::CID)); // index 0 (reaction)
    meta.addMetaObject(new TestMeta(SimpleTextObject::CID));    // index 1 (survivor)

    meta.resetReactionData(); // compacts the arrow away; the survivor moves to index 0
    ASSERT_EQ(1, meta.metaData().size());

    EXPECT_NO_THROW(meta.resetReactionData());
    EXPECT_EQ(1, meta.metaData().size());
    EXPECT_EQ(1, meta.getMetaCount(SimpleTextObject::CID));
    EXPECT_EQ(SimpleTextObject::CID, meta.getMetaObject(SimpleTextObject::CID, 0)._class_id);
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
