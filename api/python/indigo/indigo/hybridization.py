# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from enum import Enum


class Hybridization(Enum):
    S = 1
    SP = 2
    SP2 = 3
    SP3 = 4
    SP3D = 5
    SP3D2 = 6
    SP3D3 = 7
    SP3D4 = 8
    SP2D = 9
