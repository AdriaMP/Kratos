//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//


#if !defined(KRATOS_KRATOS_PARAMETERS_H_INCLUDED )
#define  KRATOS_KRATOS_PARAMETERS_H_INCLUDED

// System includes
#include <string>
#include <iostream>
#include <sstream>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/ublas_interface.h"
#include "json/json.hpp"                      //import nlohmann json library

namespace Kratos
{
///@addtogroup KratosCore
///@{

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
 * @class Parameters
 * @ingroup KratosCore
 * @brief This class provides to Kratos a data structure for I/O based on the standard of JSON
 * @details In computing, JavaScript Object Notation or JSON is an open-standard file format that uses human-readable text to transmit data objects consisting of attribute–value pairs and array data types (or any other serializable value). It is a very common data format used for asynchronous browser–server communication, including as a replacement for XML in some AJAX-style systems. More info: https://json.org/
 * This class uses nlohmann JSON header only library
 * @author Riccardo Rossi
 */
class Parameters
{
private:
    ///@name Nested clases
    ///@{
    class iterator_adaptor
        : public std::iterator<std::forward_iterator_tag, Parameters>
    {
        using value_iterator = nlohmann::json::iterator;
        value_iterator mValueIterator;
        std::unique_ptr<Parameters> mpParameters;
    public:
        iterator_adaptor(value_iterator itValue,  Kratos::shared_ptr<nlohmann::json> pRoot) :mValueIterator(itValue), mpParameters(new Parameters(&(*itValue), pRoot)) {}
        iterator_adaptor(const iterator_adaptor& itValue) : mValueIterator(itValue.mValueIterator),  mpParameters(new Parameters(*(itValue.mpParameters))) {}
        iterator_adaptor& operator++()
        {
            mValueIterator++;
            return *this;
        }
        iterator_adaptor operator++(int)
        {
            iterator_adaptor tmp(*this);
            operator++();
            return tmp;
        }
        bool operator==(const iterator_adaptor& rhs) const
        {
            return mValueIterator == rhs.mValueIterator;
        }
        bool operator!=(const iterator_adaptor& rhs) const
        {
            return mValueIterator != rhs.mValueIterator;
        }
        Parameters& operator*() const
        {
            mpParameters->mpValue = &(*mValueIterator);
            return *mpParameters;
        }
        Parameters* operator->() const
        {
            mpParameters->mpValue = &(*mValueIterator);
            return mpParameters.get();
        }
        value_iterator& base()
        {
            return mValueIterator;
        }
        value_iterator const& base() const
        {
            return mValueIterator;
        }
        const std::string name()
        {
            return mValueIterator.key();
        }
    };

    class const_iterator_adaptor : public std::iterator<std::forward_iterator_tag, Parameters>
    {
        using value_iterator = nlohmann::json::const_iterator;
        value_iterator mValueIterator;
        std::unique_ptr<Parameters> mpParameters;
    public:
        const_iterator_adaptor(value_iterator it,  Kratos::shared_ptr<nlohmann::json> proot) :mValueIterator(it), mpParameters(new Parameters(const_cast<nlohmann::json*>(&(*it)), proot)) {}
        //TODO: use copy constructor in the following method
        const_iterator_adaptor(const const_iterator_adaptor& it) : mValueIterator(it.mValueIterator), mpParameters(new Parameters(*(it.mpParameters))) {}
        const_iterator_adaptor& operator++()
        {
            mValueIterator++;
            mpParameters->mpValue = const_cast<nlohmann::json*>( &(*mValueIterator) );
            return *this;
        }
        const_iterator_adaptor operator++(int)
        {
            const_iterator_adaptor tmp(*this);
            operator++();
            return tmp;
        }
        bool operator==(const const_iterator_adaptor& rhs) const
        {
            return mValueIterator == rhs.mValueIterator;
        }
        bool operator!=(const const_iterator_adaptor& rhs) const
        {
            return mValueIterator != rhs.mValueIterator;
        }
        const Parameters& operator*() const
        {
            //mpParameters->mpValue = &(*mValueIterator);
            return *mpParameters;

        }
        const Parameters* operator->() const
        {
            return mpParameters.get();
        }
        value_iterator& base()
        {
            return mValueIterator;
        }
        value_iterator const& base() const
        {
            return mValueIterator;
        }
        const std::string name()
        {
            return mValueIterator.key();
        }
    };

    ///@}

public:
    ///@name Type Definitions
    ///@{

    /// Index definition
    typedef std::size_t IndexType;

    /// Size definition
    typedef std::size_t SizeType;

