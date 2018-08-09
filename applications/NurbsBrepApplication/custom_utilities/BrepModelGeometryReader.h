#if !defined(KRATOS_BREP_MODEL_GEOMETRY_READER_H_INCLUDED )
#define  KRATOS_BREP_MODEL_GEOMETRY_READER_H_INCLUDED



// ------------------------------------------------------------------------------
// System includes
// ------------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <math.h>

// ------------------------------------------------------------------------------
// External includes
// ------------------------------------------------------------------------------
// #include <boost/python.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

// ------------------------------------------------------------------------------
// Project includes
// ------------------------------------------------------------------------------
#include "brep/BrepModel.h"
#include "brep/BrepTrimmingCurve.h"
#include "brep/BrepBoundaryLoop.h"
#include "brep/BrepFace.h"
#include "brep/BrepEdge.h"

#include "includes/kratos_parameters.h"
#include "includes/model_part.h"
#include "includes/node.h"

//#include "nurbs_utilities.h"

namespace Kratos
{

  ///@name Kratos Globals
  ///@{
  ///@}
  ///@name Type Definitions
  ///@{
  ///@}
  ///@name  Enum's
  ///@{
  ///@}
  ///@name  Functions
  ///@{
  ///@}
  ///@name Kratos Classes
  ///@{
  /// Short class definition.
  /** Detail class definition.
  */
    class BrepModelGeometryReader
  {
  public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of KratosNurbsTestcaseApplication
    KRATOS_CLASS_POINTER_DEFINITION(BrepModelGeometryReader);

    ///@}
    ///@name Life Cycle
    ///@{
	//Parameters WriteGeometry(ModelPart& model_part);
    std::vector<BrepModel> ReadGeometry(ModelPart& model_part);
	double ReadModelTolerance();

    void WriteGaussPoints(ModelPart& model_part);
	void WriteGaussPointsJson(ModelPart& rModelPart, const std::string& rOutputFileName);
    void WriteGaussPointsIteration(ModelPart& model_part, int step);

    /// Constructor.
    BrepModelGeometryReader(Parameters& cad_geometry_in_json);

    /// Destructor.
    virtual ~BrepModelGeometryReader();

  protected:
    ///@name Protected static Member Variables
    ///@{
    ///@}
    ///@name Protected member Variables
    ///@{
    ///@}
    ///@name Protected Operators
    ///@{
    ///@}
    ///@name Protected Operations
    ///@{
    ///@}
    ///@name Protected  Access
    ///@{
    ///@}
    ///@name Protected Inquiry
    ///@{
    ///@}
    ///@name Protected LifeCycle
    ///@{
    ///@}
  private:
    ///@name Static Member Variables
    ///@{
    //       static const ApplicationCondition  msApplicationCondition;
    ///@}
        ///@name Member Variables
    ///@{

    Parameters m_cad_geometry_in_json;

    ///@}
    ///@name Private Operators
    ///@{
    ///@}
    ///@name Private Operations
    ///@{
    ///@}
    ///@name Private  Access
    ///@{
    ///@}
    ///@name Private Inquiry
    ///@{
    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    BrepModelGeometryReader& operator=(BrepModelGeometryReader const& rOther);

    /// Copy constructor.
    BrepModelGeometryReader(BrepModelGeometryReader const& rOther);
    ///@}

  }; // Class BrepModelGeometryReader
  ///@}
  ///@name Type Definitions
  ///@{
  ///@}
  ///@name Input and output
  ///@{
  ///@}


}  // namespace Kratos.

#endif // KRATOS_BREP_MODEL_GEOMETRY_READER_H_INCLUDED  defined