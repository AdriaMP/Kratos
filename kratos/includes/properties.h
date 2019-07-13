//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                     Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//                   Riccardo Rossi
//


#if !defined(KRATOS_PROPERTIES_H_INCLUDED )
#define  KRATOS_PROPERTIES_H_INCLUDED

// System includes
#include <string>
#include <iostream>
#include <cstddef>
#include <unordered_map>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/node.h"
#include "containers/data_value_container.h"
#include "includes/process_info.h"
#include "includes/table.h"

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
 * @class Properties
 * @ingroup KratosCore
 * @brief Properties encapsulates data shared by different Elements or Conditions. It can store any type of data and provides a variable base access to them.
 * @details These are all parameters that can be shared between Element. Usually material parameters are common for a set of element, so this category of data is referred as properties.
 * But in general it can be any common parameter for a group of Elements. Sharing these data as properties reduces the memory used by the application and also helps updating them if necessary.
 * As mentioned before Properties is a shared data container between Elements or Conditions. In finite element problems there are several parameters which are the same for a set of elements and conditions.
 * Thermal conductivity, elasticity of the material and viscosity of the fluid are examples of these parameters. Properties holds these data and is shared by elements or Conditions. This eliminates memory overhead due to redundant copies of these data for each element and Condition. Properties also can be used to access nodal data if it is necessary.
 * It is important to mention that accessing the nodal data via Properties is not the same as accessing it via Node. When user asks Properties for a variable data in a Node, the process starts with finding the variable in the Properties data container and if it does not exist then get it from Node.
 * This means that the priority of data is with the one stored in Properties and then in Node.
 * @author Pooyan Dadvand
 */
class Properties : public IndexedObject
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of Properties
    KRATOS_CLASS_POINTER_DEFINITION(Properties);

#ifdef  _WIN32 // work around for windows int64_t error
    typedef __int64 int64_t;
