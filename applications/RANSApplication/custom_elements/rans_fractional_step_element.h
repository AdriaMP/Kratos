//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//
//  Extended by :    Suneth Warnakulasuriya

#if !defined(KRATOS_RANS_FRACTIONAL_STEP_ELEMENT_H_INCLUDED)
#define KRATOS_RANS_FRACTIONAL_STEP_ELEMENT_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_elements/fractional_step.h"

// Application includes

namespace Kratos
{
///@addtogroup RANSApplication
///@{

///@name Kratos Classes
///@{

/// A stabilized element for the incompressible Navier-Stokes equations.
/**
 */
template <unsigned int TDim>
class RansFractionalStepElement : public FractionalStep<TDim>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of RansFractionalStepElement
    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION(RansFractionalStepElement);

    /// Node type (default is: Node<3>)
    typedef Node<3> NodeType;

    /// Geometry type (using with given NodeType)
    typedef Geometry<NodeType> GeometryType;

    /// Definition of nodes container type, redefined from GeometryType
    typedef Geometry<NodeType>::PointsArrayType NodesArrayType;

    /// Vector type for local contributions to the linear system
    typedef Vector VectorType;

    /// Matrix type for local contributions to the linear system
    typedef Matrix MatrixType;

    typedef std::size_t IndexType;

    typedef std::size_t SizeType;

    typedef std::vector<std::size_t> EquationIdVectorType;

    typedef std::vector<Dof<double>::Pointer> DofsVectorType;

    typedef PointerVectorSet<Dof<double>, IndexedObject> DofsArrayType;

    /// Type for shape function values container
    typedef Kratos::Vector ShapeFunctionsType;

    /// Type for a matrix containing the shape function gradients
    typedef Kratos::Matrix ShapeFunctionDerivativesType;

    /// Type for an array of shape function gradient matrices
    typedef GeometryType::ShapeFunctionsGradientsType ShapeFunctionDerivativesArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

    // Constructors.

    /// Default constuctor.
    /**
     * @param NewId Index number of the new element (optional)
     */
    RansFractionalStepElement(IndexType NewId = 0) : FractionalStep<TDim>(NewId)
    {
    }

    /// Constructor using an array of nodes.
    /**
     * @param NewId Index of the new element
     * @param ThisNodes An array containing the nodes of the new element
     */
    RansFractionalStepElement(IndexType NewId, const NodesArrayType& ThisNodes)
        : FractionalStep<TDim>(NewId, ThisNodes)
    {
    }

    /// Constructor using a geometry object.
    /**
     * @param NewId Index of the new element
     * @param pGeometry Pointer to a geometry object
     */
    RansFractionalStepElement(IndexType NewId, GeometryType::Pointer pGeometry)
        : FractionalStep<TDim>(NewId, pGeometry)
    {
    }

    /// Constuctor using geometry and properties.
    /**
     * @param NewId Index of the new element
     * @param pGeometry Pointer to a geometry object
     * @param pProperties Pointer to the element's properties
     */
    RansFractionalStepElement(IndexType NewId,
                              GeometryType::Pointer pGeometry,
                              Element::PropertiesType::Pointer pProperties)
        : FractionalStep<TDim>(NewId, pGeometry, pProperties)
    {
    }

    /// Destructor.
    ~RansFractionalStepElement() override
    {
    }

    ///@}
    ///@name Operations
    ///@{

    /// Create a new element of this type
    /**
     * Returns a pointer to a new RansFractionalStepElement element, created using given input
     * @param NewId the ID of the new element
     * @param ThisNodes the nodes of the new element
     * @param pProperties the properties assigned to the new element
     * @return a Pointer to the new element
     */
    Element::Pointer Create(IndexType NewId,
                            NodesArrayType const& ThisNodes,
                            Element::PropertiesType::Pointer pProperties) const override
    {
        return Kratos::make_intrusive<RansFractionalStepElement<TDim>>(
            NewId, this->GetGeometry().Create(ThisNodes), pProperties);
    }

    /**
     * Returns a pointer to a new RansFractionalStepElement element, created using given input
     * @param NewId the ID of the new element
     * @param pGeom a pointer to the geometry
     * @param pProperties the properties assigned to the new element
     * @return a Pointer to the new element
     */

    Element::Pointer Create(IndexType NewId,
                            Element::GeometryType::Pointer pGeom,
                            Element::PropertiesType::Pointer pProperties) const override
    {
        return Kratos::make_intrusive<RansFractionalStepElement<TDim>>(
            NewId, pGeom, pProperties);
    }

    /**
     * Clones the selected element variables, creating a new one
     * @param NewId the ID of the new element
     * @param ThisNodes the nodes of the new element
     * @param pProperties the properties assigned to the new element
     * @return a Pointer to the new element
     */

    Element::Pointer Clone(IndexType NewId, NodesArrayType const& rThisNodes) const override
    {
        Element::Pointer pNewElement = Create(
            NewId, this->GetGeometry().Create(rThisNodes), this->pGetProperties());

        pNewElement->SetData(this->GetData());
        pNewElement->SetFlags(this->GetFlags());

        return pNewElement;
    }

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "RansFractionalStepElement #" << this->Id();
        return buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "RansFractionalStepElement" << TDim << "D";
    }

    //        /// Print object's data.
    //        virtual void PrintData(std::ostream& rOStream) const;

    ///@}

protected:
    ///@name Protected Access
    ///@{

    void CalculateLocalFractionalVelocitySystem(MatrixType& rLeftHandSideMatrix,
                                                VectorType& rRightHandSideVector,
                                                const ProcessInfo& rCurrentProcessInfo) override;

    void CalculateLocalPressureSystem(MatrixType& rLeftHandSideMatrix,
                                      VectorType& rRightHandSideVector,
                                      const ProcessInfo& rCurrentProcessInfo) override;

    ///@}

private:
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
    }

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    RansFractionalStepElement& operator=(RansFractionalStepElement const& rOther);

    /// Copy constructor.
    RansFractionalStepElement(RansFractionalStepElement const& rOther);

    ///@}

}; // Class RansFractionalStepElement

///@}
///@name Input and output
///@{

/// input stream function
template <unsigned int TDim>
inline std::istream& operator>>(std::istream& rIStream, RansFractionalStepElement<TDim>& rThis)
{
    return rIStream;
}

/// output stream function
template <unsigned int TDim>
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansFractionalStepElement<TDim>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@}

} // namespace Kratos.

#endif // KRATOS_RANS_FRACTIONAL_STEP_ELEMENT_H_INCLUDED  defined
