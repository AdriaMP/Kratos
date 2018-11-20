//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:     BSD License
//           Kratos default license: kratos/IGAStructuralMechanicsApplication/license.txt
//
//  Main authors:    Tobias Teschemacher
//                   Michael Breitenberger
//

// System includes

// External includes 

// Project includes
#include "brep_boundary_loop.h"


namespace Kratos
{
    std::vector<BrepTrimmingCurve>& BrepBoundaryLoop::GetTrimmingCurves()
    {
        return m_brep_trimming_curves;
    }

    //BrepTrimmingCurve& BrepBoundaryLoop::GetTrimmingCurve(const int& trim_index)
    //{
    //    for (int i = 0; i < m_brep_trimming_curves.size(); ++i)
    //    {
    //        if (m_brep_trimming_curves[i].GetTrimIndex() == trim_index)
    //            return m_brep_trimming_curves[i];
    //    }
    //}

    bool& BrepBoundaryLoop::IsOuterLoop()
    {
        return m_is_outer_loop;
    }

    /// Constructor.
    BrepBoundaryLoop::BrepBoundaryLoop(
        std::vector<BrepTrimmingCurve>& rBrepTrimmingCurves,
        bool rIsOuterLoop)
        : m_brep_trimming_curves(rBrepTrimmingCurves),
          m_is_outer_loop(rIsOuterLoop)
    {
    }
}  // namespace Kratos.