    /// Pointer definition of MmgProcess
    KRATOS_CLASS_POINTER_DEFINITION(Parameters);

    /// Definiton of the iterators
    using iterator = iterator_adaptor;
    using const_iterator = const_iterator_adaptor;

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * @brief Default constructor.
     * @brief It assigns null pointers to the member variables
     */
    Parameters()
    {
        mpRoot = nullptr;
        mpValue = nullptr;
    }

    /**
     * @brief String constructor. It takes a string as input, which parses into a nlohmann::json class
     * @param rJsonString The string to be parsed into a nlohmann::json class
     */
    Parameters(const std::string& json_string)
    {
        mpRoot = Kratos::shared_ptr<nlohmann::json>(new nlohmann::json( nlohmann::json::parse( json_string )));
        mpValue = mpRoot.get();
    }

    /// Copy constructor.
    Parameters(Parameters const& rOther)
    {
        //TODO: verify if mpValue is not null and eventually destruct correctly the data
        mpRoot = rOther.mpRoot;
        mpValue = rOther.mpValue;
    }

    /// Destructor.
    virtual ~Parameters()
    {
    }

    ///@}
    ///@name Operators
    ///@{

    /// Assignment operator.
    Parameters& operator=(Parameters const& rOther)
    {
        if(mpRoot.get() ==  mpValue || mpRoot == nullptr) {
            mpRoot = Kratos::shared_ptr<nlohmann::json>(new nlohmann::json( nlohmann::json::parse( rOther.WriteJsonString() )));
            mpValue = mpRoot.get();
        } else {
            *mpValue = nlohmann::json( nlohmann::json::parse( rOther.WriteJsonString() ) );
            // note that mpRoot is unchanged
        }

        return *this;
    }

    /**
     * @brief This metrod returns the Parameter corresponding to a given key
     * @param rEntry The key identifier of the parameter
     * @return The desired Parameter
     */
    Parameters operator[](const std::string& rEntry)
    {
        return this->GetValue(rEntry);
    }

    /**
     * @brief This method allows to acces to an array item with the operator []
     * @param Index The index of the term of interest
     * @return The desired Parameter
     */
    Parameters operator[](const IndexType Index)
    {
        return this->GetArrayItem(Index);
    }

    ///@}
    ///@name Operations
    ///@{

    //generates a clone of the current document
    Parameters Clone()
    {
        //TODO: make a clone
        //TODO: find a better way to make the copy
        return Parameters(mpValue->dump());                     //new json(*mpValue));
    }

    /**
     * @brief This method returns a string with the corresponding text to the equivalent *.json file
     * @return The corresponding text
     */
    const std::string WriteJsonString() const
    {
        return mpValue->dump();
    }

    /**
     * @brief This method returns a string with the corresponding text to the equivalent *.json file (this version is prettier, and considers tabulations)
     * @return The corresponding text
     */
    const std::string PrettyPrintJsonString() const
    {
        return mpValue->dump(4);
    }

    /**
     * @brief This method returns the Parameter corresponding to a certain entry
     * @param rEntry The key identifier of the parameter
     * @return The corresponding parameter
     */
    Parameters GetValue(const std::string& rEntry)
    {
        auto j = mpValue->find(rEntry);
        KRATOS_ERROR_IF(j == mpValue->end()) << "Getting a value that does not exist. entry string : " << rEntry << std::endl;
        return Parameters(&(*j), mpRoot);
    }

    /**
     * @brief This method sets an existing parameter with a given parameter
     * @param rEntry The key identifier of the parameter
     * @param rOtherValue The value to set
     */
    void SetValue(
        const std::string& rEntry,
        const Parameters& rOtherValue
        )
    {
        KRATOS_ERROR_IF(mpValue->find(rEntry) == mpValue->end()) << "Value must exist to be set. Use AddValue instead" << std::endl;
        (*mpValue)[rEntry] = *(rOtherValue.mpValue);
    }

    /**
     * @brief This method sets a non-existing parameter with a given parameter
     * @param rEntry The key identifier of the parameter
     * @param rOtherValue The value to set
     */
    void AddValue(
        const std::string& rEntry,
        const Parameters& rOtherValue
        )
    {
        if(mpValue->find(rEntry) == mpValue->end()) {
            (*mpValue)[rEntry] = *(rOtherValue.mpValue);
        }
    }

    /**
     * @brief This method adds an empty parameter
     * @param rEntry The key identifier of the parameter
     */
    Parameters AddEmptyValue(const std::string& rEntry)
    {
        if(this->Has(rEntry) == false) {
            return Parameters(&(*mpValue)[rEntry],  mpRoot);
        }
        return this->GetValue(rEntry);
    }

