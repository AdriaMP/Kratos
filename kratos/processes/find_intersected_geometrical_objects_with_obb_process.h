//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                     Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//  Collaborators:   Ruben Zorrilla Martinez
//                   Vicente Mataix Ferrandiz
//

#if !defined(KRATOS_FIND_INTERSECTED_GEOMETRICAL_OBJECTS_WITH_OBB_PROCESS_H_INCLUDED )
#define  KRATOS_FIND_INTERSECTED_GEOMETRICAL_OBJECTS_WITH_OBB_PROCESS_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/oriented_bounding_box.h"
#include "processes/find_intersected_geometrical_objects_process.h"

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

/**
 * @class FindIntersectedGeometricalObjectsWithOBBProcess
 * @ingroup KratosCore
 * @brief This class takes two modelparts and marks the intersected ones with SELECTED flag. Does the check considering an OBB for the intersection
 * @details It creates a spatial datastructure and search for interaction. It also provides some helper methods for derived classes to check individual element or condition interesections.
 * @tparam TIntersectedEntity The type of geometrical (intersected) entity considered (if conditions or elements)
 * @tparam TIntersectingEntity The type of geometrical (intersecting) entity considered (if conditions or elements)
 * @todo Add possibility to use conditions with elements and vice versa (add second template argument)
 * @author Vicente Mataix Ferrandiz
*/
template<class TIntersectedEntity = Element, class TIntersectingEntity = TIntersectedEntity>
class KRATOS_API(KRATOS_CORE) FindIntersectedGeometricalObjectsWithOBBProcess
    : public FindIntersectedGeometricalObjectsProcess<TIntersectedEntity, TIntersectingEntity>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of FindIntersectedGeometricalObjectsWithOBBProcess
    KRATOS_CLASS_POINTER_DEFINITION(FindIntersectedGeometricalObjectsWithOBBProcess);

    /// Definition of the index type
    typedef std::size_t IndexType;

    /// Definition of the size type
    typedef std::size_t SizeType;

    /// Definition of the point type
    typedef Point PointType;

    /// Definition of the base type
    typedef FindIntersectedGeometricalObjectsProcess<TIntersectedEntity, TIntersectingEntity> BaseType;

    /// Octree type definition
    typedef typename BaseType::OctreeType OctreeType;

    /// Definition of the node type
    using NodeType = Node<3>;

    /// Definition of the geometry type
    using GeometryType = Geometry<NodeType>;

    /// Definition of the entity container type
    typedef PointerVectorSet<TIntersectingEntity, IndexedObject> IntersectingEntityContainerType;
    typedef PointerVectorSet<TIntersectedEntity, IndexedObject> IntersectedEntityContainerType;

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * @brief Default constructor.
     * @details Removed
     */
    FindIntersectedGeometricalObjectsWithOBBProcess() = delete;

    /**
     * @brief Constructor to be used.
     * @param rModelPartIntersected First model part (the one to compute the intersection)
     * @param rModelPartIntersecting Second model part (the "skin" model part)
     */
    FindIntersectedGeometricalObjectsWithOBBProcess(
        ModelPart& rModelPartIntersected,
        ModelPart& rModelPartIntersecting,
        const double BoundingBoxFactor = -1.0,
        const bool DebugOBB = false,
        OBBHasIntersectionType IntersectionType = OBBHasIntersectionType::SeparatingAxisTheorem
        );

    /**
     * @brief Constructor to be used. (with model and Parameters)
     * @param rModel The model containing all model parts
     * @param ThisParameters The configuration parameters
     */
    FindIntersectedGeometricalObjectsWithOBBProcess(
        Model& rModel,
        Parameters ThisParameters
        );

    /// Copy constructor.
    FindIntersectedGeometricalObjectsWithOBBProcess(FindIntersectedGeometricalObjectsWithOBBProcess const& rOther) = delete;

    /// Destructor.
    ~FindIntersectedGeometricalObjectsWithOBBProcess() override {}

    ///@name Member Variables
    ///@{

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override {
        return "FindIntersectedGeometricalObjectsWithOBBProcess";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override  {

    }

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    double mBoundingBoxFactor = -1.0;         /// The factor to be consider when computing the bounding box (if negative not considered)
    bool mDebugOBB = false;                   /// If we debug the boxes
    OBBHasIntersectionType mIntersectionType; /// Intersection type
    Parameters mThisParameters;               /// The configuration parameters

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    /**
     * @brief This method sets the Octree bounding box
     */
    void SetOctreeBoundingBox() override;

    /**
     * @brief This method check if there is an intersection between two geometries
     * @param rFirstGeometry The first geometry
     * @param rSecondGeometry The second geometry
     */
    bool HasIntersection(
        GeometryType& rFirstGeometry,
        GeometryType& rSecondGeometry
        ) override;

    /**
     * @brief This method check if there is an intersection between two geometries in 2D
     * @param rFirstGeometry The first geometry
     * @param rSecondGeometry The second geometry
     */
    bool HasIntersection2D(
        GeometryType& rFirstGeometry,
        GeometryType& rSecondGeometry
        ) override;

    /**
     * @brief This method check if there is an intersection between two geometries in 3D
     * @param rFirstGeometry The first geometry
     * @param rSecondGeometry The second geometry
     */
    bool HasIntersection3D(
        GeometryType& rFirstGeometry,
        GeometryType& rSecondGeometry
        ) override;

    /**
     * @brief This creates auxiliar elements with the provided OBB (2D)
     * @param rModelPart The model part where to add the elements
     * @param pProperties Pointer to the considered properties
     * @param rOrientedBoundingBox The bounding box to be postprocessed
     */
    void CreateDebugOBB2D(
        ModelPart& rModelPart,
        Properties::Pointer pProperties,
        OrientedBoundingBox<2>& rOrientedBoundingBox
        );

    /**
     * @brief This creates auxiliar elements with the provided OBB (3D)
     * @param rModelPart The model part where to add the elements
     * @param pProperties Pointer to the considered properties
     * @param rOrientedBoundingBox The bounding box to be postprocessed
     */
    void CreateDebugOBB3D(
        ModelPart& rModelPart,
        Properties::Pointer pProperties,
        OrientedBoundingBox<3>& rOrientedBoundingBox
        );

    /**
     * @brief This converts the interpolation string to an enum
     * @param Str The string that you want to comvert in the equivalent enum
     * @return OBBHasIntersectionType: The equivalent enum (this requires less memmory than a std::string)
     */
    OBBHasIntersectionType ConvertInter(const std::string& Str)
    {
        if(Str == "Direct" || Str == "direct")
            return OBBHasIntersectionType::Direct;
        else if(Str == "SeparatingAxisTheorem" || Str == "separating_axis_theorem")
            return OBBHasIntersectionType::SeparatingAxisTheorem;
        else
            return OBBHasIntersectionType::SeparatingAxisTheorem;
    }

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


    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief This method provides the defaults parameters to avoid conflicts between the different constructors
     */
    Parameters GetDefaultParameters();

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    FindIntersectedGeometricalObjectsWithOBBProcess& operator=(FindIntersectedGeometricalObjectsWithOBBProcess const& rOther);


    ///@}

}; // Class FindIntersectedGeometricalObjectsWithOBBProcess

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
template<class TIntersectedEntity = Element, class TIntersectingEntity = TIntersectedEntity>
inline std::istream& operator >> (std::istream& rIStream,
                FindIntersectedGeometricalObjectsWithOBBProcess<TIntersectedEntity>& rThis);

/// output stream function
template<class TIntersectedEntity = Element, class TIntersectingEntity = TIntersectedEntity>
inline std::ostream& operator << (std::ostream& rOStream,
                const FindIntersectedGeometricalObjectsWithOBBProcess<TIntersectedEntity>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_FIND_INTERSECTED_GEOMETRICAL_OBJECTS_WITH_OBB_PROCESS_H_INCLUDED  defined
