/**
  @author Jukka Jylänki

  @brief Implements different bin packer algorithms that use the MAXRECTS data structure.
  This work is released to Public Domain, do whatever you want with it.

  EDIT: modified for use with this project.
*/

#pragma once

#include <QList>
#include <QRect>
#include <QSize>

namespace binpacking
{
class DisjointRectCollection
{
  public:
    bool add(const QRect& r);

    void clear();

    bool disjoint(const QRect& r) const;

    static bool disjoint(const QRect& a, const QRect& b);

    QList<QRect> rects;
};

class MaxRectsBinPack
{
  public:
    explicit MaxRectsBinPack(QSize size);

    enum class FreeRectChoiceHeuristic
    {
        RectBestShortSideFit,
        RectBestLongSideFit,
        RectBestAreaFit,
        RectBottomLeftRule,
        RectContactPointRule
    };

    QRect insert(QSize size, FreeRectChoiceHeuristic method);

    float occupancy() const;

  private:
    QRect score_rect(QSize size, FreeRectChoiceHeuristic method, int& score1, int& score2) const;

    void place_rect(const QRect& node);

    int contact_point_score_node(int x, int y, QSize size) const;

    QRect find_position_for_new_node_bottom_left(QSize size, int& bestY, int& bestX) const;

    QRect find_position_for_new_node_best_short_side_fit(QSize size,
                                                         int&  bestShortSideFit,
                                                         int&  bestLongSideFit) const;

    QRect find_position_for_new_node_best_long_side_fit(QSize size,
                                                        int&  bestShortSideFit,
                                                        int&  bestLongSideFit) const;

    QRect find_position_for_new_node_best_area_fit(QSize size,
                                                   int&  bestAreaFit,
                                                   int&  bestShortSideFit) const;

    QRect find_position_for_new_node_contact_point(QSize size, int& contactScore) const;

    bool split_free_node(QRect freeNode, const QRect& usedNode);

    void prune_free_list();

    QSize m_bin_size;

    QList<QRect> m_used_rectangles;
    QList<QRect> m_free_rectangles;
};
} // namespace binpacking