    /**
     * @brief This method removes an entry of the Parameters given a certain key
     * @param rEntry The key identifier of the parameter
     * @return False if failed, true otherwise
     */
    bool RemoveValue(const std::string& rEntry)
    {
        return static_cast<bool>(mpValue->erase(rEntry));
    }

    /**
     * @brief This method checks if the Parameter contains a certain entry
     * @param rEntry The key identifier of the parameter
     * @return True if it contains, false otherwise
     */
    bool Has(const std::string& rEntry) const
    {
        return mpValue->find(rEntry) != mpValue->end();
    }

    /**
     * @brief This method checks if the parameter is a null
     * @return True if it is null, false otherwise
     */
    bool IsNull() const
    {
        return mpValue->is_null();
    }

    /**
     * @brief This method checks if the parameter is a number
     * @return True if it is a number, false otherwise
     */
    bool IsNumber() const
    {
        return mpValue->is_number();
    }

    /**
     * @brief This method checks if the parameter is a double
     * @return True if it is a double, false otherwise
     */
    bool IsDouble() const
    {
        return mpValue->is_number_float();
    }

    /**
     * @brief This method checks if the parameter is a integer
     * @return True if it is a integer, false otherwise
     */
    bool IsInt() const
    {
        return mpValue->is_number_integer();
    }

    /**
     * @brief This method checks if the parameter is a boolean
     * @return True if it is a boolean, false otherwise
     */
    bool IsBool() const
    {
        return mpValue->is_boolean();
    }

    /**
     * @brief This method checks if the parameter is a string
     * @return True if it is a string, false otherwise
     */
    bool IsString() const
    {
        return mpValue->is_string();
    }

    /**
     * @brief This method checks if the parameter is an array
     * @return True if it is an array, false otherwise
     */
    bool IsArray() const
    {
        return mpValue->is_array();
    }

    /**
     * @brief This method checks if the parameter is a vector
     * @return True if it is a vector, false otherwise
     */
    bool IsVector() const
    {
        if (!mpValue->is_array())
            return false;

        for (IndexType i = 0; i < mpValue->size(); ++i) {
            if (!(*mpValue)[i].is_number())
                return false;
        }
        return true; // All entries are numbers or Vector is empty
    }

    /**
     * @brief This method checks if the parameter is a matrix
     * @return True if it is a matrix, false otherwise
     */
    bool IsMatrix() const
    {
        if (!mpValue->is_array()) // mpValue != [ ... ]
            return false;

        const SizeType nrows = mpValue->size();
        if (nrows == 0) // mpValue is an empty array/vector => "[]"
            return false;

        for (IndexType i = 0; i < nrows; ++i) {
            auto& row_i = (*mpValue)[i];
            if (!row_i.is_array())
                return false;

            IndexType ncols = row_i.size();
            if (ncols != (*mpValue)[0].size()) // Compare number of columns to first row
                return false;                  // Number of columns is not consistent

            for (IndexType j = 0; j < ncols; ++j) { // Check all values in column
                if (!row_i[j].is_number())
                return false;
            }
        }

        return true; // All entries are numbers or Matrix is empty ([[]] or
                     // [[],[],[],...])
    }

    /**
     * @brief This method checks if the parameter is a subparameter
     * @return True if it is a suparameter, false otherwise
     */
    bool IsSubParameter() const
    {
        return mpValue->is_object();
    }

