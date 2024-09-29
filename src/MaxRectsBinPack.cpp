#include "MaxRectsBinPack.hpp"

namespace binpacking
{
bool is_contained_in(const QRect& a, const QRect& b)
{
    return a.x() >= b.x() && a.y() >= b.y() && a.x() + a.width() <= b.x() + b.width() &&
           a.y() + a.height() <= b.y() + b.height();
}

bool DisjointRectCollection::add(const QRect& r)
{
    // Degenerate rectangles are ignored.
    if (r.width() == 0 || r.height() == 0)
    {
        return true;
    }

    if (!disjoint(r))
    {
        return false;
    }

    rects.push_back(r);
    return true;
}

void DisjointRectCollection::clear()
{
    rects.clear();
}

bool DisjointRectCollection::disjoint(const QRect& r) const
{
    if (r.width() == 0 || r.height() == 0)
        return true;

    for (qsizetype i = 0; i < rects.size(); ++i)
        if (!disjoint(rects[i], r))
            return false;

    return true;
}

bool DisjointRectCollection::disjoint(const QRect& a, const QRect& b)
{
    return a.x() + a.width() <= b.x() || b.x() + b.width() <= a.x() ||
           a.y() + a.height() <= b.y() || b.y() + b.height() <= a.y();
}

MaxRectsBinPack::MaxRectsBinPack(QSize size)
    : m_bin_size(size)
{
    m_free_rectangles.push_back(QRect{QPoint{}, size});
}

QRect MaxRectsBinPack::insert(QSize size, FreeRectChoiceHeuristic method)
{
    QRect newNode{};

    // Unused in this function. We don't need to know the score after finding the
    // position.
    auto score1 = std::numeric_limits<int>::max();
    auto score2 = std::numeric_limits<int>::max();

    switch (method)
    {
        case FreeRectChoiceHeuristic::RectBestShortSideFit:
            newNode = find_position_for_new_node_best_short_side_fit(size, score1, score2);
            break;
        case FreeRectChoiceHeuristic::RectBottomLeftRule:
            newNode = find_position_for_new_node_bottom_left(size, score1, score2);
            break;
        case FreeRectChoiceHeuristic::RectContactPointRule:
            newNode = find_position_for_new_node_contact_point(size, score1);
            break;
        case FreeRectChoiceHeuristic::RectBestLongSideFit:
            newNode = find_position_for_new_node_best_long_side_fit(size, score2, score1);
            break;
        case FreeRectChoiceHeuristic::RectBestAreaFit:
            newNode = find_position_for_new_node_best_area_fit(size, score1, score2);
            break;
    }

    if (newNode.height() == 0)
    {
        return newNode;
    }

    auto numRectanglesToProcess = m_free_rectangles.size();

    for (qsizetype i = 0; i < numRectanglesToProcess; ++i)
    {
        if (split_free_node(m_free_rectangles[i], newNode))
        {
            m_free_rectangles.erase(m_free_rectangles.constBegin() + i);
            --i;
            --numRectanglesToProcess;
        }
    }

    prune_free_list();
    m_used_rectangles.push_back(newNode);

    return newNode;
}

void MaxRectsBinPack::place_rect(const QRect& node)
{
    qsizetype numRectanglesToProcess = m_free_rectangles.size();

    for (qsizetype i = 0; i < numRectanglesToProcess; ++i)
    {
        if (split_free_node(m_free_rectangles[i], node))
        {
            m_free_rectangles.erase(m_free_rectangles.constBegin() + i);
            --i;
            --numRectanglesToProcess;
        }
    }

    prune_free_list();
    m_used_rectangles.push_back(node);
}

QRect MaxRectsBinPack::score_rect(QSize                   size,
                                  FreeRectChoiceHeuristic method,
                                  int&                    score1,
                                  int&                    score2) const
{
    QRect newNode{};
    score1 = std::numeric_limits<int>::max();
    score2 = std::numeric_limits<int>::max();

    switch (method)
    {
        case FreeRectChoiceHeuristic::RectBestShortSideFit:
            newNode = find_position_for_new_node_best_short_side_fit(size, score1, score2);
            break;
        case FreeRectChoiceHeuristic::RectBottomLeftRule:
            newNode = find_position_for_new_node_bottom_left(size, score1, score2);
            break;
        case FreeRectChoiceHeuristic::RectContactPointRule:
            newNode = find_position_for_new_node_contact_point(size, score1);
            score1  = -score1; // Reverse since we are minimizing, but for contact point
                              // score bigger is better.
            break;
        case FreeRectChoiceHeuristic::RectBestLongSideFit:
            newNode = find_position_for_new_node_best_long_side_fit(size, score2, score1);
            break;
        case FreeRectChoiceHeuristic::RectBestAreaFit:
            newNode = find_position_for_new_node_best_area_fit(size, score1, score2);
            break;
    }

    // Cannot fit the current rectangle.
    if (newNode.height() == 0)
    {
        score1 = std::numeric_limits<int>::max();
        score2 = std::numeric_limits<int>::max();
    }

    return newNode;
}

float MaxRectsBinPack::occupancy() const
{
    auto usedSurfaceArea = 0UL;

    for (const auto& usedRectangle : m_used_rectangles)
    {
        usedSurfaceArea += usedRectangle.width() * usedRectangle.height();
    }

    return float(usedSurfaceArea) / (m_bin_size.width() * m_bin_size.height());
}

QRect MaxRectsBinPack::find_position_for_new_node_bottom_left(QSize size,
                                                              int&  bestY,
                                                              int&  bestX) const
{
    QRect bestNode{};

    bestY = std::numeric_limits<int>::max();
    bestX = std::numeric_limits<int>::max();

    for (const auto& freeRectangle : m_free_rectangles)
    {
        if (freeRectangle.width() >= size.width() && freeRectangle.height() >= size.height())
        {
            const auto topSideY = freeRectangle.y() + size.height();
            if (topSideY < bestY || (topSideY == bestY && freeRectangle.x() < bestX))
            {
                bestNode = QRect{freeRectangle.x(), freeRectangle.y(), size.width(), size.height()};
                bestY    = topSideY;
                bestX    = freeRectangle.x();
            }
        }
    }

    return bestNode;
}

QRect MaxRectsBinPack::find_position_for_new_node_best_short_side_fit(QSize size,
                                                                      int&  bestShortSideFit,
                                                                      int&  bestLongSideFit) const
{
    QRect bestNode{};

    bestShortSideFit = std::numeric_limits<int>::max();
    bestLongSideFit  = std::numeric_limits<int>::max();

    for (const auto& freeRectangle : m_free_rectangles)
    {
        // Try to place the rectangle in upright (non-flipped) orientation.
        if (freeRectangle.width() >= size.width() && freeRectangle.height() >= size.height())
        {
            const auto leftoverHoriz = std::abs(freeRectangle.width() - size.width());
            const auto leftoverVert  = std::abs(freeRectangle.height() - size.height());
            const auto shortSideFit  = std::min(leftoverHoriz, leftoverVert);
            const auto longSideFit   = std::max(leftoverHoriz, leftoverVert);

            if (shortSideFit < bestShortSideFit ||
                (shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit))
            {
                bestNode = QRect{freeRectangle.x(), freeRectangle.y(), size.width(), size.height()};
                bestShortSideFit = shortSideFit;
                bestLongSideFit  = longSideFit;
            }
        }
    }

    return bestNode;
}

QRect MaxRectsBinPack::find_position_for_new_node_best_long_side_fit(QSize size,
                                                                     int&  bestShortSideFit,
                                                                     int&  bestLongSideFit) const
{
    QRect bestNode{};

    bestShortSideFit = std::numeric_limits<int>::max();
    bestLongSideFit  = std::numeric_limits<int>::max();

    for (const auto& freeRectangle : m_free_rectangles)
    {
        // Try to place the rectangle in upright (non-flipped) orientation.
        if (freeRectangle.width() >= size.width() && freeRectangle.height() >= size.height())
        {
            const auto leftoverHoriz = std::abs(freeRectangle.width() - size.width());
            const auto leftoverVert  = std::abs(freeRectangle.height() - size.height());
            const auto shortSideFit  = std::min(leftoverHoriz, leftoverVert);
            const auto longSideFit   = std::max(leftoverHoriz, leftoverVert);

            if (longSideFit < bestLongSideFit ||
                (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
            {
                bestNode = QRect{freeRectangle.x(), freeRectangle.y(), size.width(), size.height()};
                bestShortSideFit = shortSideFit;
                bestLongSideFit  = longSideFit;
            }
        }
    }

    return bestNode;
}

QRect MaxRectsBinPack::find_position_for_new_node_best_area_fit(QSize size,
                                                                int&  bestAreaFit,
                                                                int&  bestShortSideFit) const
{
    QRect bestNode{};

    bestAreaFit      = std::numeric_limits<int>::max();
    bestShortSideFit = std::numeric_limits<int>::max();

    for (const auto& freeRectangle : m_free_rectangles)
    {
        const auto areaFit =
            freeRectangle.width() * freeRectangle.height() - size.width() * size.height();

        // Try to place the rectangle in upright (non-flipped) orientation.
        if (freeRectangle.width() >= size.width() && freeRectangle.height() >= size.height())
        {
            const auto leftoverHoriz = std::abs(freeRectangle.width() - size.width());
            const auto leftoverVert  = std::abs(freeRectangle.height() - size.height());
            const auto shortSideFit  = std::min(leftoverHoriz, leftoverVert);

            if (areaFit < bestAreaFit ||
                (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
            {
                bestNode = QRect{freeRectangle.x(), freeRectangle.y(), size.width(), size.height()};
                bestShortSideFit = shortSideFit;
                bestAreaFit      = areaFit;
            }
        }
    }
    return bestNode;
}

int CommonIntervalLength(int i1start, int i1end, int i2start, int i2end)
{
    if (i1end < i2start || i2end < i1start)
    {
        return 0;
    }

    return std::min(i1end, i2end) - std::max(i1start, i2start);
}

int MaxRectsBinPack::contact_point_score_node(int x, int y, QSize size) const
{
    auto score = 0;

    if (x == 0 || x + size.width() == m_bin_size.width())
    {
        score += size.height();
    }

    if (y == 0 || y + size.height() == m_bin_size.height())
    {
        score += size.width();
    }

    for (const auto& usedRectangle : m_used_rectangles)
    {
        if (usedRectangle.x() == x + size.width() || usedRectangle.x() + usedRectangle.width() == x)
        {
            score += CommonIntervalLength(usedRectangle.y(),
                                          usedRectangle.y() + usedRectangle.height(),
                                          y,
                                          y + size.height());
        }

        if (usedRectangle.y() == y + size.height() ||
            usedRectangle.y() + usedRectangle.height() == y)
        {
            score += CommonIntervalLength(usedRectangle.x(),
                                          usedRectangle.x() + usedRectangle.width(),
                                          x,
                                          x + size.width());
        }
    }

    return score;
}

QRect MaxRectsBinPack::find_position_for_new_node_contact_point(QSize size,
                                                                int&  bestContactScore) const
{
    QRect bestNode{};
    bestContactScore = -1;

    for (const auto& freeRectangle : m_free_rectangles)
    {
        // Try to place the rectangle in upright (non-flipped) orientation.
        if (freeRectangle.width() >= size.width() && freeRectangle.height() >= size.height())
        {
            const auto score = contact_point_score_node(freeRectangle.x(), freeRectangle.y(), size);
            if (score > bestContactScore)
            {
                bestNode = QRect{freeRectangle.x(), freeRectangle.y(), size.width(), size.height()};
                bestContactScore = score;
            }
        }
    }

    return bestNode;
}

bool MaxRectsBinPack::split_free_node(QRect freeNode, const QRect& usedNode)
{
    // Test with SAT if the rectangles even intersect.
    if (usedNode.x() >= freeNode.x() + freeNode.width() ||
        usedNode.x() + usedNode.width() <= freeNode.x() ||
        usedNode.y() >= freeNode.y() + freeNode.height() ||
        usedNode.y() + usedNode.height() <= freeNode.y())
        return false;

    if (usedNode.x() < freeNode.x() + freeNode.width() &&
        usedNode.x() + usedNode.width() > freeNode.x())
    {
        // New node at the top side of the used node.
        if (usedNode.y() > freeNode.y() && usedNode.y() < freeNode.y() + freeNode.height())
        {
            auto newNode = freeNode;
            newNode.setHeight(usedNode.y() - newNode.y());
            m_free_rectangles.push_back(newNode);
        }

        // New node at the bottom side of the used node.
        if (usedNode.y() + usedNode.height() < freeNode.y() + freeNode.height())
        {
            auto newNode = freeNode;
            newNode.setY(usedNode.y() + usedNode.height());
            newNode.setHeight(freeNode.y() + freeNode.height() -
                              (usedNode.y() + usedNode.height()));
            m_free_rectangles.push_back(newNode);
        }
    }

    if (usedNode.y() < freeNode.y() + freeNode.height() &&
        usedNode.y() + usedNode.height() > freeNode.y())
    {
        // New node at the left side of the used node.
        if (usedNode.x() > freeNode.x() && usedNode.x() < freeNode.x() + freeNode.width())
        {
            auto newNode = freeNode;
            newNode.setWidth(usedNode.x() - newNode.x());
            m_free_rectangles.push_back(newNode);
        }

        // New node at the right side of the used node.
        if (usedNode.x() + usedNode.width() < freeNode.x() + freeNode.width())
        {
            auto newNode = freeNode;
            newNode.setX(usedNode.x() + usedNode.width());
            newNode.setWidth(freeNode.x() + freeNode.width() - (usedNode.x() + usedNode.width()));
            m_free_rectangles.push_back(newNode);
        }
    }

    return true;
}

void MaxRectsBinPack::prune_free_list()
{
    /// Go through each pair and remove any rectangle that is redundant.
    for (qsizetype i = 0; i < m_free_rectangles.size(); ++i)
        for (qsizetype j = i + 1; j < m_free_rectangles.size(); ++j)
        {
            if (is_contained_in(m_free_rectangles[i], m_free_rectangles[j]))
            {
                m_free_rectangles.erase(m_free_rectangles.constBegin() + i);
                --i;
                break;
            }
            if (is_contained_in(m_free_rectangles[j], m_free_rectangles[i]))
            {
                m_free_rectangles.erase(m_free_rectangles.constBegin() + j);
                --j;
            }
        }
}
} // namespace binpacking
