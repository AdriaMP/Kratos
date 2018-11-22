#if !defined(KRATOS_BREP_MODEL_H_INCLUDED )
#define  KRATOS_BREP_MODEL_H_INCLUDED

// System includes

// Project includes
#include "iga_application_variables.h"

#include "brep_face.h"
#include "brep_edge.h"
#include "brep_vertex.h"

#include <ANurbs/Integration>
namespace Kratos
{
    ///@name Kratos Classes
    ///@{
    /// Holder of all Brep entities.
    /** This class contains the full Brep description needed to describe all CAD-geometries.
    */
    class BrepModel : public IndexedObject, public Flags
    {
    public:
        ///@name Type Definitions
        ///@{
        //KRATOS_CLASS_POINTER_DEFINITION(BrepModel);

        ///@}
        ///@name Life Cycle
        ///@{

        bool GetIntegrationDomainGeometry(
            ModelPart& rModelPart,
            int& brep_id,
            const std::string& rType,
            const std::string& rName,
            const int& rPropertiesId,
            const int& rShapeFunctionDerivativesOrder,
            std::vector<std::string> rVariables);

        bool GetIntegrationDomainBrep(
            ModelPart& rModelPart,
            int& brep_id,
            const std::string& rType,
            const std::string& rName,
            const int& rPropertiesId,
            const int& rShapeFunctionDerivativesOrder,
            std::vector<std::string> rVariables);

        bool GetIntegrationDomainBrepCoupling(
            ModelPart& rModelPart,
            int& brep_id,
            const std::string& rType,
            const std::string& rName,
            const int& rPropertiesId,
            const int& rShapeFunctionDerivativesOrder,
            std::vector<std::string> rVariables);

        bool GetIntegrationDomainBrepCoupling(
            ModelPart& rModelPart,
            const std::string& rType,
            const std::string& rName,
            const int& rPropertiesId,
            const int& rShapeFunctionDerivativesOrder,
            std::vector<std::string> rVariables);

        void GetIntegrationBrepCouplingEdge(
            const BrepEdge::EdgeTopology& master,
            const BrepEdge::EdgeTopology& slave,
            ModelPart& rModelPart,
            const std::string& rType,
            const std::string& rName,
            const int& rPropertiesId,
            const int& rShapeFunctionDerivativesOrder,
            std::vector<std::string> rVariables) const;

        //BrepFace& GetFaceNonConst(const int& brep_id);

        const BrepFace& GetFace(const int& brep_id) const;

        const std::vector<BrepFace>&   GetFaceVector() const;
        const std::vector<BrepEdge>&   GetEdgeVector() const;
        const std::vector<BrepVertex>& GetVertexVector() const;

        /// Constructor
        BrepModel(
            int& brep_id,
            double& model_tolerance,
            std::vector<BrepFace>& faces,
            std::vector<BrepEdge>& edges,
            std::vector<BrepVertex>& vertices);

        /// Destructor.
        virtual ~BrepModel()
        {};

        ///@}
    protected:

    private:
        ///@name Member Variables
        ///@{

        double                  m_model_tolerance;
        std::vector<BrepFace>   m_brep_faces;
        std::vector<BrepEdge>   m_brep_edges;
        std::vector<BrepVertex> m_brep_vertices;

        ///@}
    }; // Class BrepModel

}  // namespace Kratos.

#endif // KRATOS_BREP_MODEL_H_INCLUDED  defined