    /**
     * @brief This method returns the double contained in the current Parameter
     * @return The double value
     */
    double GetDouble() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_number()) << "Argument must be a number" << std::endl;
        return mpValue->get<double>();
    }

    /**
     * @brief This method returns the integer contained in the current Parameter
     * @return The integer value
     */
    int GetInt() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_number()) << "Argument must be a number" << std::endl;
        return mpValue->get<int>();
    }

    /**
     * @brief This method returns the boolean contained in the current Parameter
     * @return The boolean value
     */
    bool GetBool() const
    {
        if (mpValue->is_boolean() == false) {
            //RecursivelyFindValue(*mpdoc, *mpValue);
            KRATOS_ERROR << "Argument must be a bool" << std::endl;
        }
        return mpValue->get<bool>();
    }

    /**
     * @brief This method returns the string contained in the current Parameter
     * @return The string value
     */
    std::string GetString() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_string()) << "Argument must be a string" << std::endl;
        return mpValue->get<std::string>();
    }

    /**
     * @brief This method returns the vector contained in the current Parameter
     * @return The vector value
     */
    Vector GetVector() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "Argument must be a Vector (a json list)" << std::endl;

        const SizeType size = mpValue->size();

        Vector aux_V(size);

        for (IndexType i = 0; i < size; ++i) {
            KRATOS_ERROR_IF_NOT((*mpValue)[i].is_number()) << "Entry " << i << " is not a number!" << std::endl;
            aux_V(i) = (*mpValue)[i].get<double>();
        }

        return aux_V;
    }

    /**
     * @brief This method returns the matrix contained in the current Parameter
     * @return The matrix value
     */
    Matrix GetMatrix() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "Argument must be a Matrix (a json list of lists)" << std::endl;

        const SizeType nrows = mpValue->size();
        KRATOS_ERROR_IF(nrows == 0) << "Argument must be a Matrix (a json list of lists)" << std::endl;

        IndexType ncols = 0;
        if ((*mpValue)[0].is_array())
            ncols = (*mpValue)[0].size();

        Matrix aux_A(nrows, ncols);

        for (IndexType i = 0; i < nrows; ++i) {
            auto &row_i = (*mpValue)[i];
            KRATOS_ERROR_IF_NOT(row_i.is_array()) << "Not an array on row " << i << std::endl;
            KRATOS_ERROR_IF_NOT(row_i.size() == ncols) << "Wrong size of row " << i << std::endl;
            for (IndexType j = 0; j < ncols; ++j) {
                KRATOS_ERROR_IF_NOT((row_i)[j].is_number()) << "Entry (" << i << "," << j << ") is not a number!" << std::endl;
                aux_A(i, j) = (row_i)[j].get<double>();
            }
        }

        return aux_A;
    }

    /**
     * @brief This method sets the double contained in the current Parameter
     * @param Value The double value
     */
    void SetDouble(const double Value)
    {
        *mpValue=Value;
    }

    /**
     * @brief This method sets the integer contained in the current Parameter
     * @param Value The integer value
     */
    void SetInt(const int Value)
    {
        *mpValue=Value;
    }

    /**
     * @brief This method sets the bool contained in the current Parameter
     * @param Value The bool value
     */
    void SetBool(const bool Value)
    {
        *mpValue=Value;
    }

    /**
     * @brief This method sets the string contained in the current Parameter
     * @param rValue The string value
     */
    void SetString(const std::string& rValue)
    {
        *mpValue=rValue;
    }

    /**
     * @brief This method sets the vector contained in the current Parameter
     * @param rValue The vector value
     */
    void SetVector(const Vector& rValue)
    {
        const SizeType size = rValue.size();

        nlohmann::json j_array(0.0, size);
        (*mpValue) = j_array;

        for (IndexType i = 0; i < size; ++i) {
            mpValue[i] = rValue[i];
        }
    }

    /**
     * @brief This method sets the matrix contained in the current Parameter
     * @param Value The matrix value
     */
    void SetMatrix(const Matrix& rValue)
    {
        const SizeType nrows = rValue.size1();
        const SizeType ncols = rValue.size2();

        nlohmann::json j_col_array(0.0, ncols);
        nlohmann::json j_row_array(0.0, nrows);
        (*mpValue) = j_row_array;

        for (IndexType i = 0; i < nrows; ++i) {
            (*mpValue)[i] = j_col_array;

            for (IndexType j = 0; j < ncols; ++j) {
                (*mpValue)[i][j] = rValue(i, j);
            }
        }
    }

    /**
     * @brief This returns the begin iterator
     * @return The begin iterator
     */
    iterator begin()
    {
        return iterator(mpValue->begin(),  mpRoot);
    }

    /**
     * @brief This returns the end iterator
     * @return The end iterator
     */
    iterator end()
    {
        return iterator(mpValue->end(),  mpRoot);
    }

    /**
     * @brief This returns the constant begin iterator
     * @return The constant begin iterator
     */
    const_iterator begin() const
    {
        return const_iterator(mpValue->cbegin(),  mpRoot);
    }

    /**
     * @brief This returns the constant end iterator
     * @return The constant end iterator
     */
    const_iterator end() const
    {
        return const_iterator(mpValue->cend(),  mpRoot);
    }

    /**
     * @brief This method returns the total size of the current array parameter
     * @return The size of the current array parameter
     */
    SizeType size() const
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array())  << "Size can only be queried if the value if of Array type" << std::endl;
        return mpValue->size();
    }

    /**
     * @brief This method does a swap between two parameters
     * @param rOther The othe parameter to compute the swap
     */
    void swap(Parameters& rOther) noexcept
    {
        std::swap(mpValue, rOther.mpValue);
        std::swap(mpRoot, rOther.mpRoot);
    }

    /**
     * @brief This method resets the whole parameter (it assigns an empty parameter)
     */
    void Reset() noexcept
    {
        Parameters p;
        swap(p);
    }

    /**
     * @brief This method returns an array item given an index
     * @param Index The index of the parameter to obtain
     * @return The parameter corresponding to the given index
     */
    Parameters GetArrayItem(const IndexType Index)
    {
        if(mpValue->is_array() == false) {
            KRATOS_ERROR << "GetArrayItem only makes sense if the value if of Array type" << std::endl;
        } else {
            KRATOS_ERROR_IF(Index >= mpValue->size()) << "Index exceeds array size. Index value is : " << Index << std::endl;
            return Parameters(&((*mpValue)[Index]),  mpRoot);
        }
    }

    /**
     * @brief This method sets an array item given an index
     * @param Index The index of the parameter to set
     * @param rOtherArrayItem The parameter corresponding to the given index
     */
    void SetArrayItem(
        const IndexType Index,
        const Parameters& rOtherArrayItem
        )
    {
        if(mpValue->is_array() == false) {
            KRATOS_ERROR << "SetArrayItem only makes sense if the value if of Array type" << std::endl;
        } else {
            KRATOS_ERROR_IF(Index >= mpValue->size()) << "Index exceeds array size. Index value is : " << Index <<std::endl;
            (*mpValue)[Index] = *rOtherArrayItem.mpValue;
        }
    }

    /**
     * @brief This method add a new entry with no value assigned
     * @param rEntry The key identifier of the parameter
     */
    void AddEmptyArray(const std::string& rEntry)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        if(mpValue->find(rEntry) == mpValue->end()) {
            nlohmann::json j_array(nlohmann::json::value_t::array);
            (*mpValue)[rEntry] = j_array;
        }
    }

    /**
     * @brief This method appends into an array a double value
     * @param Value The double value to append
     */
    void Append(const double Value)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        nlohmann::json j_number_float(nlohmann::json::value_t::number_float);
        j_number_float = Value;
        mpValue->push_back(j_number_float);
    }

    /**
     * @brief This method appends into an array a integer value
     * @param Value The integer value to append
     */
    void Append(const int Value)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        nlohmann::json j_number_integer(nlohmann::json::value_t::number_integer);
        j_number_integer = Value;
        mpValue->push_back(j_number_integer);
    }

    /**
     * @brief This method appends into an array a boolean value
     * @param Value The boolean value to append
     */
    void Append(const bool Value)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        nlohmann::json j_boolean(nlohmann::json::value_t::boolean);
        j_boolean = Value;
        mpValue->push_back(j_boolean);
    }

    /**
     * @brief This method appends into an array a string value
     * @param rValue The string value to append
     */
    void Append(const std::string& rValue)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        nlohmann::json j_string(nlohmann::json::value_t::string);
        j_string = rValue;
        mpValue->push_back(j_string);
    }

    /**
     * @brief This method appends into an array a vector value
     * @param rValue The vector value to append
     */
    void Append(const Vector& rValue)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        const SizeType size = rValue.size();

        nlohmann::json j_array(0.0, size);

        for (IndexType i = 0; i < size; ++i) {
            j_array = rValue[i];
        }

        mpValue->push_back(j_array);
    }

    /**
     * @brief This method appends into an array a matrix value
     * @param rValue The matrix value to append
     */
    void Append(const Matrix& rValue)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        const SizeType nrows = rValue.size1();
        const SizeType ncols = rValue.size2();

        nlohmann::json j_col_array(0.0, ncols);
        nlohmann::json j_row_array(0.0, nrows);

        for (IndexType i = 0; i < nrows; ++i) {
            for (IndexType j = 0; j < ncols; ++j) {
                j_col_array[j] = rValue(i, j);
            }

            j_row_array[i] = j_col_array;
        }

        mpValue->push_back(j_row_array);
    }

    /**
     * @brief This method appends into an array a Parameter value
     * @param rValue The Parameter value to append
     */
    void Append(const Parameters& rValue)
    {
        KRATOS_ERROR_IF_NOT(mpValue->is_array()) << "It must be an Array parameter to append" << std::endl;
        nlohmann::json j_object = nlohmann::json( nlohmann::json::parse( rValue.WriteJsonString() ) );
        mpValue->push_back(j_object);
    }

    /**
     * @brief This method looks in a recursive way in the json structure
     * @param rBaseValue The value where to find
     * @param rValueToFind The value to look
     */
    void RecursivelyFindValue(
        const nlohmann::json& rBaseValue,
        const nlohmann::json& rValueToFind
        ) const
    {
        // TODO: Finish this!!!
//         for (auto itr = rBaseValue.begin(); itr != rBaseValue.end(); ++itr) {
//             if (&(itr->value) == &rValueToFind) {
// //                 rapidjson::StringBuffer buffer;
// //                 rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
// //                 mpValue->Accept(writer);
// //                 KRATOS_INFO("Parameters") << "Base = " << buffer.GetString() << std::endl
// //                 << "Problematic var name " << itr->name.GetString() << " value " << itr->value.GetString() << std::endl;
//             } else {
//                 if (itr->value.is_object()) RecursivelyFindValue(itr->value, rValueToFind);
//                 //TODO: it could be an array
//             }
//         }
    }

    /**
     * @brief Checks if the names and values are the same, no importance to the order.
     * @details Lists have to be ordered, though! Take into account that in Kratos some physical vectors are represented with a list.
     * @param rParameters The parameters to be checked
     * @return True if it has, false othersise
     */
    bool IsEquivalentTo(Parameters& rParameters)
    {
        // TODO: Implement this
//         for (auto itr = this->mpValue->begin(); itr != this->mpValue->end(); ++itr) {
//             std::string item_name = itr->name.GetString();
//
//             bool found = false;
//
//             for (auto itr_ref = rParameters.mpValue->begin(); itr_ref != rParameters.mpValue->end(); ++itr_ref) {
//                 if (item_name == itr_ref->name.GetString()) {
//                     found = true;
//                     Parameters subobject = (*this)[item_name];
//                     Parameters reference_subobject = rParameters[item_name];
//
//                     if (itr->value.IsObject()) {
//                         if (!subobject.IsEquivalentTo(reference_subobject))
//                             return false;
//                     } else {
//                         if (itr->value != itr_ref->value)
//                             return false;
//                     }
//                     break;
//                 }
//             }
//
//             if (!found)
//                 return false;
//         }
//
//         // Reverse check: the rParameters can contain fields that are missing in the object
//         for (auto itr =  rParameters.mpValue->begin();  itr != rParameters.mpValue->end(); ++itr) {
//             std::string item_name = itr->name.GetString();
//
//             bool found = false;
//
//             for (auto itr_ref = this->mpValue->begin(); itr_ref != this->mpValue->end(); ++itr_ref) {
//                 if (item_name == itr_ref->name.GetString()) {
//                     found = true;
//                     // no need to check the values here, if they were found in the
//                     // previous loop, values were checked there
//                     break;
//                 }
//             }
//
//             if (!found)
//                 return false;
//         }

        return true;
    }

    /**
     * @brief  Checks if the names and the type of values are the same, no importance to the order.
     * @details Lists have to be ordered, though! Take into account that in Kratos some physical vectors are represented with a list.
     * @param rParameters The parameters to be checked
     * @return True if it has, false othersise
     */
    bool HasSameKeysAndTypeOfValuesAs(Parameters& rParameters)
    {
        // TODO: Implement this
//         for (auto itr = this->mpValue->begin(); itr != this->mpValue->end(); ++itr) {
//             std::string item_name = itr->name.GetString();
//
//             bool found = false;
//
//             for (auto itr_ref = rParameters.mpValue->begin(); itr_ref != rParameters.mpValue->end(); ++itr_ref) {
//                 if (item_name == itr_ref->name.GetString()) {
//                     found = true;
//                     Parameters subobject = (*this)[item_name];
//                     Parameters reference_subobject = rParameters[item_name];
//
//                     if (itr->value.IsObject()) {
//                         if (!subobject.HasSameKeysAndTypeOfValuesAs(reference_subobject))
//                             return false;
//                     } else {
//                         if (itr->value.GetType() != itr_ref->value.GetType()) {
//                             return false;
//                         }
//                     }
//                     break;
//                 }
//             }
//
//             if (!found)
//                 return false;
//         }
//
//         // reverse check: the rParameters can contain fields that are missing in the
//         // object
//         for (auto itr = rParameters.mpValue->begin(); itr != rParameters.mpValue->end(); ++itr) {
//             std::string item_name = itr->name.GetString();
//
//         bool found = false;
//
//         for (auto itr_ref =  this->mpValue->begin(); itr_ref != this->mpValue->end(); ++itr_ref) {
//             if (item_name == itr_ref->name.GetString()) {
//                 found = true;
//                 // no need to check the types here, if they were found in the previous
//                 // loop, types were checked there
//                 break;
//             }
//         }
//
//         if (!found)
//             return false;
//         }

        return true;
    }

    /**
     * @brief This function is designed to verify that the parameters under testing match the form prescribed by the rDefaultParameters.
     * @details If the parameters contain values that do not appear in the rDefaultParameters, an error is thrown, whereas if a parameter is found in the rDefaultParameters but not in the Parameters been tested, it is copied to the parameters.
     * This version of the function only walks one level, without descending in the branches
     * @param rDefaultParameters Parameters of reference which we use to check
     */
    void ValidateAndAssignDefaults(Parameters& rDefaultParameters)
    {
        KRATOS_TRY

        // First verifies that all the enries in the current parameters have a correspondance in the rDefaultParameters.
        // If it is not the case throw an error
        for (auto itr = this->mpValue->begin(); itr != this->mpValue->end(); ++itr) {
            std::string item_name = itr.key();
            if(!rDefaultParameters.Has(item_name) ) {
                std::stringstream msg;
                msg << "The item with name \"" << item_name << "\" is present in this Parameters but NOT in the default values" << std::endl;
                msg << "Hence Validation fails" << std::endl;
                msg << "Parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "Defaults against which the current parameters are validated are :" << std::endl;
                msg << rDefaultParameters.PrettyPrintJsonString() << std::endl;
                KRATOS_ERROR << msg.str() << std::endl;
            }

            bool type_coincides = false;
            auto value_defaults = (rDefaultParameters[item_name]).GetUnderlyingStorage();
            if(itr->is_number() && value_defaults->is_number()) type_coincides = true;

//             if(itr->is_number_float() && value_defaults->is_number_float()) type_coincides = true;
            if(itr->is_array() && value_defaults->is_array()) type_coincides = true;
            if(itr->is_string() && value_defaults->is_string()) type_coincides = true;
            if(itr->is_object() && value_defaults->is_object()) type_coincides = true;

            // Both must be bool to be acceptable
            if(itr->is_boolean() && value_defaults->is_boolean()) type_coincides = true;

            if(type_coincides == false) {
                std::stringstream msg;
                msg << "******************************************************************************************************" << std::endl;
                msg << "The item with name :\"" << item_name << "\" does not have the same type as the corresponding one in the default values" << std::endl;
                msg << "******************************************************************************************************" << std::endl;
                msg << "Parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "Defaults against which the current parameters are validated are :" << std::endl;
                msg << rDefaultParameters.PrettyPrintJsonString() << std::endl;
                KRATOS_ERROR << msg.str() << std::endl;
            }

            // Now iterate over all the rDefaultParameters. In the case a default value is not assigned in the current Parameters add an item copying its value
            if(rDefaultParameters.IsSubParameter()) {
                for (auto itr = rDefaultParameters.mpValue->begin(); itr != rDefaultParameters.mpValue->end(); ++itr) {
                    std::string item_name = itr.key();
                    if(!this->Has(item_name)) {
                        (*mpValue)[item_name] = itr.value();
                    }
                }
            }
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief This function is designed to verify that the parameters under testing match the form prescribed by the defaults.
     * @details If the parameters contain values that do not appear in the defaults, an error is thrown, whereas if a parameter is found in the defaults but not in the Parameters been tested, it is copied to the parameters.
     * This version walks and validates the entire json tree below the point at which the function is called
     * @param rDefaultParameters Parameters of reference which we use to check
     */
    void RecursivelyValidateAndAssignDefaults(Parameters& rDefaultParameters)
    {
        KRATOS_TRY

        // First verifies that all the enries in the current parameters have a correspondance in the rDefaultParameters.
        // If it is not the case throw an error
        for (auto itr = this->mpValue->cbegin(); itr != this->mpValue->cend(); ++itr) {
            const std::string& item_name = itr.key();

            if(!rDefaultParameters.Has(item_name) ) {
                std::stringstream msg;
                msg << "The item with name \"" << item_name << "\" is present in this Parameters but NOT in the default values" << std::endl;
                msg << "Hence Validation fails" << std::endl;
                msg << "Parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "Defaults against which the current parameters are validated are :" << std::endl;
                msg << rDefaultParameters.PrettyPrintJsonString() << std::endl;
                KRATOS_ERROR << msg.str() << std::endl;
            }

            bool type_coincides = false;
            auto value_defaults = (rDefaultParameters[item_name]).GetUnderlyingStorage();
            if(itr->is_number_integer() && value_defaults->is_number_integer()) type_coincides = true;
            if(itr->is_boolean() && value_defaults->is_boolean()) type_coincides = true;
            if(itr->is_number_float() && value_defaults->is_number_float()) type_coincides = true;
            if(itr->is_array() && value_defaults->is_array()) type_coincides = true;
            if(itr->is_string() && value_defaults->is_string()) type_coincides = true;
            if(itr->is_object() && value_defaults->is_object()) type_coincides = true;

            if(type_coincides == false) {
                std::stringstream msg;
                msg << "The item with name :\"" << item_name << "\" does not have the same type as the corresponding one in the default values" << std::endl;
                msg << "Parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "Defaults against which the current parameters are validated are :" << std::endl;
                msg << rDefaultParameters.PrettyPrintJsonString() << std::endl;
                KRATOS_ERROR << msg.str() << std::endl;
            }

            // Now walk the tree recursively
            if(itr->is_object()) {
                Parameters subobject = (*this)[item_name];
                Parameters defaults_subobject = rDefaultParameters[item_name];
                subobject.ValidateAndAssignDefaults(defaults_subobject);
            }
        }

        // Now iterate over all the rDefaultParameters. In the case a default value is not assigned in the current Parameters add an item copying its value
        if(rDefaultParameters.IsSubParameter()) {
            for (auto itr = rDefaultParameters.mpValue->begin(); itr != rDefaultParameters.mpValue->end(); ++itr) {
                const std::string& item_name = itr.key();
                if(mpValue->find(item_name) ==  mpValue->end()) {
                    (*mpValue)[item_name] = itr.value();
                }

                // Now walk the tree recursively
                if(itr->is_object()) {
                    Parameters subobject = (*this)[item_name];
                    Parameters defaults_subobject = rDefaultParameters[item_name];
                    subobject.ValidateAndAssignDefaults(defaults_subobject);
                }
            }
        }

        KRATOS_CATCH("")
    }

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        return this->PrettyPrintJsonString();
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << "Parameters Object " << Info();
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const {};

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

    nlohmann::json* mpValue;                   /// This is where the json is actually stored
    Kratos::shared_ptr<nlohmann::json> mpRoot; /// This is a shared pointer to the root structure (this is what allows us to acces in a tree structure to the JSON database)

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief Direct constructor. It takes as parameters the "member" variables of the Parameters class
     * @param pValue The nlohmann::json class raw pointer
     * @param pRoot A shared pointer to a nlohmann::json class
     * @warning Please DO NOT use this constructor. It assumes nlohmann::json and hence it should be considered as an implementation detail
     */
    Parameters(nlohmann::json* pvalue, Kratos::shared_ptr<nlohmann::json> proot): mpValue(pvalue), mpRoot(proot)
    {}

    //ATTENTION: please DO NOT use this constructor. It assumes rapidjson and hence it should be considered as an implementation detail
//     Parameters(const json::iterator& it): mpValue(*it)
//     {
//         mis_owner = false;
//     }
//     //ATTENTION: please DO NOT use this constructor. It assumes rapidjson and hence it should be considered as an implementation detail
//     Parameters(const json::const_iterator& it): mpValue(*it)
//     {
//         mis_owner = false;
//     }

    /**
     * @brief This method is created in order to access from the iterators to the database
     * @return mpValue The database storage
     * @warning Please DO NOT use this method. It is a low level accessor, and may change in the future
     */
    nlohmann::json* GetUnderlyingStorage()
    {
        return mpValue;
    }

    /**
     * @brief This method is created in order to set the database
     * @param pNewValue The database storage
     * @warning Please DO NOT use this method. It is a low level accessor, and may change in the future
     */
    void SetUnderlyingSotrage(nlohmann::json* pNewValue)
    {
        mpValue = pNewValue;
    }

    /**
     * @brief This method is created in order to access from the iterators to the database
     * @return mpValue The database storage
     * @warning Please DO NOT use this method. It is a low level accessor, and may change in the future
     */
    Kratos::shared_ptr<nlohmann::json> GetUnderlyingRootStorage()
    {
        return mpRoot;
    }

    /**
     * @brief This method is created in order to set the database
     * @param pNewValue The database storage
     * @warning Please DO NOT use this method. It is a low level accessor, and may change in the future
     */
    void SetUnderlyingRootStorage(Kratos::shared_ptr<nlohmann::json> pNewValue)
    {
        mpRoot = pNewValue;
    }

    /**
     * @brief This method sets the database from other Parameters
     * @param rOtherValue The database to copy
     * @warning Please DO NOT use this method. It is a low level accessor, and may change in the future
     */
    void InternalSetValue(const Parameters& rOtherValue)
    {
        mpValue = new nlohmann::json( nlohmann::json::parse( rOtherValue.WriteJsonString()));
    }

}; // Parameters class

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  Parameters& rThis)
{
    return rIStream;
}

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const Parameters& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_KRATOS_PARAMETERS_H_INCLUDED  defined
