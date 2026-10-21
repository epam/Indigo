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

#include "layout/bracket_spacing.h"

#include <algorithm>
#include <numeric>

#include "layout/sgroup_texts.h"

using namespace indigo;

BracketSpacing::BracketSpacing(const SGroupTexts& texts, float bond_length) : _texts(texts), _bondLength(bond_length)
{
}

std::vector<std::optional<float>> BracketSpacing::distances(const Gap& gap) const
{
    const size_t count = gap.brackets.size();
    if (_fits(gap))
        return std::vector<std::optional<float>>(count);
    if (const std::optional<std::vector<float>> reduced = _atReducedStep(gap))
        return std::vector<std::optional<float>>(reduced->begin(), reduced->end());
    return _nearCount(gap) == count ? _beforeLoneAtom(gap) : _betweenGroups(gap);
}

// Whether every bracket can stand at STEP from the bounding box of its own group: those around the near atom counted
// from the near end of the gap, those around the far atom from its far end, the texts between them kept clear
bool BracketSpacing::_fits(const Gap& gap) const
{
    const float step = STEP * _bondLength, clearance = TEXT_CLEARANCE * _bondLength;
    const std::vector<float> text_widths = _textWidths(gap);
    const size_t count = gap.brackets.size(), near_count = _nearCount(gap);
    float near_end = step;
    for (size_t k = 0; k + 1 < near_count; k++)
        near_end += std::max(step, text_widths[k]);
    if (near_count == count)
        return near_end <= gap.length - clearance;
    float far_end = gap.length - step;
    for (size_t k = near_count; k + 1 < count; k++)
        far_end -= std::max(step, text_widths[k]);
    return far_end - near_end >= std::max(text_widths[near_count - 1], clearance);
}

std::optional<std::vector<float>> BracketSpacing::_atReducedStep(const Gap& gap) const
{
    const std::vector<float> text_widths = _textWidths(gap);
    const float step = _reducedStep(gap.length, text_widths);
    if (step < MIN_STEP * _bondLength)
        return std::nullopt;
    std::vector<float> distances{step};
    for (float text_width : text_widths)
        distances.push_back(distances.back() + std::max(step, text_width));
    return distances;
}

// The inner brackets that fit before the atom stay there; the outer ones are left unset, stand at STEP from their own
// bounding box and are moved past the atom by SGroupBracketLayout. If not even the innermost fits, it stands in the
// middle of the gap
std::vector<std::optional<float>> BracketSpacing::_beforeLoneAtom(const Gap& gap) const
{
    const size_t count = gap.brackets.size();
    for (int inner_count = static_cast<int>(count) - 1; inner_count > 0; inner_count--)
    {
        Gap inner = gap;
        inner.brackets.resize(static_cast<size_t>(inner_count));
        if (_fits(inner))
            return std::vector<std::optional<float>>(count);
        if (const std::optional<std::vector<float>> reduced = _atReducedStep(inner))
        {
            std::vector<std::optional<float>> distances(reduced->begin(), reduced->end());
            distances.resize(count);
            return distances;
        }
    }
    std::vector<std::optional<float>> distances(count);
    distances[0] = gap.length / 2;
    return distances;
}

std::vector<std::optional<float>> BracketSpacing::_betweenGroups(const Gap& gap) const
{
    const std::vector<float> text_widths = _textWidths(gap);
    const float step = std::min(STEP * _bondLength, gap.length / static_cast<float>(gap.brackets.size()));
    std::vector<float> intervals;
    for (float text_width : text_widths)
        intervals.push_back(std::max(step, text_width));
    // When the texts do not fit either, the brackets share the gap evenly and the texts overlap
    if (std::accumulate(intervals.begin(), intervals.end(), 0.f) > gap.length)
        intervals.assign(text_widths.size(), step);
    const std::vector<float> distances = _centered(intervals, gap.length);
    return std::vector<std::optional<float>>(distances.begin(), distances.end());
}

// Bisection for the largest step whose intervals fit in the length: their sum grows with the step
float BracketSpacing::_reducedStep(float length, const std::vector<float>& text_widths) const
{
    const auto total = [&text_widths](float step) {
        float sum = 2 * step;
        for (float text_width : text_widths)
            sum += std::max(step, text_width);
        return sum;
    };
    float low = 0, high = STEP * _bondLength;
    for (int i = 0; i < BISECTION_ROUNDS; i++)
    {
        const float middle = (low + high) / 2;
        if (total(middle) <= length)
            low = middle;
        else
            high = middle;
    }
    return low;
}

// The texts stand to the right of their bracket: an interval holds those of the bracket on its left
std::vector<float> BracketSpacing::_textWidths(const Gap& gap) const
{
    const float clearance = TEXT_CLEARANCE * _bondLength;
    std::vector<float> widths;
    for (size_t k = 0; k + 1 < gap.brackets.size(); k++)
    {
        const Gap::Bracket& left = gap.rightwards ? gap.brackets[k] : gap.brackets[k + 1];
        widths.push_back(left.is_closing ? _texts.widthPastBracket(left.sgroup_idx) + clearance : 0.f);
    }
    return widths;
}

// The brackets around the group's own atom come first and are all opening or all closing
size_t BracketSpacing::_nearCount(const Gap& gap)
{
    size_t count = 0;
    while (count < gap.brackets.size() && gap.brackets[count].is_closing == gap.brackets[0].is_closing)
        count++;
    return count;
}

std::vector<float> BracketSpacing::_centered(const std::vector<float>& intervals, float length)
{
    std::vector<float> distances{(length - std::accumulate(intervals.begin(), intervals.end(), 0.f)) / 2};
    for (float interval : intervals)
        distances.push_back(distances.back() + interval);
    return distances;
}