#endif
    typedef IndexedObject BaseType;

    typedef DataValueContainer ContainerType;

    typedef Node<3> NodeType;

    typedef NodeType::IndexType IndexType;

    typedef Table<double> TableType;

    typedef std::unordered_map<std::size_t, TableType> TablesContainerType; // This is a provisional implmentation and should be changed to hash. Pooyan.

    /// Properties container. A vector set of properties with their Id's as key.
    typedef PointerVectorSet<Properties, IndexedObject> PropertiesContainerType;

    /** Iterator over the properties. This iterator is an indirect
    iterator over Properties::Pointer which turn back a reference to
    properties by * operator and not a pointer for more convenient
    usage. */
    typedef typename PropertiesContainerType::iterator PropertiesIterator;

    /** Const iterator over the properties. This iterator is an indirect
    iterator over Properties::Pointer which turn back a reference to
    properties by * operator and not a pointer for more convenient
    usage. */
    typedef typename PropertiesContainerType::const_iterator PropertiesConstantIterator;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    explicit Properties(IndexType NewId = 0) : BaseType(NewId), mData(), mTables(), mSubPropertiesList() {}

    /// Default of properties with subproperties
    explicit Properties(IndexType NewId, PropertiesContainerType SubPropertiesList) : BaseType(NewId), mData(), mTables(), mSubPropertiesList(SubPropertiesList) {}

    /// Copy constructor.
    Properties(const Properties& rOther) : BaseType(rOther), mData(rOther.mData), mTables(rOther.mTables), mSubPropertiesList(rOther.mSubPropertiesList) {}

    /// Destructor.
    ~Properties() override {}


    ///@}
    ///@name Operators
    ///@{

    /// Assignment operator.
    Properties& operator=(const Properties& rOther)
    {
        BaseType::operator=(rOther);
        mData = rOther.mData;
        mTables = rOther.mTables;
        mSubPropertiesList = rOther.mSubPropertiesList;
        return *this;
    }

    template<class TVariableType>
    typename TVariableType::Type& operator()(const TVariableType& rV)
    {
        return GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type const& operator()(const TVariableType& rV) const
    {
        return GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type& operator[](const TVariableType& rV)
    {
        return GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type const& operator[](const TVariableType& rV) const
    {
        return GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type& operator()(const TVariableType& rV, NodeType& rThisNode)
    {
        return GetValue(rV, rThisNode);
    }

    template<class TVariableType>
    typename TVariableType::Type const& operator()(const TVariableType& rV, NodeType const& rThisNode) const
    {
        return GetValue(rV, rThisNode);
    }

    template<class TVariableType>
    typename TVariableType::Type& operator()(const TVariableType& rV, NodeType& rThisNode, IndexType SolutionStepIndex)
    {
        return GetValue(rV, rThisNode, SolutionStepIndex);
    }

    template<class TVariableType>
    typename TVariableType::Type const& operator()(const TVariableType& rV, NodeType const& rThisNode, IndexType SolutionStepIndex) const
    {
        return GetValue(rV, rThisNode, SolutionStepIndex);
    }

    template<class TVariableType>
    typename TVariableType::Type& operator()(const TVariableType& rV, NodeType& rThisNode, ProcessInfo const& rCurrentProcessInfo)
    {
        return GetValue(rV, rThisNode, rCurrentProcessInfo.GetSolutionStepIndex());
    }

    template<class TVariableType>
    typename TVariableType::Type const& operator()(const TVariableType& rV, NodeType const& rThisNode, ProcessInfo const& rCurrentProcessInfo) const
    {
        return GetValue(rV, rThisNode, rCurrentProcessInfo.GetSolutionStepIndex());
    }

    ///@}
    ///@name Operations
    ///@{

    template<class TVariableType>
    typename TVariableType::Type& GetValue(const TVariableType& rV)
    {
        return mData.GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type const& GetValue(const TVariableType& rV) const
    {
        return mData.GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type& GetValue(const TVariableType& rV, NodeType& rThisNode)
    {
        if(mData.Has(rV))
            return mData.GetValue(rV);

        return rThisNode.GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type const& GetValue(const TVariableType& rV, NodeType const& rThisNode) const
    {
        if(mData.Has(rV))
            return mData.GetValue(rV);

        return rThisNode.GetValue(rV);
    }

    template<class TVariableType>
    typename TVariableType::Type& GetValue(const TVariableType& rV, NodeType& rThisNode, IndexType SolutionStepIndex)
    {
        if(mData.Has(rV))
            return mData.GetValue(rV);

        return rThisNode.GetValue(rV, SolutionStepIndex);
    }

    template<class TVariableType>
    typename TVariableType::Type const& GetValue(const TVariableType& rV, NodeType const& rThisNode, IndexType SolutionStepIndex) const
    {
        if(mData.Has(rV))
            return mData.GetValue(rV);

        return rThisNode.GetValue(rV, SolutionStepIndex);
    }

    template<class TVariableType>
    void SetValue(TVariableType const& rV, typename TVariableType::Type const& rValue)
    {
        mData.SetValue(rV, rValue);
    }

    bool HasVariables() const
    {
        return !mData.IsEmpty();
    }

    template<class TXVariableType, class TYVariableType>
    TableType& GetTable(const TXVariableType& XVariable, const TYVariableType& YVariable)
    {
        return mTables[Key(XVariable.Key(), YVariable.Key())];
    }

    template<class TXVariableType, class TYVariableType>
    TableType const& GetTable(const TXVariableType& XVariable, const TYVariableType& YVariable) const
    {
        return mTables.at(Key(XVariable.Key(), YVariable.Key()));
    }

    template<class TXVariableType, class TYVariableType>
    void SetTable(const TXVariableType& XVariable, const TYVariableType& YVariable, TableType const& rThisTable)
    {
        mTables[Key(XVariable.Key(), YVariable.Key())] = rThisTable;
    }

    bool HasTables() const
    {
        return !mTables.empty();
    }

    bool IsEmpty() const
    {
        return !( HasVariables() || HasTables() );
    }

    int64_t Key(std::size_t XKey, std::size_t YKey) const
    {
        int64_t result_key = XKey;
        result_key = result_key << 32;
        result_key |= YKey; // I know that the key is less than 2^32 so I don't need zeroing the upper part
        return result_key;
    }

    /**
     * @brief This method returns the number of subproperties
     * @return The current number of subproperties
     */
    std::size_t NumberOfSubproperties()
    {
        return mSubPropertiesList.size();
    }

    /**
     * @brief This method insert a new property into the list of subproperties
     * @param pNewSubProperty The new property to be added
     */
    void AddSubProperty(Properties::Pointer pNewSubProperty)
    {
        KRATOS_ERROR_IF(HasSubPropertiesById(pNewSubProperty->Id())) << "SubProperties with Id: " << pNewSubProperty->Id() << " already exists. In each level the Id of properties (new or existing) should be unique" << std::endl;
        mSubPropertiesList.insert(mSubPropertiesList.begin(), pNewSubProperty);
    }

    /**
     * @brief This method checks if the subproperty exists from the index corresponding to the property id
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return True if there is such subproperty, false otherwise
     */
    bool HasSubPropertiesById(const IndexType SubPropertyIndex)
    {
        return mSubPropertiesList.find(SubPropertyIndex) != mSubPropertiesList.end();
    }

    /**
     * @brief This method checks if the subproperty exists from the index corresponding to the property id  (constant version)
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return True if there is such subproperty, false otherwise
     */
    bool HasSubPropertiesById(const IndexType SubPropertyIndex) const
    {
        return mSubPropertiesList.find(SubPropertyIndex) != mSubPropertiesList.end();
    }

    /**
     * @brief This method gets the subproperty from the index corresponding to the property id
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return The pointer to the subproperty of interest
     */
    Properties::Pointer pGetSubPropertiesById(const IndexType SubPropertyIndex)
    {
        // Checking if the id is the current properties
        if (this->Id() == SubPropertyIndex) {
            return Kratos::make_shared<Properties>(*this);
        }

        // Looking into the database
        auto property_iterator = mSubPropertiesList.find(SubPropertyIndex);
        if (property_iterator != mSubPropertiesList.end()) {
            return *(property_iterator.base());
        } else {
            KRATOS_WARNING("Properties") << "Subproperty ID: " << SubPropertyIndex << " is not defined on the current Properties ID: " << this->Id() << " creating a new one with ID: " << SubPropertyIndex << std::endl;
            auto p_new_property = Kratos::make_shared<Properties>(SubPropertyIndex);
            AddSubProperty(p_new_property);
            return p_new_property;
        }
    }

    /**
     * @brief This method gets the subproperty from the index corresponding to the property id (constant version)
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return The pointer to the subproperty of interest
     */
    Properties::Pointer pGetSubPropertiesById(const IndexType SubPropertyIndex) const
    {
        // Checking if the id is the current properties
        if (this->Id() == SubPropertyIndex) {
            return Kratos::make_shared<Properties>(*this);
        }

        // Looking into the database
        const auto property_iterator = mSubPropertiesList.find(SubPropertyIndex);
        if (property_iterator != mSubPropertiesList.end()) {
            return *(property_iterator.base());
        } else {
            KRATOS_ERROR << "SubProperties\t" << SubPropertyIndex << "\tnot found" << std::endl;
        }
    }

    /**
     * @brief This method gets the subproperty from the index corresponding to the property id
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return The reference to the subproperty of interest
     */
    Properties& GetSubPropertiesById(const IndexType SubPropertyIndex)
    {
        // Checking if the id is the current properties
        KRATOS_WARNING_IF("Properties", this->Id() == SubPropertyIndex) << "Subproperty ID: " << SubPropertyIndex << " coincides on the current Properties ID: " << this->Id() << std::endl;

        // Looking into the database
        if (mSubPropertiesList.find(SubPropertyIndex) != mSubPropertiesList.end()) {
            return *(mSubPropertiesList(SubPropertyIndex));
        } else {
            KRATOS_WARNING("Properties") << "Subproperty ID: " << SubPropertyIndex << " is not defined on the current Properties ID: " << this->Id() << " creating a new one with ID: " << SubPropertyIndex << std::endl;
            AddSubProperty(Kratos::make_shared<Properties>(SubPropertyIndex));
            return *(mSubPropertiesList(SubPropertyIndex));
        }

        return *this;
    }

    /**
     * @brief This method gets the subproperty from the index corresponding to the property id (constant version)
     * @param SubPropertyIndex The index of the subproperty to be get
     * @return The reference to the subproperty of interest
     */
    Properties& GetSubPropertiesById(const IndexType SubPropertyIndex) const
    {
        // Checking if the id is the current properties
        KRATOS_ERROR_IF(this->Id() == SubPropertyIndex) << "Subproperty ID: " << SubPropertyIndex << " coincides on the current Properties ID: " << this->Id() << std::endl;

        // Looking into the database
        if (mSubPropertiesList.find(SubPropertyIndex) != mSubPropertiesList.end()) {
            return *(mSubPropertiesList.find(SubPropertyIndex));
        } else {
            KRATOS_ERROR << "Subproperty ID: " << SubPropertyIndex << " is not defined on the current Properties ID: " << this->Id() << std::endl;
        }
    }

    /**
     * @brief This method checks if the subproperty exists from the adress corresponding to the proper structure of indexes
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return True if there is such subproperty, false otherwise
     */
    bool HasSubPropertiesByAddress(const std::string& rAdress)
    {
        const auto indexes = TrimComponentName(rAdress);
        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                if ( !p_property_interest->HasSubPropertiesById(indexes[i_index])) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief This method checks if the subproperty exists from the adress corresponding to the proper structure of indexes  (constant version)
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return True if there is such subproperty, false otherwise
     */
    bool HasSubPropertiesByAddress(const std::string& rAdress) const
    {
        const auto indexes = TrimComponentName(rAdress);
        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                if ( !p_property_interest->HasSubPropertiesById(indexes[i_index])) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief This method gets the subproperty from the adress corresponding to the proper structure of indexes
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return The pointer to the subproperty of interest
     */
    Properties::Pointer pGetSubPropertiesByAddress(const std::string& rAdress)
    {
        const auto indexes = TrimComponentName(rAdress);

        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                p_property_interest = p_property_interest->pGetSubPropertiesById(indexes[i_index]);
            }
            return p_property_interest;
        } else {
            KRATOS_ERROR << "First index is wrong, does not correspond with current Properties Id: " << indexes[0] << " vs " << this->Id() << std::endl;
        }
    }

    /**
     * @brief This method gets the subproperty from the adress corresponding to the proper structure of indexes (constant version)
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return The pointer to the subproperty of interest
     */
    Properties::Pointer pGetSubPropertiesByAddress(const std::string& rAdress) const
    {
        const auto indexes = TrimComponentName(rAdress);

        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                p_property_interest = p_property_interest->pGetSubPropertiesById(indexes[i_index]);
            }
            return p_property_interest;
        } else {
            KRATOS_ERROR << "First index is wrong, does not correspond with current Properties Id: " << indexes[0] << " vs " << this->Id() << std::endl;
        }
    }

    /**
     * @brief This method gets the subproperty from the adress corresponding to the proper structure of indexes
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return The reference to the subproperty of interest
     */
    Properties& GetSubPropertiesByAddress(const std::string& rAdress)
    {
        const auto indexes = TrimComponentName(rAdress);

        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                p_property_interest = p_property_interest->pGetSubPropertiesById(indexes[i_index]);
            }
            return *p_property_interest;
        } else {
            KRATOS_ERROR << "First index is wrong, does not correspond with current Properties Id: " << indexes[0] << " vs " << this->Id() << std::endl;
        }
    }

    /**
     * @brief This method gets the subproperty from the adress corresponding to the proper structure of indexes (constant version)
     * @param rAdress The text that indicates the structure of subproperties to iterate and found the property of interest
     * @return The reference to the subproperty of interest
     */
    Properties& GetSubPropertiesByAddress(const std::string& rAdress) const
    {
        const auto indexes = TrimComponentName(rAdress);

        if (indexes[0] == this->Id()) {
            Properties::Pointer p_property_interest = Kratos::make_shared<Properties>(*this);
            for (IndexType i_index = 1; i_index < indexes.size(); ++i_index) {
                p_property_interest = p_property_interest->pGetSubPropertiesById(indexes[i_index]);
            }
            return *p_property_interest;
        } else {
            KRATOS_ERROR << "First index is wrong, does not correspond with current Properties Id: " << indexes[0] << " vs " << this->Id() << std::endl;
        }
    }

    /**
     * @brief This method returns the whole list of subproperties
     * @return The whole lis of subproperties
     */
    PropertiesContainerType& GetSubProperties()
    {
        return SubPropertiesList();
    }

    /**
     * @brief This method returns the whole list of subproperties
     * @return The whole lis of subproperties
     */
    PropertiesContainerType const& GetSubProperties() const
    {
        return SubPropertiesList();
    }

    /**
     * @brief This method set the whole list of subproperties
     * @param rSubPropertiesList The list of subproperties
     */
    void SetSubProperties(PropertiesContainerType& rSubPropertiesList)
    {
        mSubPropertiesList = rSubPropertiesList;
    }

    ///@}
    ///@name Access
    ///@{

    /**
     * @brief This method returns the whole data container
     * @return Data container
     */
    ContainerType& Data()
    {
        return mData;
    }

    /**
     * @brief This method returns the whole data container (constant)
     * @return Data container
     */
    ContainerType const& Data() const
    {
        return mData;
    }

    /**
     * @brief This method returns the tables
     * @return The whole lis of tables
     */
    TablesContainerType& Tables()
    {
        return mTables;
    }

    /**
     * @brief This method returns the tables (constant)
     * @return The whole lis of tables
     */
    TablesContainerType const& Tables() const
    {
        return mTables;
    }

    ///@}
    ///@name Inquiry
    ///@{

    template<class TVariableType>
    bool Has(TVariableType const& rThisVariable) const
    {
        return mData.Has(rThisVariable);
    }

    template<class TXVariableType, class TYVariableType>
    bool HasTable(const TXVariableType& XVariable, const TYVariableType& YVariable) const
    {
        return (mTables.find(Key(XVariable.Key(), YVariable.Key())) != mTables.end());
    }


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "Properties";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream <<  "Properties";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        mData.PrintData(rOStream);
        rOStream << "This properties contains " << mTables.size() << " tables";
        if (mSubPropertiesList.size() > 0) {
            rOStream << "\nThis properties contains the following subproperties " << mSubPropertiesList.size() << " subproperties" << std::endl;
            for (auto& subprop : mSubPropertiesList) {
                subprop.PrintData(rOStream);
            }
        }
    }


    ///@}
    ///@name Friends
    ///@{


    ///@}

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


    ///@}
    ///@name Member Variables
    ///@{

    ContainerType mData;                        /// The data stored on the properties

    TablesContainerType mTables;                /// The tables contained on the properties

    PropertiesContainerType mSubPropertiesList; /// The vector containing the list of subproperties

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, IndexedObject );
        rSerializer.save("Data", mData);
        rSerializer.save("Tables", mTables);
        rSerializer.save("SubPropertiesList", mSubPropertiesList);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, IndexedObject );
        rSerializer.load("Data", mData);
        rSerializer.load("Tables", mTables);
        rSerializer.load("SubPropertiesList", mSubPropertiesList);
    }

    ///@}
    ///@name Private  Access
    ///@{

    /**
     * @brief This method trims a string in the different components to access recursively to any subproperty
     * @param rStringName The given name to be trimmed
     * @return The list of indexes
     */
    std::vector<IndexType> TrimComponentName(const std::string& rStringName)
    {
        std::vector<IndexType> list_indexes;

        std::stringstream ss(rStringName);
        for (std::string index_string; std::getline(ss, index_string, '.'); ) {
            list_indexes.push_back(std::stoi(index_string));
        }

        return list_indexes;
    }

    /**
     * @brief This method trims a string in the different components to access recursively to any subproperty (constant version)
     * @param rStringName The given name to be trimmed
     * @return The list of indexes
     */
    std::vector<IndexType> TrimComponentName(const std::string& rStringName) const
    {
        std::vector<IndexType> list_indexes;

        std::stringstream ss(rStringName);
        for (std::string index_string; std::getline(ss, index_string, '.'); ) {
            list_indexes.push_back(std::stoi(index_string));
        }

        return list_indexes;
    }

    /**
     * @brief This method returns the whole list of subproperties (constant)
     * @return The whole lis of subproperties
     */
    PropertiesContainerType& SubPropertiesList()
    {
        return mSubPropertiesList;
    }

    /**
     * @brief This method returns the whole list of subproperties (constant)
     * @return The whole lis of subproperties
     */
    PropertiesContainerType const& SubPropertiesList() const
    {
        return mSubPropertiesList;
    }

    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{


    ///@}

}; // Class Properties

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  Properties& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const Properties& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}


}  // namespace Kratos.

#endif // KRATOS_PROPERTIES_H_INCLUDED  defined
