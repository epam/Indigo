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

#include "common.h"
#include "molecule/ket_commons.h"
#include "molecule/metadata_storage.h"

using namespace std;
using namespace indigo;

class IndigoCoreMetadataStorageTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreMetadataStorageTest, reset_reaction_data)
{
    MetaDataStorage ms;

    KETTextObject* to = new KETTextObject({}, "{ \"data\" : \"test\" }");
    KETSimpleObject* so = new KETSimpleObject(10, std::make_pair(Vec2f{}, Vec2f{}));
    KETReactionArrow* ra = new KETReactionArrow(15, {}, {});
    KETReactionPlus* rp = new KETReactionPlus({20, 25});

    ms.addMetaObject(ra);
    ms.addMetaObject(rp);
    ms.addMetaObject(to);
    ms.addMetaObject(so);

    ms.resetReactionData();

    ASSERT_EQ(ms.metaData().size(), 2);

    const KETTextObject& to2 = static_cast<const KETTextObject&>(ms.getMetaObject(KETTextObject::CID, 0));
    ASSERT_EQ(to2._content, "{ \"data\" : \"test\" }");

    const KETSimpleObject& so2 = static_cast<const KETSimpleObject&>(ms.getMetaObject(KETSimpleObject::CID, 0));
    ASSERT_EQ(so2._mode, 10);

    MetaDataStorage ms2(ms);

    MetaDataStorage ms3(std::move(ms2));
}
