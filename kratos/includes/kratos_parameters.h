//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license:
//kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//

#if !defined(KRATOS_KRATOS_PARAMETERS_H_INCLUDED)
#define KRATOS_KRATOS_PARAMETERS_H_INCLUDED

// System includes

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

// External includes

// Project includes
#include "includes/define.h"
#include "json/json.hpp"                     

namespace Kratos {
///@addtogroup ApplicationNameApplication
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
class Parameters {
private:
    using json = nlohmann::json;
    ///@name Nested clases
    ///@{
    class iterator_adaptor : public std::iterator<std::forward_iterator_tag, Parameters>
    {
        using value_iterator = json::iterator;
        value_iterator mValueIterator;
        std::unique_ptr<Parameters> mpParameters;
    public:
        iterator_adaptor(value_iterator it,  boost::shared_ptr<json> proot) :mValueIterator(it), mpParameters(new Parameters(&(*it), proot)) {}
        iterator_adaptor(const iterator_adaptor& it) : mValueIterator(it.mValueIterator),  mpParameters(new Parameters(*(it.mpParameters))) {}
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
            mpParameters->mpvalue = &(*mValueIterator);
            return *mpParameters;
        }
        Parameters* operator->() const
        {
            mpParameters->mpvalue = &(*mValueIterator);
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
        using value_iterator = json::const_iterator;
        value_iterator mValueIterator;
        std::unique_ptr<Parameters> mpParameters;
    public:
        const_iterator_adaptor(value_iterator it,  boost::shared_ptr<json> proot) :mValueIterator(it), mpParameters(new Parameters(const_cast<json*>(&(*it)), proot)) {}
        //TODO: use copy constructor in the following method
        const_iterator_adaptor(const const_iterator_adaptor& it) : mValueIterator(it.mValueIterator), mpParameters(new Parameters(*(it.mpParameters))) {}
        const_iterator_adaptor& operator++()
        {
            mValueIterator++;
            mpParameters->mpvalue = const_cast<json*>( &(*mValueIterator) );
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
            //mpParameters->mpvalue = &(*mValueIterator);
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
  KRATOS_CLASS_POINTER_DEFINITION(Parameters);

  using iterator = iterator_adaptor;
  using const_iterator = const_iterator_adaptor;

  Parameters(const std::string& json_string = "{}") {

    mpdoc = Kratos::make_shared<rapidjson::Document>();
    rapidjson::ParseResult ok = mpdoc->Parse<0>(json_string.c_str());

    if (!ok) {
      std::stringstream msg;
      msg << rapidjson::GetParseError_En(ok.Code())
          << " offset of the error from the beginning of the string = "
          << ok.Offset() << std::endl;
      msg << "a much more explicative error message can be obtained by "
             "analysing the input string with an online analyzer such for "
             "example json lint"
          << std::endl;
      msg << "the value of the string that was attempted to parse is :"
          << std::endl
          << std::endl;
      msg << json_string;
      KRATOS_ERROR << "error found in parsing the json_string, the value of "
                      "the json string was: \n"
                   << msg.str() << std::endl;
    }

    mpvalue = (mpdoc.get());
  }

  /// Assignment operator.
  Parameters &operator=(Parameters const &rOther) {
    mpvalue->CopyFrom(*(rOther.GetUnderlyingStorage()), mpdoc->GetAllocator());

    Parameters(const std::string& json_string)
    {
        mproot = boost::shared_ptr<json>(new json( json::parse( json_string )));
        mpvalue = mproot.get();
    }

    /// Assignment operator.
    Parameters& operator=(Parameters const& rOther)
    {
        if(mproot.get() ==  mpvalue || mproot == nullptr)
        {
            mproot = boost::shared_ptr<json>(new json( json::parse( rOther.WriteJsonString() )));
            mpvalue = mproot.get();
        }
        else
        {
            *mpvalue = json( json::parse( rOther.WriteJsonString() ) );
            // note that mproot is unchanged
        }

        return *this;
    }
    /// Copy constructor.
    Parameters(Parameters const& rOther)
    {
        //TODO: verify if mpvalue is not null and eventually destruct correctly the data
        mproot = rOther.mproot;
        mpvalue = rOther.mpvalue;
    }

    //generates a clone of the current document
    Parameters Clone()
    {
        //TODO: make a clone
        //TODO: find a better way to make the copy
        return Parameters(mpvalue->dump());                     //new json(*mpvalue));
    }

    /// Destructor.
    virtual ~Parameters()
    {
    }

    const std::string WriteJsonString() const
    {
        return mpvalue->dump();
    }
  }
  Parameters AddEmptyValue(const std::string& entry) {
    if (this->Has(entry) == false) {
      rapidjson::Value tmp;
      rapidjson::Value name(entry.c_str(),
                            mpdoc->GetAllocator()); // rhis will be moved away
      this->mpvalue->AddMember(name, tmp, mpdoc->GetAllocator());
    }
    return this->GetValue(entry);
  }

  bool RemoveValue(const std::string& entry) {
    return mpvalue->RemoveMember(entry.c_str());
  }

    const  std::string PrettyPrintJsonString() const
    {
        return mpvalue->dump(4);

    }

  double GetDouble() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsNumber()) << "argument must be a number"
                                             << std::endl;
    return mpvalue->GetDouble();
  }
  int GetInt() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsNumber()) << "argument must be a number"
                                             << std::endl;
    return mpvalue->GetInt();
  }
  bool GetBool() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsBool()) << "argument must be a bool"
                                           << std::endl;
    return mpvalue->GetBool();
  }
  std::string GetString() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsString()) << "argument must be a string"
                                             << std::endl;
    return mpvalue->GetString();
  }
  Vector GetVector() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsArray())
        << "argument must be a Vector (a json list)" << std::endl;

    //*******************************************************************************************************
    Parameters GetValue(const std::string& entry)
    {
        auto j = mpvalue->find(entry);
        if( j == mpvalue->end()) KRATOS_THROW_ERROR(std::invalid_argument,"--------- ERROR : --------- getting a value that does not exist. entry string : ",entry);
        return Parameters(&(*j), mproot);
    }

    Parameters operator[](const std::string& entry)
    {
        return this->GetValue(entry);
    }

    void SetValue(const std::string& entry, const Parameters& other_value)
    {
        if(mpvalue->find(entry) == mpvalue->end()) KRATOS_THROW_ERROR(std::invalid_argument,"value must exist to be set. Use AddValue instead","");

        (*mpvalue)[entry] = *(other_value.mpvalue);
    }

    void AddValue(const std::string& entry, const Parameters& other_value)
    {
        if(mpvalue->find(entry) == mpvalue->end())
        {
            (*mpvalue)[entry] = *(other_value.mpvalue);
        }
    }

    Parameters AddEmptyValue(const std::string& entry)
    {
        if(this->Has(entry) == false)
        {
            return Parameters(&(*mpvalue)[entry],  mproot);
        }
        return this->GetValue(entry);
    }

    return V;
  }
  Matrix GetMatrix() const {
    KRATOS_ERROR_IF_NOT(mpvalue->IsArray())
        << "argument must be a Matrix (a json list of lists)" << std::endl;

    const unsigned int nrows = mpvalue->Size();
    KRATOS_ERROR_IF(nrows == 0)
        << "argument must be a Matrix (a json list of lists)" << std::endl;

    unsigned int ncols = 0;
    if ((*mpvalue)[0].IsArray())
      ncols = (*mpvalue)[0].Size();

    Matrix A(nrows, ncols);

    for (unsigned int i = 0; i < nrows; ++i) {
      auto &row_i = (*mpvalue)[i];
      KRATOS_ERROR_IF_NOT(row_i.IsArray()) << "not an array on row " << i
                                           << std::endl;
      KRATOS_ERROR_IF_NOT(row_i.Size() == ncols) << "wrong size of row " << i
                                                 << std::endl;
      for (unsigned int j = 0; j < ncols; ++j) {
        KRATOS_ERROR_IF_NOT((row_i)[j].IsNumber())
            << "Entry (" << i << "," << j << ") is not a number!" << std::endl;
        A(i, j) = (row_i)[j].GetDouble();
      }
    }

    return A;
  }

    //*******************************************************************************************************
    bool Has(const std::string& entry) const
    {
        return mpvalue->find(entry) != mpvalue->end();
    }

    bool IsNull() const
    {
        return mpvalue->is_null();
    }
    bool IsNumber() const
    {
        return mpvalue->is_number();
    }
    bool IsDouble() const
    {
        return mpvalue->is_number_float();
    }
    bool IsInt() const
    {
        return mpvalue->is_number_integer();
    }
    bool IsBool() const
    {
        return mpvalue->is_boolean();
    }
    bool IsString() const
    {
        return mpvalue->is_string();
    }
    bool IsArray() const
    {
        return mpvalue->is_array();
    }
    bool IsSubParameter() const
    {
        return mpvalue->is_object();
    }

    double GetDouble() const
    {
        return mpvalue->get<double>();
    }
    int GetInt() const
    {
        if(mpvalue->is_number() == false) KRATOS_THROW_ERROR(std::invalid_argument,"argument must be a number","");
        return mpvalue->get<int>();
    }
    bool GetBool() const
    {
        if (mpvalue->is_boolean() == false)
        {
            //RecursivelyFindValue(*mpdoc, *mpvalue);
            KRATOS_THROW_ERROR(std::invalid_argument, "argument must be a bool", "");
        }
        return mpvalue->get<bool>();
    }
    std::string GetString() const
    {
        if(mpvalue->is_string() == false) KRATOS_THROW_ERROR(std::invalid_argument,"argument must be a string","");
        return mpvalue->get<std::string>();
    }
  }
  void SetMatrix(const Matrix &mat) {
    const unsigned int nrows = mat.size1();
    const unsigned int ncols = mat.size2();

    void SetDouble(const double value)
    {
        *mpvalue=value;
    }
    void SetInt(const int value)
    {
        *mpvalue=value;
    }
    void SetBool(const bool value)
    {
        *mpvalue=value;
    }
    void SetString(const std::string& value)
    {
        *mpvalue=value;
    }
  }

  iterator begin() { return iterator(this->mpvalue->MemberBegin(), mpdoc); }

    iterator begin()
    {
        return iterator(mpvalue->begin(),  mproot);
    }

    iterator end()
    {
        return iterator(mpvalue->end(),  mproot);
    }

    const_iterator begin() const
    {
        return const_iterator(mpvalue->cbegin(),  mproot);
    }

    const_iterator end() const
    {
        return const_iterator(mpvalue->cend(),  mproot);
    }

    //*******************************************************************************************************
    //methods for array
    unsigned int size() const
    {
        if(mpvalue->is_array() == false)
            KRATOS_THROW_ERROR(std::invalid_argument,"size can only be queried if the value if of Array type","");
        return mpvalue->size();
    }

    Parameters GetArrayItem(unsigned int index)
    {
        if(mpvalue->is_array() == false)
            KRATOS_THROW_ERROR(std::invalid_argument,"GetArrayItem only makes sense if the value if of Array type","")
            else
            {
                if(index >= mpvalue->size())
                    KRATOS_THROW_ERROR(std::invalid_argument,"index exceeds array size. Index value is : ",index)
                    return Parameters(&((*mpvalue)[index]),  mproot);
            }
    }

    void SetArrayItem(unsigned int index, const Parameters& other_array_item)
    {
        if(mpvalue->is_array() == false)
            KRATOS_THROW_ERROR(std::invalid_argument,"SetArrayItem only makes sense if the value if of Array type","")
            else
            {
                if(index >= mpvalue->size())
                    KRATOS_THROW_ERROR(std::invalid_argument,"index exceeds array size. Index value is : ",index)

                    (*mpvalue)[index] = *other_array_item.mpvalue;
            }
    }
    Parameters operator[](unsigned int index)
    {
        return this->GetArrayItem(index);
    }

    /**This function is designed to verify that the parameters under testing match the
     * form prescribed by the defaults.
     * If the parameters contain values that do not appear in the defaults, an error is thrown,
     * whereas if a parameter is found in the defaults but not in the Parameters been tested,
     * it is copied to the parameters.
     *
     * this version of the function only walks one level, without descending in the branches
     */
    void ValidateAndAssignDefaults(Parameters& defaults)
    {
        KRATOS_TRY

//first verifies that all the enries in the current parameters
        //have a correspondance in the defaults.
        //if it is not the case throw an error
        for (auto itr = this->mpvalue->begin(); itr != this->mpvalue->end(); ++itr)
        {
            std::string item_name = itr.key();
            if(!defaults.Has(item_name) )
            {
                std::stringstream msg;
                msg << "the item with name \"" << item_name << "\" is present in this Parameters but NOT in the default values" << std::endl;
                msg << "hence Validation fails" << std::endl;
                msg << "parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "defaults against which the current parameters are validated are :" << std::endl;
                msg << defaults.PrettyPrintJsonString() << std::endl;
                KRATOS_THROW_ERROR(std::invalid_argument,"",msg.str());
            }

            bool type_coincides = false;
            auto value_defaults = (defaults[item_name]).GetUnderlyingStorage();
            if(itr->is_number() && value_defaults->is_number()) type_coincides = true;
            
//             if(itr->is_number_float() && value_defaults->is_number_float()) type_coincides = true;
            if(itr->is_array() && value_defaults->is_array()) type_coincides = true;
            if(itr->is_string() && value_defaults->is_string()) type_coincides = true;
            if(itr->is_object() && value_defaults->is_object()) type_coincides = true;

            //
            //both must be bool to be acceptable
            if(itr->is_boolean() && value_defaults->is_boolean()) type_coincides = true;
                
            if(type_coincides == false)
            {
                std::stringstream msg;
                msg << "******************************************************************************************************" << std::endl;
                msg << "the item with name :\"" << item_name << "\" does not have the same type as the corresponding one in the default values" << std::endl;
                msg << "******************************************************************************************************" << std::endl;
                msg << "parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "defaults against which the current parameters are validated are :" << std::endl;
                msg << defaults.PrettyPrintJsonString() << std::endl;
                KRATOS_THROW_ERROR(std::invalid_argument,"",msg.str());
            }

            //now iterate over all the defaults. In the case a default value is not assigned in the current Parameters
            //add an item copying its value
            if(defaults.IsSubParameter())
            {
                for (json::iterator itr = defaults.mpvalue->begin(); itr != defaults.mpvalue->end(); ++itr)
                {
                    std::string item_name = itr.key();
                    if(!this->Has(item_name))
                    {
                        (*mpvalue)[item_name] = itr.value();
                    }
                }
            }
        }

        KRATOS_CATCH("")
    }

    /**This function is designed to verify that the parameters under testing match the
     * form prescribed by the defaults.
     * If the parameters contain values that do not appear in the defaults, an error is thrown,
     * whereas if a parameter is found in the defaults but not in the Parameters been tested,
     * it is copied to the parameters.
     *
     * this version walks and validates the entire json tree below
     * the point at which the function is called
    */
    void RecursivelyValidateAndAssignDefaults(Parameters& defaults)
    {
        KRATOS_TRY


        //first verifies that all the enries in the current parameters
        //have a correspondance in the defaults.
        //if it is not the case throw an error
        for (auto itr = this->mpvalue->cbegin(); itr != this->mpvalue->cend(); ++itr)
        {
            std::string item_name = itr.key();

            if(!defaults.Has(item_name) )
            {
                std::stringstream msg;
                msg << "the item with name \"" << item_name << "\" is present in this Parameters but NOT in the default values" << std::endl;
                msg << "hence Validation fails" << std::endl;
                msg << "parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "defaults against which the current parameters are validated are :" << std::endl;
                msg << defaults.PrettyPrintJsonString() << std::endl;
                KRATOS_THROW_ERROR(std::invalid_argument,"",msg.str());
            }

            bool type_coincides = false;
            auto value_defaults = (defaults[item_name]).GetUnderlyingStorage();
            if(itr->is_number_integer() && value_defaults->is_number_integer()) type_coincides = true;
            if(itr->is_boolean() && value_defaults->is_boolean()) type_coincides = true;
            if(itr->is_number_float() && value_defaults->is_number_float()) type_coincides = true;
            if(itr->is_array() && value_defaults->is_array()) type_coincides = true;
            if(itr->is_string() && value_defaults->is_string()) type_coincides = true;
            if(itr->is_object() && value_defaults->is_object()) type_coincides = true;

            if(type_coincides == false)
            {
                std::stringstream msg;
                msg << "the item with name :\"" << item_name << "\" does not have the same type as the corresponding one in the default values" << std::endl;
                msg << "parameters being validated are : " << std::endl;
                msg << this->PrettyPrintJsonString() << std::endl;
                msg << "defaults against which the current parameters are validated are :" << std::endl;
                msg << defaults.PrettyPrintJsonString() << std::endl;
                KRATOS_THROW_ERROR(std::invalid_argument,"",msg.str());
            }
            //now walk the tree recursively
            if(itr->is_object())
            {
                Parameters subobject = (*this)[item_name];
                Parameters defaults_subobject = defaults[item_name];
                subobject.ValidateAndAssignDefaults(defaults_subobject);
            }
        }

  void Append(const Matrix &mat) {
    KRATOS_ERROR_IF_NOT(mpvalue->IsArray())
        << "it must be an Array parameter to append" << std::endl;
    rapidjson::Value tmp_value;

    const unsigned int nrows = mat.size1();
    const unsigned int ncols = mat.size2();

        //now iterate over all the defaults. In the case a default value is not assigned in the current Parameters
        //add an item copying its value
        if(defaults.IsSubParameter())
        {
            for (auto itr = defaults.mpvalue->begin(); itr != defaults.mpvalue->end(); ++itr)
            {
                std::string item_name = itr.key();
                if(mpvalue->find(item_name) ==  mpvalue->end())
                {
                    (*mpvalue)[item_name] = itr.value();
                }

                //now walk the tree recursively
                if(itr->is_object())
                {
                    Parameters subobject = (*this)[item_name];
                    Parameters defaults_subobject = defaults[item_name];
                    subobject.ValidateAndAssignDefaults(defaults_subobject);
                }
            }
        }

    for (unsigned int i = 0; i < nrows; ++i) {
      tmp_value.PushBack(0, mpdoc->GetAllocator()); // Pushing back a default
                                                    // element to allocate
                                                    // memory
      tmp_value[i].SetArray(); // change that default element to an array
      tmp_value[i].Reserve(ncols, mpdoc->GetAllocator());

      for (unsigned int j = 0; j < ncols; ++j) {
        tmp_value[i].PushBack(mat(i, j), mpdoc->GetAllocator());
      }
    }

//     void RecursivelyFindValue(
//         const rapidjson::Value& rbase_value,
//         const rapidjson::Value& rvalue_to_find) const
//     {
//         for (rapidjson::Value::ConstMemberIterator itr = rbase_value.MemberBegin(); itr != rbase_value.MemberEnd(); ++itr)
//         {
//             if (&(itr->value) == &rvalue_to_find)
//             {
//                 rapidjson::StringBuffer buffer;
//                 rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
//                 mpvalue->Accept(writer);
//                 std::cout << "base = " << buffer.GetString() << std::endl;
//                 std::cout << "problematic var name " << itr->name.GetString() << " value " << itr->value.GetString() << std::endl;
//             }
//             else
//             {
//                 if (itr->value.IsObject()) RecursivelyFindValue(itr->value, rvalue_to_find);
//                 //TODO: it could be an array
//             }
//         }
//     }

  void Append(const Parameters &object) {
    KRATOS_ERROR_IF_NOT(mpvalue->IsArray())
        << "it must be an Array parameter to append" << std::endl;
    rapidjson::Value tmp_value;
    tmp_value.CopyFrom(*(object.GetUnderlyingStorage()), mpdoc->GetAllocator());
    mpvalue->PushBack(tmp_value, mpdoc->GetAllocator());
  }

  Parameters operator[](unsigned int index) {
    return this->GetArrayItem(index);
  }

  /**This function is designed to verify that the parameters under testing match
   * the
   * form prescribed by the defaults.
   * If the parameters contain values that do not appear in the defaults, an
   * error is thrown,
   * whereas if a parameter is found in the defaults but not in the Parameters
   * been tested,
   * it is copied to the parameters.
   *
   * this version of the function only walks one level, without descending in
   * the branches
   */
  void ValidateAndAssignDefaults(Parameters &defaults) {
    KRATOS_TRY

    // first verifies that all the enries in the current parameters
    // have a correspondance in the defaults.
    // if it is not the case throw an error
    for (rapidjson::Value::ConstMemberIterator itr =
             this->mpvalue->MemberBegin();
         itr != this->mpvalue->MemberEnd(); ++itr) {
      std::string item_name = itr->name.GetString();

      if (!defaults.Has(item_name)) {
        std::stringstream msg;
        msg << "***************************************************************"
               "***************************************"
            << std::endl;
        msg << "the item with name \"" << item_name
            << "\" is present in this Parameters but NOT in the default values"
            << std::endl;
        msg << "***************************************************************"
               "***************************************"
            << std::endl;
        msg << "hence Validation fails" << std::endl;
        msg << "parameters being validated are : " << std::endl;
        msg << this->PrettyPrintJsonString() << std::endl;
        msg << "defaults against which the current parameters are validated "
               "are :"
            << std::endl;
        msg << defaults.PrettyPrintJsonString() << std::endl;
        KRATOS_ERROR << msg.str() << std::endl;
      }

      bool type_coincides = false;
      rapidjson::Value *value_defaults =
          (defaults[item_name.c_str()]).GetUnderlyingStorage();
      if (itr->value.IsInt() && value_defaults->IsNumber())
        type_coincides = true;
      if (itr->value.IsBool() && value_defaults->IsBool())
        type_coincides = true;
      if (itr->value.IsDouble() && value_defaults->IsDouble())
        type_coincides = true;
      if (itr->value.IsArray() && value_defaults->IsArray())
        type_coincides = true;
      if (itr->value.IsString() && value_defaults->IsString())
        type_coincides = true;
      if (itr->value.IsObject() && value_defaults->IsObject())
        type_coincides = true;
      if (itr->value.IsNull() && value_defaults->IsNull())
        type_coincides = true;

      if (type_coincides == false) {
        std::stringstream msg;
        msg << "***************************************************************"
               "***************************************"
            << std::endl;
        msg << "the item with name :\"" << item_name
            << "\" does not have the same type as the corresponding one in the "
               "default values"
            << std::endl;
        msg << "***************************************************************"
               "***************************************"
            << std::endl;
        msg << "parameters being validated are : " << std::endl;
        msg << this->PrettyPrintJsonString() << std::endl;
        msg << "defaults against which the current parameters are validated "
               "are :"
            << std::endl;
        msg << defaults.PrettyPrintJsonString() << std::endl;
        KRATOS_ERROR << msg.str() << std::endl;
      }
    }

    // now iterate over all the defaults. In the case a default value is not
    // assigned in the current Parameters
    // add an item copying its value
    if (defaults.IsSubParameter()) {
      for (rapidjson::Value::MemberIterator itr =
               defaults.mpvalue->MemberBegin();
           itr != defaults.mpvalue->MemberEnd(); ++itr) {
        std::string item_name = itr->name.GetString();
        if (!this->Has(item_name)) {
          rapidjson::Value *pvalue = &itr->value;

          this->AddValue(item_name, Parameters(pvalue, defaults.mpdoc));
        }
      }
    }

    KRATOS_CATCH("")
  }

  /**This function is designed to verify that the parameters under testing match
   * the
   * form prescribed by the defaults.
   * If the parameters contain values that do not appear in the defaults, an
   * error is thrown,
   * whereas if a parameter is found in the defaults but not in the Parameters
   * been tested,
   * it is copied to the parameters.
   *
   * this version walks and validates the entire json tree below
   * the point at which the function is called
  */
  void RecursivelyValidateAndAssignDefaults(Parameters &defaults) {
    KRATOS_TRY

    // first verifies that all the enries in the current parameters
    // have a correspondance in the defaults.
    // if it is not the case throw an error
    for (rapidjson::Value::ConstMemberIterator itr =
             this->mpvalue->MemberBegin();
         itr != this->mpvalue->MemberEnd(); ++itr) {
      std::string item_name = itr->name.GetString();

      if (!defaults.Has(item_name)) {
        std::stringstream msg;
        msg << "the item with name \"" << item_name
            << "\" is present in this Parameters but NOT in the default values"
            << std::endl;
        msg << "hence Validation fails" << std::endl;
        msg << "parameters being validated are : " << std::endl;
        msg << this->PrettyPrintJsonString() << std::endl;
        msg << "defaults against which the current parameters are validated "
               "are :"
            << std::endl;
        msg << defaults.PrettyPrintJsonString() << std::endl;
        KRATOS_ERROR << msg.str() << std::endl;
      }

      bool type_coincides = false;
      rapidjson::Value *value_defaults =
          (defaults[item_name.c_str()]).GetUnderlyingStorage();
      if (itr->value.IsInt() && value_defaults->IsInt())
        type_coincides = true;
      if (itr->value.IsBool() && value_defaults->IsBool())
        type_coincides = true;
      if (itr->value.IsDouble() && value_defaults->IsDouble())
        type_coincides = true;
      if (itr->value.IsArray() && value_defaults->IsArray())
        type_coincides = true;
      if (itr->value.IsString() && value_defaults->IsString())
        type_coincides = true;
      if (itr->value.IsObject() && value_defaults->IsObject())
        type_coincides = true;
      if (itr->value.IsNull() && value_defaults->IsNull())
        type_coincides = true;

      if (type_coincides == false) {
        std::stringstream msg;
        msg << "the item with name :\"" << item_name
            << "\" does not have the same type as the corresponding one in the "
               "default values"
            << std::endl;
        msg << "parameters being validated are : " << std::endl;
        msg << this->PrettyPrintJsonString() << std::endl;
        msg << "defaults against which the current parameters are validated "
               "are :"
            << std::endl;
        msg << defaults.PrettyPrintJsonString() << std::endl;
        KRATOS_ERROR << msg.str() << std::endl;
      }
      // now walk the tree recursively
      if (itr->value.IsObject()) {
        Parameters subobject = (*this)[item_name];
        Parameters defaults_subobject = defaults[item_name];
        subobject.RecursivelyValidateAndAssignDefaults(defaults_subobject);
      }
    }

    // now iterate over all the defaults. In the case a default value is not
    // assigned in the current Parameters
    // add an item copying its value
    if (defaults.IsSubParameter()) {
      for (rapidjson::Value::MemberIterator itr =
               defaults.mpvalue->MemberBegin();
           itr != defaults.mpvalue->MemberEnd(); ++itr) {
        std::string item_name = itr->name.GetString();
        if (!this->Has(item_name)) {
          rapidjson::Value *pvalue = &itr->value;

          this->AddValue(item_name, Parameters(pvalue, defaults.mpdoc));
        }

        // now walk the tree recursively
        if (itr->value.IsObject()) {
          Parameters subobject = (*this)[item_name];
          Parameters defaults_subobject = defaults[item_name];
          subobject.ValidateAndAssignDefaults(defaults_subobject);
        }
      }
    }

    KRATOS_CATCH("")
  }

private:
    json* mpvalue; //this is where the json is actually stored
    boost::shared_ptr<json> mproot;

    //ATTENTION: please DO NOT use this constructor. It assumes rapidjson and hence it should be considered as an implementation detail
    Parameters(json* pvalue, boost::shared_ptr<json> proot): mpvalue(pvalue), mproot(proot)
    {}

    //ATTENTION: please DO NOT use this constructor. It assumes rapidjson and hence it should be considered as an implementation detail
//     Parameters(const json::iterator& it): mpvalue(*it)
//     {
//         mis_owner = false;
//     }
//     //ATTENTION: please DO NOT use this constructor. It assumes rapidjson and hence it should be considered as an implementation detail
//     Parameters(const json::const_iterator& it): mpvalue(*it)
//     {
//         mis_owner = false;
//     }


    //ATTENTION: please DO NOT use this method. It is a low level accessor, and may change in the future
    json* GetUnderlyingStorage() const
    {
        return mpvalue;
    }
};

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function
inline std::istream &operator>>(std::istream &rIStream, Parameters &rThis) {
  return rIStream;
}

/// output stream function
inline std::ostream &operator<<(std::ostream &rOStream,
                                const Parameters &rThis) {
  rThis.PrintInfo(rOStream);
  rOStream << std::endl;
  rThis.PrintData(rOStream);

  return rOStream;
}
///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_KRATOS_PARAMETERS_H_INCLUDED  defined
