/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Anna Bauer
//                  Thomas Oberbichler
//                  Tobias Teschemacher
*/

#if !defined(KRATOS_IGA_EDGE_CABLE_ELEMENT_H_INCLUDED)
#define KRATOS_IGA_EDGE_CABLE_ELEMENT_H_INCLUDED

// System includes
//#include "includes/define.h"
//#include "includes/element.h"

// External includes

// Project includes
#include "iga_application_variables.h"

#include "iga_base_element.h"

#include "custom_utilities/geometry_utilities/iga_curve_on_surface_utilities.h"

namespace Kratos
{

class IgaEdgeCableElement
    : public IgaBaseElement<3>
{
public:
    KRATOS_CLASS_POINTER_DEFINITION( IgaEdgeCableElement );

    using IgaBaseElementType::IgaBaseElementType;

    ~IgaEdgeCableElement() override
    {
    };

    Element::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties) const override;

    /**
	* @brief Creates a new element
	* @param NewId The Id of the new created element
	* @param pGeom The pointer to the geometry of the element
	* @param pProperties The pointer to property
	* @return The pointer to the created element
	*/
	Element::Pointer Create(
		IndexType NewId,
		GeometryType::Pointer pGeom,
		PropertiesType::Pointer pProperties
	) const override
	{
		return Kratos::make_shared<IgaEdgeCableElement>(
			NewId, pGeom, pProperties);
	};

    void GetDofList(
        DofsVectorType& rElementalDofList,
        ProcessInfo& rCurrentProcessInfo) override;

    void EquationIdVector(
        EquationIdVectorType& rResult,
        ProcessInfo& rCurrentProcessInfo) override;

    void Initialize() override;

    void CalculateAll(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo,
        const bool ComputeLeftHandSide,
        const bool ComputeRightHandSide) override;

    void PrintInfo(std::ostream& rOStream) const override;

private:
    Vector3 mReferenceBaseVector;

    //Vector3 GetReferenceBaseVector() const;

    Vector3 GetActualBaseVector() const;
};

} // namespace Kratos

#endif // !defined(KRATOS_IGA_EDGE_CABLE_ELEMENT_H_INCLUDED)
