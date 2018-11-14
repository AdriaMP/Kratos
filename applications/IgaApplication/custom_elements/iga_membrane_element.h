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

#if !defined(KRATOS_IGA_MEMBRANE_ELEMENT_H_INCLUDED)
#define KRATOS_IGA_MEMBRANE_ELEMENT_H_INCLUDED

// System includes
#include "includes/define.h"
#include "includes/element.h"

// External includes

// Project includes
#include "iga_base_element.h"


namespace Kratos
{

class IgaMembraneElement
    : public IgaBaseElement<3>
{
public:
    KRATOS_CLASS_POINTER_DEFINITION( IgaMembraneElement );

    using IgaBaseElementType::IgaBaseElementType;

    ~IgaMembraneElement() override
    {
    };

    Element::Pointer Create(
        IndexType NewId,
        NodesArrayType const& ThisNodes,
        PropertiesType::Pointer pProperties) const override;

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
};

} // namespace Kratos

#endif // !defined(KRATOS_IGA_MEMBRANE_ELEMENT_H_INCLUDED)
